#include "TQFWRawProc.h"

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

#include "TQFWRawEvent.h"
#include "TQFWRawParam.h"

#include "TGo4UserException.h"

static unsigned long skipped_events=0;
static unsigned long missing_events=0;

/* helper macro for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/
#define  QFWRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
  printf("############ unexpected end of payload for sfp:%d slave:%d with opticlen:0x%x, skip event %ld\n",sfp_id, device_id, opticlen, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

// JAM took this out since the case is clear
//psubevt->PrintMbsSubevent(kTRUE,kTRUE,kTRUE);

/******************************************
 JAM: this one can flood go4 message socket
#define  QFWRAW_CHECK_PDATA
if((pdata - pdatastart) > (opticlen/4)) \
{ \
  GO4_SKIP_EVENT_MESSAGE("############ unexpected end of payload for sfp:%d slave:%d with opticlen:0x%x, skip event\n",sfp_id, device_id, opticlen);\
  continue; \
}
******************/
/* this one just to leave internal loops*/
#define  QFWRAW_CHECK_PDATA_BREAK                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
 break; \
}


//  printf("############ reached end of payload for sfp:%d slave:%d with opticlen:0x%x\n",sfp_id, device_id, opticlen);


//***********************************************************
TQFWRawProc::TQFWRawProc() :
    TGo4EventProcessor(),fbOffsetReady(kFALSE)
{
}

//***********************************************************
// this one is used in standard factory
TQFWRawProc::TQFWRawProc(const char* name) :
    TGo4EventProcessor(name),fbOffsetReady(kFALSE)
{
  TGo4Log::Info("TQFWRawProc: Create instance %s", name);

  SetMakeWithAutosave(kTRUE);
//// init user analysis objects:

  fPar = dynamic_cast<TQFWRawParam*>(MakeParameter("QFWRawParam", "TQFWRawParam", "set_QFWRawParam.C"));
  if (fPar)
    fPar->SetConfigBoards();

  if (fPar->fSimpleCompensation)
    TGo4Log::Info("Apply simple compensation - at least 5 values required");

  for (unsigned i = 0; i < TQFWRawEvent::fgConfigQFWBoards.size(); ++i)
  {
    UInt_t uniqueid = TQFWRawEvent::fgConfigQFWBoards[i];
    fBoards.push_back(new TQFWBoardDisplay(uniqueid));
  }
  missing_events=0;
//InitDisplay(PEXOR_QFWSLICES);
}

//***********************************************************
TQFWRawProc::~TQFWRawProc()
{
  TGo4Log::Info("TQFWRawProc: Delete instance");
  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    delete fBoards[i];
  }
}

/* access to histogram set for current board id*/
TQFWBoardDisplay* TQFWRawProc::GetBoardDisplay(Int_t uniqueid)
{
  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    TQFWBoardDisplay* theDisplay = fBoards[i];
    if (uniqueid == theDisplay->GetDevId())
      return theDisplay;
  }
  return 0;
}

void TQFWRawProc::InitDisplay(int timeslices, Bool_t replace)
{
  cout << "**** TQFWRawProc: Init Display for " << timeslices << " time slices. " << endl;
//   if(replace) //TGo4Analysis::Instance()->
//         SetMakeWithAutosave(kFALSE);

  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    fBoards[i]->InitDisplay(timeslices, replace);
  }

}

