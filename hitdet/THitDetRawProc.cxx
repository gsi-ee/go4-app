#include "THitDetRawProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "TVirtualFFT.h"
#include "TGo4Analysis.h"
#include "TGo4Log.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"
#include "TGo4WinCond.h"

#include "THitDetRawEvent.h"
#include "THitDetRawParam.h"
#include "THitDetDisplay.h"

#include "TGo4Fitter.h"
#include "THitDetSinusModel.h"
#include "TGo4FitDataHistogram.h"
#include "TGo4FitParameter.h"
#include "TGo4FitModelPolynom.h"

#include "TGo4UserException.h"

/** enable this definition to print out event sample data explicitely*/
//#define HITDET_DATA_VERBOSE 1


static unsigned long skipped_events = 0;
//static unsigned long skipped_msgs = 0;

/* helper macros for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/

#define  HitDetEVENT_CHECK_PDATA                                    \
if((pdata - psubevt->GetDataField()) > lwords ) \
{ \
  printf("############ unexpected end of event for subevent size :0x%x, skip event %ld\n", lwords, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

#define  HitDetRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > HitDetRawEvent->fDataCount ) \
{ \
  printf("############ unexpected end of payload for datacount:0x%x, skip event %ld\n", HitDetRawEvent->fDataCount, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

#define  HitDetMSG_CHECK_PDATA                                    \
if((pdata - pdatastartMsg) > msize ) \
{ \
  printf("############ pdata offset 0x%x exceeds message size:0x%x, skip event %ld\n", (unsigned int)(pdata - pdatastartMsg), msize, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

// this one is to discard last message that may be cut off by vulom daq:
#define  HitDetEVENTLASTMSG_CHECK_PDATA                                    \
if((pdata - psubevt->GetDataField()) >= lwords ) \
 { \
  skipmessage=kTRUE; \
  break; \
}

/*printf("############ pdata offset 0x%x exceeds  subevent size :0x%x, skip message %ld\n", (unsigned int) (pdata - psubevt->GetDataField()), lwords, skipped_msgs++);\*/

//***********************************************************
THitDetRawProc::THitDetRawProc() :
    TGo4EventProcessor()
{
}

//***********************************************************
// this one is used in standard factory
THitDetRawProc::THitDetRawProc(const char* name) :
    TGo4EventProcessor(name),fSinusFitter(0)
{
  TGo4Log::Info("THitDetRawProc: Create instance %s", name);
  fBoards.clear();
  SetMakeWithAutosave(kTRUE);
//// init user analysis objects:

  fPar = dynamic_cast<THitDetRawParam*>(MakeParameter("HitDetRawParam", "THitDetRawParam", "set_HitDetRawParam.C"));
  if (fPar)
    fPar->SetConfigBoards();

  for (unsigned i = 0; i < THitDetRawEvent::fgConfigHitDetBoards.size(); ++i)
  {
    UInt_t uniqueid = THitDetRawEvent::fgConfigHitDetBoards[i];
    fBoards.push_back(new THitDetBoardDisplay(uniqueid));
  }

  InitDisplay(fPar->fTraceLength, fPar->fNumSnapshots, false);

}

//***********************************************************
THitDetRawProc::~THitDetRawProc()
{
  TGo4Log::Info("THitDetRawProc: Delete instance");
  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    delete fBoards[i];
  }
  delete fSinusFitter;

}

/* access to histogram set for current board id*/
THitDetBoardDisplay* THitDetRawProc::GetBoardDisplay(Int_t uniqueid)
{
  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    THitDetBoardDisplay* theDisplay = fBoards[i];
    if (uniqueid == theDisplay->GetDevId())
      return theDisplay;
  }
  return 0;
}

void THitDetRawProc::InitDisplay(int tracelength, Int_t numsnapshots, Bool_t replace)
{
  cout << "**** THitDetRawProc: Init Display for " << tracelength << " trace bins, snapshots=" << numsnapshots << endl;

  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    fBoards[i]->InitDisplay(tracelength, numsnapshots, replace);
  }

}

