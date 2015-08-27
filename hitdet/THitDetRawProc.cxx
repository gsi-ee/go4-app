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

#include "THitDetRawEvent.h"
#include "THitDetRawParam.h"
#include "THitDetDisplay.h"

#include "TGo4UserException.h"

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
    TGo4EventProcessor(name)
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

  UInt_t snapshotcount = 0;    // counter for trace snapshot display
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
                if (snapshotcount < HitDet_MAXSNAPSHOTS)
                {
                  tracesnapshot = boardDisplay->hTraceSnapshots[0][snapshotcount];
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
                    // begin of new trace, provide FFT of previous one here:
                    DoFFT(boardDisplay);
                    boardDisplay->hTraceLongPrev->Reset("");
                    boardDisplay->hTraceLongPrev->Add(tracelong);    // copy previous full trace to buffer histogram

                    // now provide corrected trace long:
                    boardDisplay->hTraceLongPrevCorrected->Reset("");
                    boardDisplay->hTraceLongPrevCorrected->Add(tracelongcorr);    // copy previous full trace to buffer histogram

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
                    trace2d->Fill(k, 0.5 + snapshotcount, val);

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
              }    // if !skipmessage
            }
            break;

          case THitDetMsg::MSG_ADC_Event:    // triggered event read out
            {
              UChar_t channel = ((header >> 28) & 0x3);
              UChar_t size12bit = ((header >> 20) & 0x3F);
              UChar_t size32bit = 1 + size12bit * 3 / 8;    // account header word again in evdata
              //printf("MSG_ADC_Event channel:%d size12:%d size32:%d\n",channel,size12bit,size32bit);
              if ((pdata - pdatastart) + size32bit > lwords)
                GO4_SKIP_EVENT_MESSAGE(
                    "ASIC Event header error: 12 bit size %d does not fit into mbs subevent buffer of restlen %d words", size12bit, lwords - (pdata - pdatastart));

              Int_t evdata[size32bit];
              for (Int_t j = 0; j < size32bit; ++j)
              {
                HitDetRAW_CHECK_PDATA;
                HitDetMSG_CHECK_PDATA;
                evdata[j] = *pdata++;
              }
              THitDetMsgEvent* theMsg = new THitDetMsgEvent(channel);
              theMsg->SetEpoch(((evdata[0] & 0xFFFFF) << 4) | ((evdata[1] >> 28) & 0xF));
              theMsg->SetTimeStamp((evdata[1] >> 16) & 0xFFF);

              // now decode 12 bit samples inside mbs data words:Ä
              Int_t binlen = size12bit - 3;    // number of sample bins (should be 8,16, or 32)
              if (binlen > 32)
                GO4_SKIP_EVENT_MESSAGE("ASIC Event header error: bin length %d exceeds maximum 32", binlen);

              UShort_t val = 0;
              Int_t dixoffset = 1;    // actual sample data begins after header and timestamp,
              // j counts global bit number in stream, j_start is first in stream, j_end is last
              // k is local bit number in evdata word (lsb=0)
              UChar_t j_start = 16;    // begin of first 12 bit sample is after timestamp
              for (Int_t bin = 0; bin < binlen; ++bin)
              {
                UChar_t j_end = j_start + 12;
                Int_t dix_start = (Int_t) j_start / 32;    // data index containing first bit of sample
                Int_t dix_end = (Int_t) j_end / 32;    // data index containing last bit of sample
                UChar_t k_start = 32 - (j_start - 32 * dix_start);    //  start bit number in evdata word
                UChar_t k_end = 32 - (j_end - 32 * dix_end);    // end bit number in evdata word
                if (dix_start == dix_end)
                {
                  // easy case, sample is inside one evdata word:
                  val = (evdata[dixoffset + dix_start] >> k_end) & 0xFFF;
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

                }
                else
                {
                  // never come here
                  GO4_STOP_ANALYSIS_MESSAGE(
                      "NEVER COME HERE: mismatch of evsample indices - dix_end:%d and dix_start:%d", dix_end, dix_start)

                }
                theMsg->SetTraceData(bin, val);
              }    // for bin

              // here do simple histogramming of traces:

              TH1* tracesnapshot = 0;
              TH2* trace2d = 0;
              if (snapshotcount < HitDet_MAXSNAPSHOTS)
              {
                tracesnapshot = boardDisplay->hTraceSnapshots[channel][snapshotcount];
                trace2d = boardDisplay->hTraceSnapshot2d[channel];
              }
              for (Int_t bin = 0; bin < binlen; ++bin)
              {
                Short_t val = theMsg->GetTraceData(bin);
                // convert raw data to signed 8bit (2 complement) representation:
                if (val > 0x7FF)
                  val = val - 0x1000;

                if (tracesnapshot)
                  tracesnapshot->SetBinContent(bin + 1, val);
                if (trace2d)
                  trace2d->Fill(bin, val, snapshotcount);

                if (channel >= HitDet_CHANNELS)
                {
                  printf("MSG_ADC_Event channel:%d out of range %d\n", channel, HitDet_CHANNELS);
                }
                else
                {
                  boardDisplay->hTrace[channel]->SetBinContent(1 + bin, val);
                  boardDisplay->hTraceSum[channel]->AddBinContent(1 + bin, val);
                }

                // value histograms:
                boardDisplay->hADCValues->Fill(val);

              }

              theBoard->AddMessage(theMsg, channel);

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
            printf("############ found unknown message type 0x%x, skip event %ld\n", mtype, skipped_events++);
            GO4_SKIP_EVENT
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
        snapshotcount++;
      }    // while ((pdata - pdatastart) < HitDetRawEvent->fDataCount)

    }    // while pdata - psubevt->GetDataField() <lwords

  }    // while subevents