//-----------------------------------------------------------
// event function
Bool_t TQFWRawProc::BuildEvent(TGo4EventElement* target)
{
// called by framework from TQFWRawEvent to fill it
  Bool_t isOffsetTrigger=kFALSE;
  QFWRawEvent = (TQFWRawEvent*) target;
  QFWRawEvent->SetValid(kFALSE);    // not store
  Int_t triggersum = 0;    // sums up all "software trigger" channels (for free running acquired data)
  TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
  if (source == 0)
  {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  UShort_t triggertype=source->GetTrigger();

  if ((triggertype != fPar->fFrontendOffsetTrigger) && (triggertype > 11))
  {
    // frontend offset trigger can be one of these, we let it through to unpacking loop
    //cout << "**** TQFWRawProc: Skip trigger event" << endl;
    GO4_SKIP_EVENT_MESSAGE( "**** TQFWRawProc: Skip event of trigger type 0x%x",
                                     triggertype);
    //return kFALSE; // this would let the second step execute!
  }

// first we fill the TQFWRawEvent with data from MBS source
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


      if (triggertype == fPar->fFrontendOffsetTrigger)
       {

           // get update of qfw offsets as measured by frontends
         cout << "**** TQFWRawProc: Use frontend offset trigger "<< source->GetTrigger() << endl;
         if ((*pdata & 0xff) != 0x42)    // regular channel data
             {
               GO4_SKIP_EVENT_MESSAGE(
                   "**** TQFWRawProc: Wrong frontend offset format 0x%x - 0x42 are expected", (*pdata & 0xff));
               // avoid that we run second step on invalid raw event!
             }
            //Int_t* pdatastart = pdata;    // remember begin of optic payload data section
             unsigned trig_type   = (*pdata & 0xf00) >> 8;
             unsigned sfp_id = (*pdata & 0xf000) >> 12;
             unsigned device_id = (*pdata & 0xff0000) >> 16;
             cout << "**** TQFWRawProc: Found trigger type "<< trig_type<<", sfp:"<<sfp_id<<", slave:"<<device_id << endl;
             if(trig_type !=triggertype)
             {
               GO4_SKIP_EVENT_MESSAGE(
                                  "**** TQFWRawProc: Offset header trigger type 0x%x does not match MBS trigger 0x%x",
                                    trig_type, triggertype);
               // just in case, but never come here!
             }
             pdata++;
             UInt_t brdid = fPar->fBoardID[sfp_id][device_id];
             TQFWBoard* theBoard = QFWRawEvent->GetBoard(brdid);
             if (theBoard == 0)
             {
               GO4_SKIP_EVENT_MESSAGE(
                   "Configuration error: Board id %d does not exist as subevent, sfp:%d device:%d", brdid, sfp_id, device_id);

               return kFALSE;
             }
             TQFWBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
             if (boardDisplay == 0)
             {
               GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
               return kFALSE;
             }
             boardDisplay->hQFWOffsets->Reset("");
             // now copy to board offset values:
               for(unsigned int c=0; c<PEXOR_QFWCHANS; ++c)
               {
                 Int_t value=*pdata++;
                 cout << "**** TQFWRawProc: Got frontend offset "<< value <<" for channel "<<c << endl;
                 theBoard->SetOffset(c,value);
                 //boardDisplay->hQFWOffsets->Fill(c,value);
               }
               isOffsetTrigger=kTRUE; // suppress evaluating offset from loop data later
               fbOffsetReady=kTRUE;
               // no check at end of payload, either we find new correct header or subevent is over
             continue;
       } // end  if (source->GetTrigger() == fPar->fFrontendOffsetTrigger)



////////////////////////////////////////////////////////////////////////////////
/// Here begin regular qfw data from token readout:


      if ((*pdata & 0xffff0000) == 0xadd00000)    // we have padding word (initial data of sfp, skip it:)
      {
        Int_t dma_padd = (*pdata & 0xff00) >> 8;
        Int_t cnt(0);
        while (cnt < dma_padd)
        {
          if ((*pdata & 0xffff0000) != 0xadd00000)
          {
            //TGo4Log::Error("Wrong padding format - missing add0");
            GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong padding format - missing add0");
            // avoid that we run second step on invalid raw event!
            //return kFALSE;
          }
          if (((*pdata & 0xff00) >> 8) != dma_padd)
          {
            //TGo4Log::Error("Wrong padding format - 8-15 bits are not the same");
            GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong padding format - 8-15 bits are not the same");
            // avoid that we run second step on invalid raw event!
            //return kFALSE;
          }
          if ((*pdata & 0xff) != cnt)
          {
            //TGo4Log::Error("Wrong padding format - 0-7 bits not as expected");
            GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong padding format - 0-7 bits not as expected");
            // avoid that we run second step on invalid raw event!
            //return kFALSE;
          }
          pdata++;
          cnt++;
        }
        continue;
      }
      else if ((unsigned) *pdata == 0xbad00bad)
      {
        GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*pdata));
      }
      else if ((*pdata & 0xff) != 0x34)    // regular channel data
      {
        //GO4_STOP_ANALYSIS_MESSAGE("Wrong optic format - 0x34 are expected0-7 bits not as expected");
        //TGo4Log::Error("Wrong optic format 0x%x - 0x34 are expected0-7 bits not as expected", (*pdata & 0xff));
        GO4_SKIP_EVENT_MESSAGE(
            "**** TQFWRawProc: Wrong optic format 0x%x - 0x34 are expected - 0-7 bits not as expected", (*pdata & 0xff));
        // avoid that we run second step on invalid raw event!
        //return kFALSE;
      }

      Int_t* pdatastart = pdata;    // remember begin of optic payload data section
      // unsigned trig_type   = (*pdata & 0xf00) >> 8;
      unsigned sfp_id = (*pdata & 0xf000) >> 12;
      unsigned device_id = (*pdata & 0xff0000) >> 16;
      // unsigned channel_id  = (*pdata & 0xff000000) >> 24;
      pdata++;

      Int_t opticlen = *pdata++;
      if (opticlen > lwords * 4)
      {
        //TGo4Log::Error("Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
        GO4_SKIP_EVENT_MESSAGE(
            "**** TQFWRawProc: Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
        // avoid that we run second step on invalid raw event!
        //return kFALSE;
      }
      QFWRAW_CHECK_PDATA;
      int eventcounter = *pdata;



      //TGo4Log::Info("Internal Event number 0x%x", eventcounter);
      // board id calculated from SFP and device id:
      UInt_t brdid = fPar->fBoardID[sfp_id][device_id];
      TQFWBoard* theBoard = QFWRawEvent->GetBoard(brdid);
      if (theBoard == 0)
      {
        GO4_SKIP_EVENT_MESSAGE(
            "Configuration error: Board id %d does not exist as subevent, sfp:%d device:%d", brdid, sfp_id, device_id);

        return kFALSE;
      }

      if(fPar->fCheckEventSequence)
            {
                if(theBoard->GetLastEventNumber()>=0 && eventcounter!= theBoard->GetLastEventNumber() +1)
                {
                  Int_t delta=(eventcounter - (theBoard->GetLastEventNumber() +1));
                  printf("***** event sequence number mismatch at board %d: this event %d, last event %d, missing events:%d, total missing:%ld\n",
                      brdid, eventcounter, theBoard->GetLastEventNumber(), delta, missing_events+=delta);\
                }
            }


      TQFWBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
      if (boardDisplay == 0)
      {
        GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
        return kFALSE;
      }

      if(fPar->fCheckEventSequence)
                  {
                    if(theBoard->GetLastEventNumber()>=0)
                      {
                        Int_t ediff=eventcounter - theBoard->GetLastEventNumber();
                        boardDisplay->hEventDelta->Fill(ediff);
                      }
                    theBoard->SetLastEventNumber(eventcounter);
                  }


      pdata += 1;
      QFWRAW_CHECK_PDATA;
      theBoard->fQfwSetup = *pdata;
      //TGo4Log::Info("QFW SEtup %d", theBoard->fQfwSetup);
      for (int j=0; j<4;++j)
      {
        QFWRAW_CHECK_PDATA_BREAK;
        pdata++;

      }
      //pdata += 4;
      QFWRAW_CHECK_PDATA;
      for (int loop = 0; loop < theBoard->getNElements(); loop++)
      {
        TQFWLoop* theLoop = theBoard->GetLoop(loop);
        if (theLoop == 0)
        {
          TGo4Log::Error("Configuration error: Loop index %d  emtpy subevent for boardid:%d", loop, brdid);
          continue;
        }

        theLoop->fQfwSetup = theBoard->fQfwSetup;    // propagate setup id to subevent
        QFWRAW_CHECK_PDATA_BREAK;
        theLoop->fQfwLoopSize = *pdata++;
        theLoop->fHasData=kTRUE; // dynamic rebinning of timeslices only if we really have valid event

//      if (theLoop->fQfwLoopSize >= PEXOR_QFWSLICES)
//      {
//        TGo4Log::Error("TQFWRawProc: found very large slice size %d max %d -  Please check set up!",
//            theLoop->fQfwLoopSize, PEXOR_QFWSLICES);
//	 return kFALSE;
//      }

      }    // first loop loop

      QFWRAW_CHECK_PDATA;
      for (int loop = 0; loop < theBoard->getNElements(); loop++)
      {
        TQFWLoop* theLoop = theBoard->GetLoop(loop);
        QFWRAW_CHECK_PDATA_BREAK;
        theLoop->fQfwLoopTime = *pdata++;
      }    // second loop loop

      // TODO: are here some useful fields
      for (int j=0; j<21;++j)
      {
        QFWRAW_CHECK_PDATA_BREAK;
        pdata++;

      }
      //pdata += 21; // need to check for each increment if we are outside this slave's payload!
      QFWRAW_CHECK_PDATA;
      /** All loops X slices/loop X channels */
      for (int loop = 0; loop < theBoard->getNElements(); loop++)
      {
        TQFWLoop* loopData = theBoard->GetLoop(loop);
        for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
          for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
          {
            QFWRAW_CHECK_PDATA_BREAK;
            Double_t value = (Double_t) *pdata++;

            if(fPar->fUseFrontendOffsets)
            {

              if(fPar->fFrontendOffsetLoop!=loop) // supress correction of frontend offset raw data if dynamic mode is set!
                {
                // we account frontend measured offset already here
                // this emulates future mode where offset is corrected already in poland
                Double_t correction=theBoard->GetOffset(ch) * loopData->GetMicroSecsPerTimeSlice()/1.0e+6;
                // <- offset is measured for 1 second, evaluate for actual time slice period

                value -= correction; // JAM2016: here we may lose precision  due to integer clipping!!!
                }
            }

            loopData->fQfwTrace[ch].push_back(value);

            if (fPar->fSelectTriggerEvents && ((UInt_t) fPar->fTriggerBoardID == brdid))
            {
              /////////// software trigger section
              // begin selecting good, bad and ugly events for free running data
              if (ch >= fPar->fTriggerFirstChannel || ch <= fPar->fTriggerLastChannel)
                triggersum += value;
            }    // if (fPar->fSelectTriggerEvents)

            //printf("loop %d slice %d ch %d = %d\n", loop, sl ,ch ,value);
          }
      }    //loop

      QFWRAW_CHECK_PDATA;
      /* errorcount values: - per QFW CHIPS*/
      for (int qfw = 0; qfw < PEXOR_QFWNUM; ++qfw)
      {
        QFWRAW_CHECK_PDATA_BREAK;
        theBoard->SetErrorScaler(qfw, (UInt_t) (*pdata++));
        //printf("EEEEEE Error counter qfw %d =0x%x\n", qfw , *(pdata-1));
      }
      QFWRAW_CHECK_PDATA;

      // skip filler words at the end of gosip payload:
      while (pdata - pdatastart <= (opticlen / 4))    // note that trailer is outside opticlen!
      {
        //printf("######### skipping word 0x%x\n ",*pdata);
        pdata++;
      }

      // crosscheck if trailer word matches eventcounter header
      if (*pdata != eventcounter)
      {
        TGo4Log::Error("Eventcounter 0x%x does not match trailing word 0x%x at position 0x%x!", eventcounter, *pdata,
            (opticlen / 4));
        pdata++;
        continue;
      }
      //TGo4Log::Info("!!!!!!!!!!! found  trailer 0x%x",*pdata);
      pdata++;
      QFWRawEvent->fSequenceNumber = eventcounter;

    }    // while pdata - psubevt->GetDataField() <lwords

  }    // while subevents

  if (fPar->fSelectTriggerEvents)
  {
    //TGo4Log::Info("Triggersum of event %d is %d", QFWRawEvent->fSequenceNumber, triggersum);
    if ((triggersum < fPar->fTriggerHighThreshold) && (triggersum > fPar->fTriggerLowThreshold))
    {
      GO4_SKIP_EVENT_MESSAGE("Skip event of seqnr %d with triggersum %d!", QFWRawEvent->fSequenceNumber, triggersum);
      // debug
      // GO4_SKIP_EVENT; // no debug mode
    }
  }
  Bool_t offsetreadyold=fbOffsetReady;
  if((fPar->fFrontendOffsetLoop<0) || fbOffsetReady)
    // always fill, but in dynamic offset loop do not fill unless we have valid offset correction (for Sven)
      {
        FillDisplays();    // we only fill histograms for the events that are selected by trigger condition
      }
  // here refresh dynamic offsets from defined loop
  // this is done _after_ displaying the previous offset and the data corrected from this
  if((fPar->fFrontendOffsetLoop>=0) && !isOffsetTrigger)
  {
    //cout << "**** TQFWRawProc: RefreshOffsetFromLoop for loop"<< fPar->fFrontendOffsetLoop<< endl;
    RefreshOffsetFromLoop(fPar->fFrontendOffsetLoop);
  }
  // do not fill histograms of second analysis step with data of uncorrected first event:
  if((fPar->fFrontendOffsetLoop>=0) && !offsetreadyold)  GO4_SKIP_EVENT_MESSAGE("Skip event of seqnr %d because first offset!", QFWRawEvent->fSequenceNumber);
  QFWRawEvent->SetValid(kTRUE);    // to store
  return kTRUE;
}