//-----------------------------------------------------------
// event function
Bool_t THitDetRawProc::BuildEvent(TGo4EventElement* target)
{
// called by framework from THitDetRawEvent to fill it
  HitDetRawEvent = (THitDetRawEvent*) target;
  HitDetRawEvent->SetValid(kFALSE);    // not store
  TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
  if (source == 0)
  {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  UShort_t triggertype = source->GetTrigger();

  if (triggertype > 11)
  {
    // frontend offset trigger can be one of these, we let it through to unpacking loop
    //cout << "**** THitDetRawProc: Skip trigger event" << endl;
    GO4_SKIP_EVENT_MESSAGE( "**** THitDetRawProc: Skip event of trigger type 0x%x", triggertype);
    //return kFALSE; // this would let the second step execute!
  }

  UInt_t snapshotcount[HitDet_CHANNELS] = {0};    // counter for trace snapshot display
  static UInt_t tracelongcount = 0;    // counter for direct adc trace long part
  static Int_t numsnapshots = fPar->fNumSnapshots;
  static Int_t tracelength = fPar->fTraceLength;

  // since we fill histograms already in BuildEvent, need to check if we must rescale histogram displays:
  if ((fPar->fNumSnapshots != numsnapshots) || (fPar->fTraceLength != tracelength))
  {
    numsnapshots = fPar->fNumSnapshots;
    tracelength = fPar->fTraceLength;
    InitDisplay(tracelength, numsnapshots, kTRUE);

  }

// first we fill the THitDetRawEvent with data from MBS source
// we have up to two subevents, crate 1 and 2
// Note that one has to loop over all subevents and select them by
// crate number:   psubevt->GetSubcrate(),
// procid:         psubevt->GetProcid(),
// and/or control: psubevt->GetControl()
// here we use only crate number

  source->ResetIterator();
  TGo4MbsSubEvent* psubevt(0);
  while ((psubevt = source->NextSubEvent()) != 0)
  {    // loop over subevents
    Int_t *pdata = psubevt->GetDataField();
    Int_t lwords = psubevt->GetIntLen();

    if ((unsigned) *pdata == 0xbad00bad)
    {
      GO4_SKIP_EVENT_MESSAGE("**** THitDetRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*pdata));
    }
    Bool_t skipmessage = kFALSE;
    // loop over single subevent data:
    while (pdata - psubevt->GetDataField() < lwords)
    {
      // vulom status word:
      HitDetRawEvent->fVULOMStatus = *pdata++;
      //event trigger counter:
      HitDetRawEvent->fSequenceNumber = *pdata++;
      // data length
      HitDetRawEvent->fDataCount = *pdata++;
      if (HitDetRawEvent->fDataCount > (lwords - 3))
      {
        GO4_SKIP_EVENT_MESSAGE(
            "**** THitDetRawProc: Mismatch with subevent len %d and data count %d", lwords, HitDetRawEvent->fDataCount);
        // avoid that we run optional second step on invalid raw event!
      }

      Int_t* pdatastart = pdata;    // remember begin of asic payload data section


      pdata++;    // skip first  word?
      // pdatastart was here JAM

      // now fetch boardwise subcomponents for output data and histograming:
      Int_t slix = 0;    // JAM here we later could evaluate a board identifier mapped to a slot/sfp number contained in subevent
      UInt_t brdid = fPar->fBoardID[slix];    // get hardware identifier from "DAQ link index" number
      THitDetBoard* theBoard = HitDetRawEvent->GetBoard(brdid);
      if (theBoard == 0)
      {
        GO4_SKIP_EVENT_MESSAGE(
            "Configuration error: Board id %d does not exist as subevent, slot index:%d", brdid, slix);

        return kFALSE;
      }
      THitDetBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
      if (boardDisplay == 0)
      {
        GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
        return kFALSE;
      }
      boardDisplay->ResetDisplay(kFALSE);

      // evaluate HitDetection ASIC messages in the payload:

      while ((pdata - pdatastart) < HitDetRawEvent->fDataCount)
      {

        // first vulom wrapper containing total message length:
        Int_t vulombytecount = *pdata++;
        if (((vulombytecount >> 28) & 0xF) != 0x4)
        {
          // check ROByteCounter marker
          printf("Data error: wrong vulom byte counter 0x%x, skip event %ld", vulombytecount, skipped_events++);
          GO4_SKIP_EVENT
        }
//        // JAM2019 new: evaluate chip id here
        Int_t chipid=(vulombytecount >> 16) & 0xFF;
        boardDisplay->hChipId->Fill(chipid);


        Int_t* pdatastartMsg = pdata;    // remember start of message for checking
        UChar_t msize = (vulombytecount & 0x3F) / sizeof(Int_t);    // message size in 32bit words
        // evaluate message type from header:
        Int_t header = *pdata;
        // not we do not increment pdata here, do this inside msg types
        Int_t mtype = ((header >> 30) & 0x3);

        boardDisplay->hMsgTypes->Fill(mtype);
        //printf("MMMMMMMM message type %d \n",mtype);
        switch (mtype)
        {

          case THitDetMsg::MSG_ADC_Direct:    // direct ADC readout
            {
              // message counter
              UShort_t msgcount = ((header >> 20) & 0x3FF);
              //printf("MSG_ADC_Direct msgcoutn=%d \n",msgcount);
              THitDetMsgDirect* theMsg = new THitDetMsgDirect(msgcount);
              tracelongcount = msgcount;    // reset trace histogram
              Int_t adcdata[4];
              for (Int_t j = 0; j < 4; ++j)
              {
                HitDetEVENTLASTMSG_CHECK_PDATA
                HitDetRAW_CHECK_PDATA;
                HitDetMSG_CHECK_PDATA;
                adcdata[j] = *pdata++;

                //printf(" - data[%d]=0x%x \n",j,adcdata[j]);
              }
              if (!skipmessage)
              {
                // decode sample bin data (0-7)
                theMsg->SetBinData(0, ((adcdata[0] >> 8) & 0xFFF));
                theMsg->SetBinData(1, ((adcdata[0] & 0xFF) << 4) | ((adcdata[1] >> 28) & 0xF));
                theMsg->SetBinData(2, (adcdata[1] >> 16) & 0xFFF);
                theMsg->SetBinData(3, (adcdata[1] >> 4) & 0xFFF);
                theMsg->SetBinData(4, ((adcdata[1] & 0xF) << 8) | ((adcdata[2] >> 24) & 0xFF));
                theMsg->SetBinData(5, (adcdata[2] >> 12) & 0xFFF);
                theMsg->SetBinData(6, adcdata[2] & 0xFFF);
                theMsg->SetBinData(7, (adcdata[3] >> 20) & 0xFFF);

                // here directly evaluate display:

                TH1* tracesnapshot = 0;
                TH1* tracelong = 0;
                TH1* tracelongcorr = 0;
                TH1* tracelongsum = 0;
                TH1* tracelongcorrsum = 0;
                TH2* trace2d = 0;
                if (snapshotcount[0] < HitDet_MAXSNAPSHOTS)
                {
                  tracesnapshot = boardDisplay->hTraceSnapshots[0][snapshotcount[0]];
                  trace2d = boardDisplay->hTraceSnapshot2d[0];
                }
                if (tracelongcount < HitDet_MAXTRACELONG)
                {
                  tracelong = boardDisplay->hTraceLong;
                  tracelongcorr = boardDisplay->hTraceLongCorrected;
                  tracelongsum = boardDisplay->hTraceLongSum;
                  tracelongcorrsum = boardDisplay->hTraceLongSumCorrected;
                  if (tracelongcount == 0)
                  {

                    boardDisplay->hTraceLongPrev->Reset("");
                    boardDisplay->hTraceLongPrev->Add(tracelong);    // copy previous full trace to buffer histogram

                    // now provide corrected trace long:
                    boardDisplay->hTraceLongPrevCorrected->Reset("");
                    boardDisplay->hTraceLongPrevCorrected->Add(tracelongcorr);    // copy previous full trace to buffer histogram

                    // begin of new trace, provide FFT of previous one here:
                    DoFFT(boardDisplay);

                    // also do the sinus fit if enabled:
                    DoSinusFit(boardDisplay);

                    tracelong->Reset("");
                    tracelongcorr->Reset("");
                  }

                }

                for (Int_t k = 0; k < 8; ++k)
                {

                  Short_t val = theMsg->GetBinData(k);
                  // convert raw data to signed 8bit (2 complement) representation:
//                  if (val & 0x800 != 0)
//                    val |= 0xf000;

                  if (val > 0x7FF)
                    val = val - 0x1000;

                  Double_t corrval = CorrectedADCVal(val, boardDisplay);

                  if (tracesnapshot)
                    tracesnapshot->SetBinContent(k + 1, val);
                  if (trace2d)
                    trace2d->Fill(k, 0.5 + snapshotcount[0], val);

                  if (tracelong)
                    tracelong->SetBinContent(1 + k + (8 * tracelongcount), val);
                  if (tracelongsum)
                    tracelongsum->AddBinContent(1 + k + (8 * tracelongcount), val);

                  if (tracelongcorr)
                    tracelongcorr->SetBinContent(1 + k + (8 * tracelongcount), corrval);
                  if (tracelongcorrsum)
                    tracelongcorrsum->AddBinContent(1 + k + (8 * tracelongcount), corrval);

                  boardDisplay->hTrace[0]->SetBinContent(1 + k, val);
                  boardDisplay->hTraceSum[0]->AddBinContent(1 + k, val);

                  // value histograms:
                  boardDisplay->hADCValues->Fill(val);

                }

                theBoard->AddMessage(theMsg, 0);    // direct ADC messages assigned to channel 0
                snapshotcount[0]++;
              }    // if !skipmessage
            }
            break;

          case THitDetMsg::MSG_ADC_Event:    // triggered event read out
         // case THitDetMsg::MSG_Unused:
            {
              UChar_t channel = ((header >> 28) & 0x3);
              UChar_t size12bit = ((header >> 20) & 0x3F);
              UChar_t size32bit = ((1 + size12bit) * 3) / 8;    // account header word again in evdata
              // account partially filled last data word here:
              if((Float_t) (1 + size12bit) * 3.0 /8.0 > (Float_t) size32bit) size32bit++;
#ifdef HITDET_DATA_VERBOSE
              printf("MSG_ADC_Event channel:%d size12:%d size32:%d\n",channel,size12bit,size32bit);
#endif
              if ((pdata - pdatastart) + size32bit > lwords)
                GO4_SKIP_EVENT_MESSAGE(
                    "ASIC Event header error: 12 bit size %d does not fit into mbs subevent buffer of restlen %d words", size12bit, lwords - (pdata - pdatastart));

              /** JAM new 2019 :*/
              boardDisplay->hDatawords->Fill(size12bit);
              boardDisplay->hChannels->Fill(channel);

              Int_t evdata[size32bit];
              for (Int_t j = 0; j < size32bit; ++j)
              {
                HitDetRAW_CHECK_PDATA;
                HitDetMSG_CHECK_PDATA;
                evdata[j] = *pdata++;
#ifdef HITDET_DATA_VERBOSE
                printf("MSG_ADC_Event copies 32bit data[%d]=0x%x \n",j,evdata[j]);
#endif
              }
              THitDetMsgEvent* theMsg = new THitDetMsgEvent(channel);
              theMsg->SetEpoch(((evdata[0] & 0x3FFFF) << 4) | ((evdata[1] >> 28) & 0xF));
              theMsg->SetTimeStamp((evdata[1] >> 16) & 0xFFF);

              /** changed format 2019 - upper bits of epoch counter used for memory row analyis:*/
              Short_t memoryrow=((evdata[0] >> 18) & 0x3);
              boardDisplay->hMemoryRow->Fill(memoryrow);

              /** JAM new 2019 - evaluate time difference between subsequent event messages*/
              UInt_t lastepoch=fLastMessages[channel].GetEpoch();
              UShort_t lasttimestamp=fLastMessages[channel].GetTimeStamp();
              if(lastepoch!=0 && lasttimestamp!=0)
                {
                  UInt_t deltaepoch=theMsg->GetEpoch() - lastepoch;
                  UShort_t deltatimestamp = theMsg->GetTimeStamp() - lasttimestamp;
                  //ULong_t deltaT= (deltaepoch<<12) | deltatimestamp;
                  // the above would require too large a histogram. We separate ts and epoch histograms:
                  boardDisplay->hDeltaTSMsg[channel]->Fill(deltatimestamp);
                  boardDisplay->hDeltaEPMsg[channel]->Fill(deltaepoch);
                  boardDisplay->hDeltaEPMsgFine[channel]->Fill(deltaepoch);

              }
              fLastMessages[channel]=*theMsg; // remember us for next message
              /** end 2019 time differences**/

              // now decode 12 bit samples inside mbs data words:�
              Int_t binlen = size12bit - 3;    // number of sample bins (should be 8,16, or 32)
              if (binlen > 32)
                GO4_SKIP_EVENT_MESSAGE("ASIC Event header error: bin length %d exceeds maximum 32", binlen);

              UShort_t val = 0;
              Int_t dixoffset = 1;    // actual sample data begins after header and timestamp,
              // j counts global bit number in stream, j_start is first in stream, j_end is last
              // k is local bit number in evdata word (lsb=0)
              UShort_t j_start = 16;    // begin of first 12 bit sample is after timestamp
              for (Int_t bin = 0; bin < binlen; ++bin)
              {
                UShort_t j_end = j_start + 12;
                Int_t dix_start = (Int_t) j_start / 32;    // data index containing first bit of sample
                Int_t dix_end = (Int_t) (j_end -1) / 32;    // data index containing last bit of sample
                UChar_t k_start = 32 - (j_start - 32 * dix_start);    //  start bit number in evdata word
                UChar_t k_end = 32 - (j_end - 32 * dix_end);    // end bit number in evdata word
#ifdef HITDET_DATA_VERBOSE
                printf("MSG_ADC_Event jstart:%d jend:%d dix_start:%d dix_end:%d k_start:%d k_end:%d \n",
                    (int) j_start, (int) j_end, dix_start, dix_end, (int) k_start, (int) k_end);
#endif
                if (dix_start == dix_end)
                {
                  // easy case, sample is inside one evdata word:
                  val = (evdata[dixoffset + dix_start] >> k_end) & 0xFFF;
#ifdef HITDET_DATA_VERBOSE
                  printf("MSG_ADC_Event sees NON spanning value\n");
#endif
                }
                else if (dix_end == dix_start + 1)
                {
                  // spanning over 2 evdata words:
                  UChar_t mask_start = 0, mask_end = 0;
                  for (UChar_t b = 0; b < k_start; ++b)
                    mask_start |= (1 << b);

                  for (UChar_t b = 0; b < (32 - k_end); ++b)
                    mask_end |= (1 << b);
                  val = ((evdata[dixoffset + dix_start] & mask_start) << (12 - k_start));
                  val |= (evdata[dixoffset + dix_end] >> k_end) & mask_end;
#ifdef HITDET_DATA_VERBOSE
                  printf("MSG_ADC_Event sees SPANNING value\n");
#endif
                }
                else
                {
                  // never come here
                  GO4_STOP_ANALYSIS_MESSAGE(
                      "NEVER COME HERE: mismatch of evsample indices - dix_end:%d and dix_start:%d", dix_end, dix_start)

                }
#ifdef HITDET_DATA_VERBOSE
                printf("MSG_ADC_Event set bin:%d to val:%d\n",bin,val);
#endif
                theMsg->SetTraceData(bin, val);
                j_start=j_end; // next 12 bit word
              }    // for bin

              // here do simple histogramming of traces:

              TH1* tracesnapshot = 0;
              TH2* trace2d = 0;
              if (snapshotcount[channel] < HitDet_MAXSNAPSHOTS)
              {
                tracesnapshot = boardDisplay->hTraceSnapshots[channel][snapshotcount[channel]];
                //std::cout<< "getting histogram for  channel "<< (int) channel<<" snapshot:"<<snapshotcount[channel]<<", binlen="<<binlen<<", tracelen="<< tracelength<< std::endl;
                trace2d = boardDisplay->hTraceSnapshot2d[channel];
              }
              for (Int_t bin = 0; bin < binlen; ++bin)
              {
                Short_t val = theMsg->GetTraceData(bin);
                // convert raw data to signed 8bit (2 complement) representation:
                if (val > 0x7FF)
                  val = val - 0x1000;

                if (tracesnapshot && bin<tracelength)
                  {
                    tracesnapshot->SetBinContent(bin + 1, val);
                    //std::cout<< " --- Set bin:"<<bin<<" to content:"<<val << std::endl;
                  }
                if (trace2d)
                  trace2d->Fill(bin, val, snapshotcount[channel]);

                if (channel >= HitDet_CHANNELS)
                {
                  printf("MSG_ADC_Event channel:%d out of range %d\n", channel, HitDet_CHANNELS);
                }
                else
                {
                  if (bin<tracelength)
                    {
                      boardDisplay->hTrace[channel]->SetBinContent(1 + bin, val);
                      boardDisplay->hTraceSum[channel]->AddBinContent(1 + bin, val);
                    }
                 }

                // value histograms:
                boardDisplay->hADCValues->Fill(val);

              }

              theBoard->AddMessage(theMsg, channel);
              snapshotcount[channel]++;
            }

            break;

          case THitDetMsg::MSG_Wishbone:
            // wishbone response (error message)
            {

              pdata = pdatastartMsg + msize;
              UChar_t wishhead = (header >> 24) & 0xFF;
              //printf("MSG_WishboneEvent header=0x%x\n",wishhead);

              THitDetMsgWishbone* theMsg = new THitDetMsgWishbone(wishhead);
              Int_t address = 0, val = 0;
//                ;
              pdata++;    //account header already processed above

              if (pdata - pdatastartMsg < msize)
              {
                address = *pdata++;
                theMsg->SetAddress(address);
              }
////                // here we could take rest of message as data contents:
              while (pdata - pdatastartMsg < msize)
              {
                HitDetRAW_CHECK_PDATA;
                val = *pdata++;
                theMsg->AddData(val);
              }
              pdata--;    // rewind pointer to end of payload
              boardDisplay->hWishboneAck->Fill(theMsg->GetAckCode());
              boardDisplay->hWishboneSource->Fill(theMsg->GetSource());
              boardDisplay->lWishboneText->SetText(0.1, 0.9, theMsg->DumpMsg());
              theBoard->AddMessage(theMsg, 0);    // wishbone messages accounted for channel 0

//                if(theMsg->GetDataSize()>0)
//                {
//                  printf("Wishbone text: %s",theMsg->DumpMsg().Data());
//                  std::cout<<std::endl;
//                }

            }
            break;

          default:
            //printf("############ found unknown message type 0x%x, skip event %ld\n", mtype, skipped_events++);
            //GO4_SKIP_EVENT
            pdata = pdatastartMsg + msize ; // do not skip complete event, but just the current message:
            break;
        };    // switch

        if (!skipmessage && (pdata - pdatastartMsg) < msize)
        {
          // never come here if messages are treated correctly!
          printf("############  pdata offset 0x%x has not yet reached message length 0x%x, correcting ,\n",
              (unsigned int) (pdata - pdatastartMsg), msize);
          pdata = pdatastartMsg + msize;
        }

        //printf("EEEEEEEE  end of message payload: pdata offset 0x%x msglength 0x%x\n",
        //             (unsigned int) (pdata - pdatastartMsg), msize);

      }    // while ((pdata - pdatastart) < HitDetRawEvent->fDataCount)

    }    // while pdata - psubevt->GetDataField() <lwords

  }    // while subevents

//

  UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
  HitDetRawEvent->SetValid(kTRUE);    // to store


  if (fPar->fSlowMotion)
    {
        Int_t evnum=source->GetCount();
        GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
    }


  return kTRUE;
}

Bool_t THitDetRawProc::UpdateDisplays()
{
  static Bool_t CalibrateWasOn = kFALSE;

// maybe later some advanced analysis from output event data here

  for (unsigned i = 0; i < THitDetRawEvent::fgConfigHitDetBoards.size(); ++i)
  {
    UInt_t brdid = THitDetRawEvent::fgConfigHitDetBoards[i];
    THitDetBoard* theBoard = HitDetRawEvent->GetBoard(brdid);
    if (theBoard == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("FillDisplays Configuration error: Board id %d does not exist!", brdid);
      //return kFALSE;
    }
    THitDetBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE(
          "FillDisplays Configuration error: Board id %d does not exist as histogram display set!", brdid);
      //return kFALSE;
    }

    // here calculate integral and differential ADC nonlinearities:
    boardDisplay->hADCNonLinInt->Reset("");
    boardDisplay->hADCNonLinDiff->Reset("");

    if (!CalibrateWasOn && fPar->fDoCalibrate)
    {
      // begin of calibration, reset value histograms
      boardDisplay->hADCValues->Reset("");
      boardDisplay->hADCCorrection->Reset("");
      TGo4Log::Info("THitDetBoardDisplay: Begin new ADC calibration for Board %d ", boardDisplay->GetDevId());

    }
    if (CalibrateWasOn && !fPar->fDoCalibrate)
    {
      TGo4Log::Info("THitDetBoardDisplay: End ADC calibration for Board %d ", boardDisplay->GetDevId());
    }
    Double_t mean = boardDisplay->hADCValues->GetEntries() / boardDisplay->hADCValues->GetNbinsX();
    Double_t inl = 0;
    Double_t corr = 0;
    for (Int_t bix = 0; bix < boardDisplay->hADCValues->GetNbinsX(); ++bix)
    {
      Double_t val = boardDisplay->hADCValues->GetBinContent(bix + 1);
      Double_t delta = (val - mean);
      boardDisplay->hADCDeltaMeanValues->SetBinContent(bix + 1, delta);
      Double_t dnl = 0;
      if (mean)
        dnl = delta / mean;    //dnl = TMath::Abs(delta / mean);
      boardDisplay->hADCNonLinDiff->SetBinContent(bix + 1, dnl);
      if (mean)
        inl += delta / mean;
      boardDisplay->hADCNonLinInt->SetBinContent(bix + 1, inl);
      // calibrate for ADC nonlinearity corrections:
      if (fPar->fDoCalibrate)
      {
        corr = inl;    // this is point to evaluate other kind of correction optionally
        boardDisplay->hADCCorrection->SetBinContent(bix + 1, corr);
      }
    }

  }    // i board

  CalibrateWasOn = fPar->fDoCalibrate;

  return kTRUE;
}

void THitDetRawProc::DoFFT(THitDetBoardDisplay* boardDisplay)
{
  if (fPar->fDoFFT)
  {
    // JAM taken from example $GO4SYS/macros/fft.C
    boardDisplay->hTraceLongFFT->Reset("");
    TString opt = fPar->fFFTOptions;    // ROOT fft parameters, user defined
    //opt.Append(" K"); // "keep" flag - use different instances of fft for each transformation.
//         Available transform types:
//         FFT:
//         - "C2CFORWARD" - a complex input/output discrete Fourier transform (DFT)
//                          in one or more dimensions, -1 in the exponent
//         - "C2CBACKWARD"- a complex input/output discrete Fourier transform (DFT)
//                          in one or more dimensions, +1 in the exponent
//         - "R2C"        - a real-input/complex-output discrete Fourier transform (DFT)
//                          in one or more dimensions,
//         - "C2R"        - inverse transforms to "R2C", taking complex input
//                          (storing the non-redundant half of a logically Hermitian array)
//                          to real output
//         - "R2HC"       - a real-input DFT with output in halfcomplex format,
//                          i.e. real and imaginary parts for a transform of size n stored as
//                          r0, r1, r2, ..., rn/2, i(n+1)/2-1, ..., i2, i1
//         - "HC2R"       - computes the reverse of FFTW_R2HC, above
//         - "DHT"        - computes a discrete Hartley transform

    //1st parameter:
    //  Possible flag_options:
    //  "ES" (from "estimate") - no time in preparing the transform, but probably sub-optimal
    //       performance
    //  "M" (from "measure") - some time spend in finding the optimal way to do the transform
    //  "P" (from "patient") - more time spend in finding the optimal way to do the transform
    //  "EX" (from "exhaustive") - the most optimal way is found
    //  This option should be chosen depending on how many transforms of the same size and
    //  type are going to be done. Planning is only done once, for the first transform of this
    //  size and type.

    //Examples of valid options: "R2C ES ", "C2CF M", "DHT P ", etc.

    Int_t N = boardDisplay->hTraceLongPrev->GetNbinsX();
    Double_t *in = new Double_t[N];
    // since we do not know type of input histo, we copy contents to Double array:
    for (Int_t ix = 0; ix < N; ++ix)
    {
      in[ix] = boardDisplay->hTraceLongPrev->GetBinContent(ix + 1);
    }
    DoFilter(in, N);
    TVirtualFFT *thefft = TVirtualFFT::FFT(1, &N, opt.Data());
    thefft->SetPoints(in);
    thefft->Transform();
    Double_t re, im;
    for (Int_t i = 0; i < N; i++)
    {
      thefft->GetPointComplex(i, re, im);
      boardDisplay->hTraceLongFFT->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
      // JAM todo: maybe it is more simple to directly work with GetPointReal()
      // in case of option DHT (real 2 real)

    }
    // now partial fft from window condition:
    boardDisplay->hTracePartFFT->Reset("");
    Double_t *inpart = new Double_t[N];
    Int_t Npart = 0;
    for (Int_t ix = 0; ix < N; ++ix)
    {
      if (boardDisplay->cWindowFFT->Test(ix))
      {
        inpart[ix] = boardDisplay->hTraceLongPrev->GetBinContent(ix + 1);
        Npart++;
      }
    }
    DoFilter(inpart, Npart);
    //delete thefft; // for keep option
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(inpart);
    thefft->Transform();
    for (Int_t i = 0; i < Npart; i++)
    {
      thefft->GetPointComplex(i, re, im);
      boardDisplay->hTracePartFFT->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
    }
    // finally partial fft from window at corrected trace:
    boardDisplay->hTracePartCorrectedFFT->Reset("");
    Double_t *incorr = new Double_t[N];
    Npart = 0;
    for (Int_t ix = 0; ix < N; ++ix)
    {
      if (boardDisplay->cWindowFFT->Test(ix))
      {
        incorr[ix] = boardDisplay->hTraceLongPrevCorrected->GetBinContent(ix + 1);
        Npart++;
      }
    }
    DoFilter(incorr, Npart);
    //delete thefft; // for keep option
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(incorr);
    thefft->Transform();
    for (Int_t i = 0; i < Npart; i++)
    {
      thefft->GetPointComplex(i, re, im);
      boardDisplay->hTracePartCorrectedFFT->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
    }
    delete[] in;
    delete[] inpart;
    delete[] incorr;
  }    // if dofft
}

void THitDetRawProc::DoFilter(Double_t* array, Int_t N)
{
  // please compare https://en.wikipedia.org/wiki/Window_function
  if (fPar->fFilterType == THitDetRawParam::FIL_NONE)
    return;
  Double_t factor = 0;
  for (Int_t i = 0; i < N; i++)
  {
    // generalized cosine filter:
    factor = fPar->fFilterCoeff[0];    // JAM put this out of sum loop to avoid nasty numerical problems with TMath::Cos ???
    for (Int_t j = 1; j < HitDet_FILTERCOEFFS; ++j)
    {
      factor += fPar->fFilterCoeff[j] * TMath::Cos((double) j * 2.0 * TMath::Pi() * (double) i / (double) (N - 1));
    }    // for j
    array[i] *= factor;
  }    // for i

}


void THitDetRawProc::DoSinusFit( THitDetBoardDisplay* boardDisplay)
{
  if (!fPar->fDoSinusFit) return;

  if(fSinusFitter==0)
  {
    fSinusFitter= new TGo4Fitter ("SinusFitter", TGo4Fitter::ff_chi_square, kTRUE);
    TGo4FitDataHistogram* datahis=fSinusFitter->AddH1("data1",  boardDisplay->hTraceLongPrev, kFALSE, boardDisplay->cWindowFFT->GetXLow(), boardDisplay->cWindowFFT->GetXUp());
    datahis->SetExcludeLessThen(-10000.0);

    fSinusFitter->AddPolynomX("data1", "Offset", 0);

    THitDetSinusModel* model = new THitDetSinusModel("sinus");
    fSinusFitter->AddModel("data1", model);
    fSinusFitter->SetMemoryUsage(2);
  }

  TGo4FitModelPolynom* baseModel = dynamic_cast<TGo4FitModelPolynom*> (fSinusFitter->GetModel(0));
    if(!baseModel)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
                           "NEVER COME HERE: baseline model not correctly assigned to fitter!");
    }


  THitDetSinusModel* sinusModel = dynamic_cast<THitDetSinusModel*> (fSinusFitter->GetModel(1));
  if(!sinusModel)
  {
    GO4_STOP_ANALYSIS_MESSAGE(
                         "NEVER COME HERE: sinus model not correctly assigned to fitter!");
  }
