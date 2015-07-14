#include "THitDetRawProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "TGo4Analysis.h"
#include "TGo4Log.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"

#include "THitDetRawEvent.h"
#include "THitDetRawParam.h"
#include "THitDetDisplay.h"

#include "TGo4UserException.h"

extern "C"
{
#include "MbsAPIbase/f_swaplw.h"
}
static unsigned long skipped_events = 0;

/** local definition to optionally swap the data words according to endianness*/
#define HitDetMAYSWAPDATA(src, tgt) \
    (needswap ? f_swaplw(src,1,tgt): *tgt=*src);

/* helper macro for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/
#define  HitDetRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > HitDetRawEvent->fDataCount ) \
{ \
  printf("############ unexpected end of payload for datacount:0x%x, skip event %ld\n", HitDetRawEvent->fDataCount, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

// JAM took this out since the case is clear
//psubevt->PrintMbsSubevent(kTRUE,kTRUE,kTRUE);

/******************************************
 JAM: this one can flood go4 message socket
 #define  HitDetRAW_CHECK_PDATA
 if((pdata - pdatastart) > (opticlen/4)) \
{ \
  GO4_SKIP_EVENT_MESSAGE("############ unexpected end of payload for sfp:%d slave:%d with opticlen:0x%x, skip event\n",sfp_id, device_id, opticlen);\
  continue; \
}
 ******************/
/* this one just to leave internal loops*/
#define  HitDetRAW_CHECK_PDATA_BREAK                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
 break; \
}

//  printf("############ reached end of payload for sfp:%d slave:%d with opticlen:0x%x\n",sfp_id, device_id, opticlen);

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

  InitDisplay(fPar->fTraceLength, fPar->fNumSnapshots);

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
  static Int_t numsnapshots=0;
  static Int_t tracelength=0;

  // since we fill histograms already in BuildEvent, need to check if we must rescale histogram displays:
if((fPar->fNumSnapshots != numsnapshots) || (fPar->fTraceLength != tracelength))
{
  numsnapshots=fPar->fNumSnapshots;
  tracelength=fPar->fTraceLength;
  InitDisplay(tracelength, numsnapshots, kTRUE);


}







  /////////////////////////////////////////////////////////////
  ////// evaluate from buffer header if we need to swap data words later:
  Bool_t needswap = kFALSE;
  s_bufhe* head = source->GetMbsBufferHeader();
  if (head && head->l_free[0] != 1)
    needswap = kTRUE;

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

    // loop over single subevent data:
    while (pdata - psubevt->GetDataField() < lwords)
    {
      if ((unsigned) *pdata == 0xbad00bad)
      {
        GO4_SKIP_EVENT_MESSAGE("**** THitDetRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*pdata));
      }
      else
      {

        // JAM TODO: here is the actual unpacker

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
          // evaluate message type from header:
          Int_t header = 0;
          HitDetMAYSWAPDATA(pdata, &header);
          // not we do not increment pdata here, do this inside msg types
          Int_t mtype = ((header >> 29) & 0x3);

          boardDisplay->hMsgTypes->Fill(mtype);
          switch (mtype)
          {

            case THitDetMsg::MSG_ADC_Direct:    // direct ADC readout
              {
                // message counter
                UShort_t msgcount = ((header >> 20) & 0x3FF);
                THitDetMsgDirect* theMsg = new THitDetMsgDirect(msgcount);
                tracelongcount = msgcount;    // reset trace histogram

                Int_t adcdata[4];
                for (Int_t j = 0; j < 4; ++j)
                  HitDetMAYSWAPDATA(pdata++, (adcdata + j));

                // checkhere if we are already outside allowed range:
                HitDetRAW_CHECK_PDATA

                // decode sample bin data (0-7)
                theMsg->SetBinData(0, ((adcdata[0] >> 8) & 0xFFF));
                theMsg->SetBinData(1, (adcdata[0] & 0xFF) | ((adcdata[1] >> 28) & 0xF));
                theMsg->SetBinData(2, (adcdata[1] >> 16) & 0xFFF);
                theMsg->SetBinData(3, (adcdata[1] >> 4) & 0xFFF);
                theMsg->SetBinData(4, ((adcdata[1] & 0xF) << 8) | ((adcdata[2] >> 24) & 0xFF));
                theMsg->SetBinData(5, (adcdata[2] >> 12) & 0xFFF);
                theMsg->SetBinData(6, adcdata[2] & 0xFFF);
                theMsg->SetBinData(7, (adcdata[3] >> 20) & 0xFFF);

                // here directly evaluate display  TODO: optionally do this in FillDisplays from output event

                TH1* tracesnapshot = 0;
                TH1* tracelong = 0;
                TH1* tracelongsum = 0;
                TH2* trace2d = 0;
                if (snapshotcount < HitDet_MAXSNAPSHOTS)
                {
                  tracesnapshot = boardDisplay->hTraceSnapshots[0][snapshotcount];
                  trace2d = boardDisplay->hTraceSnapshot2d[0];
                }
                if (tracelongcount < HitDet_MAXTRACELONG)
                {
                  tracelong = boardDisplay->hTraceLong;
                  tracelongsum = boardDisplay->hTraceLongSum;
                  if (tracelongcount == 0)
                    tracelong->Reset("");
                }

                for (Int_t k = 0; k < 8; ++k)
                {
                  UShort_t val = theMsg->GetBinData(k);
                  if (tracesnapshot)
                    tracesnapshot->SetBinContent(k + 1, val);
                  if (trace2d)
                    trace2d->Fill(k, snapshotcount, val);
                  if (tracelong)
                    tracelong->SetBinContent(1 + k + (8 * tracelongcount), val);
                  if (tracelongsum)
                    tracelongsum->AddBinContent(1 + k + (8 * tracelongcount), val);
                  boardDisplay->hTrace[0]->SetBinContent(1 + k, val);
                  boardDisplay->hTraceSum[0]->AddBinContent(1 + k, val);

                }

                theBoard->AddMessage(theMsg, 0);    // direct ADC messages assigned to channel 0
              }
              break;

            case THitDetMsg::MSG_ADC_Event:    // triggered event read out
              {
                UChar_t channel = ((header >> 28) & 0x3);
                UChar_t size12bit = ((header >> 20) & 0x3F);
                UChar_t size32bit = 1 + size12bit * 3 / 8;    // account header word again in evdata
                if ((pdata - pdatastart) + size32bit > lwords)
                  GO4_SKIP_EVENT_MESSAGE(
                      "ASIC Event header error: 12 bit size %d does not fit into mbs subevent buffer of restlen %d words", size12bit, lwords - (pdata - pdatastart));

                Int_t evdata[size32bit];
                for (Int_t j = 0; j < size32bit; ++j)
                  HitDetMAYSWAPDATA(pdata++, (evdata + j));

                THitDetMsgEvent* theMsg = new THitDetMsgEvent(channel);
                theMsg->SetEpoch(((evdata[0] & 0xFFFFF) << 4) | ((evdata[1] >> 28) & 0xF));
                theMsg->SetTimeStamp((evdata[1] >> 16) & 0xFFF);

                // now decode 12 bit samples inside mbs data words:Ä
                Int_t binlen = size12bit - 3;    // number of sample bins (should be 8,16, or 32)
                if(binlen>32)
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
                  Int_t dix_end =   (Int_t) j_end / 32;    // data index containing last bit of sample
                  UChar_t k_start = 32 - (j_start - 32 * dix_start);    //  start bit number in evdata word
                  UChar_t k_end = 32 - (j_end - 32 * dix_end);    // end bit number in evdata word
                  if (dix_start == dix_end)
                  {
                    // easy case, sample is inside one evdata word:
                    val = (evdata[dixoffset+dix_start] >> k_end) & 0xFFF;
                  }
                  else if (dix_end == dix_start + 1)
                  {
                    // spanning over 2 evdata words:
                    UChar_t mask_start = 0, mask_end = 0;
                    for (UChar_t b = 0; b < k_start; ++b)
                      mask_start |= (1 << b);

                    for (UChar_t b = 0; b < (32 - k_end); ++b)
                      mask_end |= (1 << b);
                    val = (evdata[dixoffset+ dix_start] << (12 - k_start)) & mask_start;
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
                  UShort_t val = theMsg->GetTraceData(bin);
                  if (tracesnapshot)
                    tracesnapshot->SetBinContent(bin + 1, val);
                  if (trace2d)
                    trace2d->Fill(bin, snapshotcount, val);
                  boardDisplay->hTrace[0]->SetBinContent(1 + bin, val);
                  boardDisplay->hTraceSum[0]->AddBinContent(1 + bin, val);

                }

                theBoard->AddMessage(theMsg, channel);


          }

          break;

          case THitDetMsg::MSG_Wishbone:
          // wishbone response (error message)
          {
            THitDetMsgWishbone* theMsg = new THitDetMsgWishbone(header);
            Int_t address=0;
            pdata++; //account header already processed above
            HitDetMAYSWAPDATA(pdata++, &address);
            theMsg->SetAddress(address);
            // here we could take adress data contents.
            // TODO: find out how many data words follow (1..4 bytes)?
            // assume that there are no wishbone messages here except errors!
            boardDisplay->hWishboneAck->Fill(theMsg->GetAckCode());
            boardDisplay->hWishboneSource->Fill(theMsg->GetSource());


            theBoard->AddMessage(theMsg, 0); // wishbone messages accounted for channel 0
          }
          break;

          default:
          printf("############ found unknown message type 0x%x, skip event %ld\n", mtype, skipped_events++);\
          GO4_SKIP_EVENT
          break;
        }

      };    // switch
      snapshotcount++;

    }    // if event is not bad

  }    // while pdata - psubevt->GetDataField() <lwords

}    // while subevents

//

UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
HitDetRawEvent->SetValid(kTRUE);// to store
return kTRUE;
}

Bool_t THitDetRawProc::UpdateDisplays()
{
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




}    // i board



return kTRUE;
}