//

  UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
  HitDetRawEvent->SetValid(kTRUE);    // to store
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
        dnl = delta / mean;//dnl = TMath::Abs(delta / mean);
      boardDisplay->hADCNonLinDiff->SetBinContent(bix + 1, dnl);
      if (mean)
        inl += delta / mean;
      boardDisplay->hADCNonLinInt->SetBinContent(bix + 1, inl);
      // calibrate for ADC nonlinearity corrections:
      if (fPar->fDoCalibrate)
      {
        corr=inl; // this is point to evaluate other kind of correction optionally
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
    Int_t Npart = 0;
    for (Int_t ix = 0; ix < N; ++ix)
    {
      if (boardDisplay->cWindowFFT->Test(ix))
      {
        in[ix] = boardDisplay->hTraceLongPrev->GetBinContent(ix + 1);
        Npart++;
      }
    }
    DoFilter(in, Npart);
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(in);
    thefft->Transform();
    for (Int_t i = 0; i < Npart; i++)
    {
      thefft->GetPointComplex(i, re, im);
      boardDisplay->hTracePartFFT->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
    }

    // finally partial fft from window at corrected trace:
    boardDisplay->hTracePartCorrectedFFT->Reset("");
    Npart = 0;
    for (Int_t ix = 0; ix < N; ++ix)
    {
      if (boardDisplay->cWindowFFT->Test(ix))
      {
        in[ix] = boardDisplay->hTraceLongPrevCorrected->GetBinContent(ix + 1);
        Npart++;
      }
    }
    DoFilter(in, Npart);
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(in);
    thefft->Transform();
    for (Int_t i = 0; i < Npart; i++)
    {
      thefft->GetPointComplex(i, re, im);
      boardDisplay->hTracePartCorrectedFFT->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
    }

    delete[] in;
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

Double_t THitDetRawProc::CorrectedADCVal(Short_t raw, THitDetBoardDisplay* boardDisplay)
{
  Double_t res = raw;
  Int_t corbin = boardDisplay->hADCCorrection->FindBin(raw);
  Double_t corr = boardDisplay->hADCCorrection->GetBinContent(corbin);
  res += corr;
  return res;
}