//

  TGo4FitDataHistogram* datahis=fSinusFitter->SetH1("data1", boardDisplay->hTraceLongPrev);
  datahis->SetRange(0, boardDisplay->cWindowFFT->GetXLow(), boardDisplay->cWindowFFT->GetXUp()); // may dynamically adjust range after startup
  datahis->SetExcludeLessThen(-10000.0);


  baseModel->SetParsValues(fPar->fSinusBaseline);


  // try: here first fit the baseline and fix it?
//  baseModel->FindPar("Ampl")->SetFixed(kFALSE);
//  fSinusFitter->ClearModelAssignmentTo("sinus", "data1");
//
//  fSinusFitter->DoActions();
//  fSinusFitter->DoActions();
//  fSinusFitter->DoActions();
//
//  if(fPar->fSlowMotion)  std::cout<< " ---------- first fit finds baseline value "<<baseModel->GetAmplValue()<<std::endl;
//
//  // then switch on sinus fit again:
//  baseModel->FindPar("Ampl")->SetFixed(kTRUE);
//  fSinusFitter->AssignModelTo("sinus", "data1");

  Double_t maxpos=boardDisplay->cWindowFFT->GetXMax(boardDisplay->hTraceLongPrev);
  Int_t maxbin=boardDisplay->hTraceLongPrev->FindBin(maxpos);
  Double_t amplitude=boardDisplay->hTraceLongPrev->GetBinContent(maxbin);




  if (amplitude>fPar->fSinusAmp) amplitude=fPar->fSinusAmp;

  if(fPar->fSlowMotion)  std::cout<< " ---------- Use initial fit amplitude "<<amplitude<<", parameter defines " << fPar->fSinusAmp << std::endl;
  // TODO: get initial value for sinus period from FFT maximum peak:

  TGo4WinCond conny("FFT-Maximumfinder","dummy");
  conny.SetValues(0,2048);
  Double_t fftmax=conny.GetXMax(boardDisplay->hTraceLongFFT);

  if(fPar->fSlowMotion)  std::cout<< " ---------- Got initial fft maximum at "<<fftmax << std::endl;

  Double_t firstperiod=fPar->fSinusPeriod;
  if(fftmax!=0) firstperiod=4096/fftmax; // range of full trace
  if(firstperiod<fPar->fSinusPeriod) firstperiod=fPar->fSinusPeriod;

  if(fPar->fSlowMotion)  std::cout<< " ---------- Got initial period "<<firstperiod<<", parameter defines "<<fPar->fSinusPeriod << std::endl;

  // TODO: get initial value for phase shift by evaluating nulldurchgang of data curve:
    //Int_t firstbin=boardDisplay->hTraceLongPrev->FindBin(boardDisplay->cWindowFFT->GetXLow()); // we start at left edge of our window?
    Int_t firstbin=1; // begin of sample range
    Double_t value=0, oldvalue=0;
    Int_t b=0;
    for(b=firstbin; b<=boardDisplay->hTraceLongPrev->GetNbinsX();++b)
      {
          value=boardDisplay->hTraceLongPrev->GetBinContent(b);
          if(fPar->fSlowMotion)  printf(" ---- phase shift search bin=%d, value=%e ,oldvalue=%e \n", b, value, oldvalue);
          if((value > oldvalue) && (oldvalue < 0) && (value>0)) break;
          oldvalue=value;

      }
    Double_t inphase=b;
    if(inphase<fPar->fSinusPhase) inphase=fPar->fSinusPhase;

    if(fPar->fSlowMotion)  std::cout<< " ---------- Found initial phase shift at "<<inphase << std::endl;




  sinusModel->SetParsValues(amplitude, firstperiod, inphase);

  //sinusModel->SetParRange("T",0.99*firstperiod, 1.01*firstperiod); // allow only small variation of fft found period
   sinusModel->FindPar("T")->SetFixed(kTRUE); // regard frequency as already known from fft
  sinusModel->SetParRange("X0",0.9*inphase, 1.1*inphase); // allow only small variation of phase from first zero crossing
  //sinusModel->FindPar("X0")->SetFixed(kTRUE);

  // do not specify range for amplitude!
  //  sinusModel->SetParRange("Ampl",0.80*amplitude, 1.20*amplitude); // allow only small variation of sinus amplitude