Bool_t TQFWRawProc::FillDisplays()
{
  for (unsigned i = 0; i < TQFWRawEvent::fgConfigQFWBoards.size(); ++i)
  {
    UInt_t brdid = TQFWRawEvent::fgConfigQFWBoards[i];
    TQFWBoard* theBoard = QFWRawEvent->GetBoard(brdid);
    if (theBoard == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("FillDisplays Configuration error: Board id %d does not exist!", brdid);
      //return kFALSE;
    }
    TQFWBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE(
          "FillDisplays Configuration error: Board id %d does not exist as histogram display set!", brdid);
      //return kFALSE;
    }

    // check first if we have to rebin histograms:
    Bool_t rebinned = kFALSE;
    for (int loop = 0; loop < theBoard->getNElements(); loop++)
    {
      TQFWBoardLoopDisplay* loopDisplay = boardDisplay->GetLoopDisplay(loop);
      if (loopDisplay == 0)
      {
        TGo4Log::Error("Configuration error: Loop index %d  empty histogram set for boardid:%d", loop, brdid);
        continue;
      }
      TQFWLoop* theLoop = theBoard->GetLoop(loop);
      // optionally rescale histograms of this loop:
      if (theLoop->fHasData && (loopDisplay->GetTimeSlices() != theLoop->fQfwLoopSize))
      {
        loopDisplay->InitDisplay(theLoop->fQfwLoopSize, kTRUE);
        rebinned = kTRUE;
      }
    }
    if (rebinned)
      boardDisplay->InitDisplay(-1, kTRUE);    // rebin overview histograms with true timeslices of subloops

    // now fill histograms from already unpacked data in ouput event:
    boardDisplay->hQFWRaw2DTrace->Reset("");
    int loopoffset = 0;
    /** All loops X slices/loop X channels */
    for (int loop = 0; loop < theBoard->getNElements(); loop++)
    {
      TQFWLoop* loopData = theBoard->GetLoop(loop);
      TQFWBoardLoopDisplay* loopDisplay = boardDisplay->GetLoopDisplay(loop);
      loopDisplay->hQFWRawTrace->Reset("");

      for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
        for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
        {
          Double_t value = loopData->fQfwTrace[ch].at(sl);
          //printf("loop %d slice %d ch %d = %d\n", loop, sl ,ch ,value);

          if (!fPar->fSimpleCompensation)
          {
            loopDisplay->hQFWRawTrace->SetBinContent(ch + 1 + sl * PEXOR_QFWCHANS, value);
            boardDisplay->hQFWRaw2DTrace->Fill(loopoffset + sl, ch, value);

            loopDisplay->hQFWRaw->AddBinContent(ch + 1 + sl * PEXOR_QFWCHANS, value);
            boardDisplay->hQFWRaw2D->Fill(loopoffset + sl, ch, value);
            //printf("      loopoffset= %d\n", loopoffset);




          }
        }
      if (fPar->fSimpleCompensation)
      {
        for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
        {

          if (loopData->fQfwLoopSize < 5)
            continue;

          Double_t sum = 0;
          for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
            sum += loopData->fQfwTrace[ch].at(sl);
          sum = sum / loopData->fQfwLoopSize;

          for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
          {
            Double_t value = loopData->fQfwTrace[ch].at(sl) - sum;

            loopDisplay->hQFWRawTrace->SetBinContent(ch + 1 + sl * PEXOR_QFWCHANS, value);
            boardDisplay->hQFWRaw2DTrace->Fill(loopoffset + sl, ch, value);

            loopDisplay->hQFWRaw->AddBinContent(ch + 1 + sl * PEXOR_QFWCHANS, value);
            boardDisplay->hQFWRaw2D->Fill(loopoffset + sl, ch, value);
          }    // sl
        }    //ch
      }
      loopoffset += loopData->fQfwLoopSize;
    }    //loop

    //boardDisplay->hQFWRawErrTr->Reset("");
    /* errorcount values: - per QFW CHIPS*/
    for (int qfw = 0; qfw < PEXOR_QFWNUM; ++qfw)
    {
      boardDisplay->hQFWRawErr->SetBinContent(1 + qfw, theBoard->GetErrorScaler(qfw));
      //boardDisplay->hQFWRawErrTr->SetBinContent(1 + qfw, theBoard->GetErrorScaler(qfw));
      //printf("FFFFFF Fill Error counter qfw %d with 0x%x\n", qfw , theBoard->GetErrorScaler(qfw));

    }
    // need to update this here, since histograms are initialized after offset is retrieved!
    for (int c = 0; c < PEXOR_QFWCHANS; ++c)
      boardDisplay->hQFWOffsets->SetBinContent(c+1,theBoard->GetOffset(c));


  }    // i board
  return kTRUE;
}