//  baseModel->SetParRange("Ampl",-50,+50); // baseline range






  fSinusFitter->DoActions();
  fSinusFitter->DoActions();
  fSinusFitter->DoActions();
  fSinusFitter->DoActions();

  if(fPar->fSlowMotion) fSinusFitter->PrintResults();

  TH1* h1 = dynamic_cast<TH1*> (fSinusFitter->CreateDrawObject("abc", "data1", kTRUE));
  if (h1!=0) {
       for (int n=0;n<boardDisplay->hTraceLongPrev->GetNbinsX();n++)
         boardDisplay->hTraceLongPrevSinusfit->SetBinContent(n+1, h1->GetBinContent(n+1));
     delete h1;
    }

  // now evalulate the parameters:

  Double_t baseline = baseModel->GetAmplValue();
  Double_t amp = sinusModel->GetAmplValue();
  Double_t period = sinusModel->GetParValue("T");
  Double_t phase = sinusModel->GetParValue("X0");

  Double_t ndf=fSinusFitter->CalculateNDF();
  Double_t chiquadrat=fSinusFitter->CalculateFitFunction();
  if(ndf) chiquadrat=chiquadrat/ndf;

  boardDisplay->hSinusfitAmplitude->Fill(amp);
  boardDisplay->hSinusfitPeriod->Fill(period);
  boardDisplay->hSinusfitPhase->Fill(phase);
  boardDisplay->hSinusfitBaseline->Fill(baseline);
  boardDisplay->hSinusfitChi2->Fill(chiquadrat);

  if(fPar->fSlowMotion)     {
    std::cout<< " ---------- Fit Results: B: "<<baseline<<", A:"<<amp<<", T:"<<period<<", X0:"<<phase << std::endl;
    std::cout<< " ---------chi2/NDF="<<chiquadrat<<std::endl;
  }



}



Double_t THitDetRawProc::CorrectedADCVal(Short_t raw, THitDetBoardDisplay* boardDisplay)
{
  Double_t res = raw;
  Int_t corbin = boardDisplay->hADCCorrection->FindBin(raw);
  Double_t corr = boardDisplay->hADCCorrection->GetBinContent(corbin);
  res += corr;
  return res;
}