Bool_t TQFWRawProc::RefreshOffsetFromLoop(UInt_t loop)
{
  if (loop >= PEXOR_QFWLOOPS)
    return kFALSE;
  for (unsigned i = 0; i < TQFWRawEvent::fgConfigQFWBoards.size(); ++i)
  {
    UInt_t brdid = TQFWRawEvent::fgConfigQFWBoards[i];
    TQFWBoard* theBoard = QFWRawEvent->GetBoard(brdid);
    if (theBoard == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("UpdateOffsetFromLoop Configuration error: Board id %d does not exist!", brdid);
      return kFALSE;
    }

    for (int l = 0; l < theBoard->getNElements(); l++)
    {
      if (loop == (unsigned) l)
      {
        TQFWLoop* loopData = theBoard->GetLoop(loop);
        int sl = 0;    // always use first slice of loop only
        for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
        {
          Double_t value = loopData->fQfwTrace[ch].at(sl);
          Double_t offset=value * 1.0e+6 /loopData->GetMicroSecsPerTimeSlice();
                         // <- offset is normalized here to 1 seconds measurement time!
          theBoard->SetOffset(ch, offset);
          //cout << "**** TQFWRawProc: Refreshed from slice frontend offset " << offset << " for channel " << ch << endl;
        }
        break;
      }
    }
  }
fbOffsetReady=kTRUE;
return kTRUE;
}

