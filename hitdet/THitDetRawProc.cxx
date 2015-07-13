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

#include "TGo4UserException.h"

static unsigned long skipped_events=0;

/* helper macro for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/
#define  HitDetRAW_CHECK_PDATA                                    \
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

void THitDetRawProc::InitDisplay(int timeslices, Int_t numsnapshots, Bool_t replace)
{
  cout << "**** THitDetRawProc: Init Display for " << timeslices << " time slices, snapshots="<<numsnapshots << endl;
//   if(replace) //TGo4Analysis::Instance()->
//         SetMakeWithAutosave(kFALSE);

  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    fBoards[i]->InitDisplay(timeslices, numsnapshots, replace);
  }

}

//-----------------------------------------------------------
// event function
Bool_t THitDetRawProc::BuildEvent(TGo4EventElement* target)
{
// called by framework from THitDetRawEvent to fill it
  HitDetRawEvent = (THitDetRawEvent*) target;
  HitDetRawEvent->SetValid(kFALSE);    // not store
  Int_t triggersum = 0;    // sums up all "software trigger" channels (for free running acquired data)
  TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
  if (source == 0)
  {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  UShort_t triggertype=source->GetTrigger();

  if (triggertype > 11)
  {
    // frontend offset trigger can be one of these, we let it through to unpacking loop
    //cout << "**** THitDetRawProc: Skip trigger event" << endl;
    GO4_SKIP_EVENT_MESSAGE( "**** THitDetRawProc: Skip event of trigger type 0x%x",
                                     triggertype);
    //return kFALSE; // this would let the second step execute!
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

    // loop over single subevent data:
    while (pdata - psubevt->GetDataField() < lwords)
    {


      // JAM TODO: here is the actual unpacker


////////////////////////////////////////////////////////////////////////////////
/// Here begin regular qfw data from token readout:


//      if ((*pdata & 0xffff0000) == 0xadd00000)    // we have padding word (initial data of sfp, skip it:)
//      {
//        Int_t dma_padd = (*pdata & 0xff00) >> 8;
//        Int_t cnt(0);
//        while (cnt < dma_padd)
//        {
//          if ((*pdata & 0xffff0000) != 0xadd00000)
//          {
//            //TGo4Log::Error("Wrong padding format - missing add0");
//            GO4_SKIP_EVENT_MESSAGE("**** THitDetRawProc: Wrong padding format - missing add0");
//            // avoid that we run second step on invalid raw event!
//            //return kFALSE;
//          }
//          if (((*pdata & 0xff00) >> 8) != dma_padd)
//          {
//            //TGo4Log::Error("Wrong padding format - 8-15 bits are not the same");
//            GO4_SKIP_EVENT_MESSAGE("**** THitDetRawProc: Wrong padding format - 8-15 bits are not the same");
//            // avoid that we run second step on invalid raw event!
//            //return kFALSE;
//          }
//          if ((*pdata & 0xff) != cnt)
//          {
//            //TGo4Log::Error("Wrong padding format - 0-7 bits not as expected");
//            GO4_SKIP_EVENT_MESSAGE("**** THitDetRawProc: Wrong padding format - 0-7 bits not as expected");
//            // avoid that we run second step on invalid raw event!
//            //return kFALSE;
//          }
//          pdata++;
//          cnt++;
//        }
//        continue;
//      }
//      else if ((unsigned) *pdata == 0xbad00bad)
//      {
//        GO4_SKIP_EVENT_MESSAGE("**** THitDetRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*pdata));
//      }
//      else if ((*pdata & 0xff) != 0x34)    // regular channel data
//      {
//        //GO4_STOP_ANALYSIS_MESSAGE("Wrong optic format - 0x34 are expected0-7 bits not as expected");
//        //TGo4Log::Error("Wrong optic format 0x%x - 0x34 are expected0-7 bits not as expected", (*pdata & 0xff));
//        GO4_SKIP_EVENT_MESSAGE(
//            "**** THitDetRawProc: Wrong optic format 0x%x - 0x34 are expected - 0-7 bits not as expected", (*pdata & 0xff));
//        // avoid that we run second step on invalid raw event!
//        //return kFALSE;
//      }
//
//      Int_t* pdatastart = pdata;    // remember begin of optic payload data section
//      // unsigned trig_type   = (*pdata & 0xf00) >> 8;
//      unsigned sfp_id = (*pdata & 0xf000) >> 12;
//      unsigned device_id = (*pdata & 0xff0000) >> 16;
//      // unsigned channel_id  = (*pdata & 0xff000000) >> 24;
//      pdata++;
//
//      Int_t opticlen = *pdata++;
//      if (opticlen > lwords * 4)
//      {
//        //TGo4Log::Error("Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
//        GO4_SKIP_EVENT_MESSAGE(
//            "**** THitDetRawProc: Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
//        // avoid that we run second step on invalid raw event!
//        //return kFALSE;
//      }
//      HitDetRAW_CHECK_PDATA;
//      int eventcounter = *pdata;
//      //TGo4Log::Info("Internal Event number 0x%x", eventcounter);
//      // board id calculated from SFP and device id:
//      UInt_t brdid = fPar->fBoardID[sfp_id][device_id];
//      THitDetBoard* theBoard = HitDetRawEvent->GetBoard(brdid);
//      if (theBoard == 0)
//      {
//        GO4_SKIP_EVENT_MESSAGE(
//            "Configuration error: Board id %d does not exist as subevent, sfp:%d device:%d", brdid, sfp_id, device_id);
//
//        return kFALSE;
//      }
//      THitDetBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
//      if (boardDisplay == 0)
//      {
//        GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
//        return kFALSE;
//      }
//
//      pdata += 1;
//      HitDetRAW_CHECK_PDATA;
//      theBoard->fQfwSetup = *pdata;
//      //TGo4Log::Info("HitDet SEtup %d", theBoard->fQfwSetup);
//      for (int j=0; j<4;++j)
//      {
//        HitDetRAW_CHECK_PDATA_BREAK;
//        pdata++;
//
//      }
//      //pdata += 4;
//      HitDetRAW_CHECK_PDATA;
//      for (int loop = 0; loop < theBoard->getNElements(); loop++)
//      {
//        THitDetLoop* theLoop = theBoard->GetLoop(loop);
//        if (theLoop == 0)
//        {
//          TGo4Log::Error("Configuration error: Loop index %d  emtpy subevent for boardid:%d", loop, brdid);
//          continue;
//        }
//
//        theLoop->fQfwSetup = theBoard->fQfwSetup;    // propagate setup id to subevent
//        HitDetRAW_CHECK_PDATA_BREAK;
//        theLoop->fQfwLoopSize = *pdata++;
//        theLoop->fHasData=kTRUE; // dynamic rebinning of timeslices only if we really have valid event
//
////      if (theLoop->fQfwLoopSize >= PEXOR_HitDetSLICES)
////      {
////        TGo4Log::Error("THitDetRawProc: found very large slice size %d max %d -  Please check set up!",
////            theLoop->fQfwLoopSize, PEXOR_HitDetSLICES);
////	 return kFALSE;
////      }
//
//      }    // first loop loop
//
//      HitDetRAW_CHECK_PDATA;
//      for (int loop = 0; loop < theBoard->getNElements(); loop++)
//      {
//        THitDetLoop* theLoop = theBoard->GetLoop(loop);
//        HitDetRAW_CHECK_PDATA_BREAK;
//        theLoop->fQfwLoopTime = *pdata++;
//      }    // second loop loop
//
//      // TODO: are here some useful fields
//      for (int j=0; j<21;++j)
//      {
//        HitDetRAW_CHECK_PDATA_BREAK;
//        pdata++;
//
//      }
//      //pdata += 21; // need to check for each increment if we are outside this slave's payload!
//      HitDetRAW_CHECK_PDATA;
//      /** All loops X slices/loop X channels */
//      for (int loop = 0; loop < theBoard->getNElements(); loop++)
//      {
//        THitDetLoop* loopData = theBoard->GetLoop(loop);
//        for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
//          for (int ch = 0; ch < PEXOR_HitDetCHANS; ++ch)
//          {
//            HitDetRAW_CHECK_PDATA_BREAK;
//            Int_t value = *pdata++;
//
//            if(fPar->fUseFrontendOffsets)
//            {
//
//              if(fPar->fFrontendOffsetLoop!=loop) // supress correction of frontend offset raw data if dynamic mode is set!
//                {
//                // we account frontend measured offset already here
//                // this emulates future mode where offset is corrected already in poland
//                Double_t correction=theBoard->GetOffset(ch) * loopData->GetMicroSecsPerTimeSlice()/1.0e+6;
//                // <- offset is measured for 1 second, evaluate for actual time slice period
//
//                value -= correction;
//                }
//            }
//
//            loopData->fQfwTrace[ch].push_back(value);
//
//            if (fPar->fSelectTriggerEvents && ((UInt_t) fPar->fTriggerBoardID == brdid))
//            {
//              /////////// software trigger section
//              // begin selecting good, bad and ugly events for free running data
//              if (ch >= fPar->fTriggerFirstChannel || ch <= fPar->fTriggerLastChannel)
//                triggersum += value;
//            }    // if (fPar->fSelectTriggerEvents)
//
//            //printf("loop %d slice %d ch %d = %d\n", loop, sl ,ch ,value);
//          }
//      }    //loop
//
//      HitDetRAW_CHECK_PDATA;
//      /* errorcount values: - per HitDet CHIPS*/
//      for (int qfw = 0; qfw < PEXOR_HitDetNUM; ++qfw)
//      {
//        HitDetRAW_CHECK_PDATA_BREAK;
//        theBoard->SetErrorScaler(qfw, (UInt_t) (*pdata++));
//        //printf("EEEEEE Error counter qfw %d =0x%x\n", qfw , *(pdata-1));
//      }
//      HitDetRAW_CHECK_PDATA;
//
//      // skip filler words at the end of gosip payload:
//      while (pdata - pdatastart <= (opticlen / 4))    // note that trailer is outside opticlen!
//      {
//        //printf("######### skipping word 0x%x\n ",*pdata);
//        pdata++;
//      }
//
//      // crosscheck if trailer word matches eventcounter header
//      if (*pdata != eventcounter)
//      {
//        TGo4Log::Error("Eventcounter 0x%x does not match trailing word 0x%x at position 0x%x!", eventcounter, *pdata,
//            (opticlen / 4));
//        pdata++;
//        continue;
//      }
//      //TGo4Log::Info("!!!!!!!!!!! found  trailer 0x%x",*pdata);
//      pdata++;
//      HitDetRawEvent->fSequenceNumber = eventcounter;
//




    }    // while pdata - psubevt->GetDataField() <lwords

  }    // while subevents

    //

  FillDisplays(); // JAM TODO: pobably we fill the raw displays immediately before storing them into output event!
  HitDetRawEvent->SetValid(kTRUE);    // to store
  return kTRUE;
}

Bool_t THitDetRawProc::FillDisplays()
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

//    // check first if we have to rebin histograms:
//    Bool_t rebinned = kFALSE;
//    for (int loop = 0; loop < theBoard->getNElements(); loop++)
//    {
//      THitDetBoardLoopDisplay* loopDisplay = boardDisplay->GetLoopDisplay(loop);
//      if (loopDisplay == 0)
//      {
//        TGo4Log::Error("Configuration error: Loop index %d  empty histogram set for boardid:%d", loop, brdid);
//        continue;
//      }
//      THitDetLoop* theLoop = theBoard->GetLoop(loop);
//      // optionally rescale histograms of this loop:
//      if (theLoop->fHasData && (loopDisplay->GetTimeSlices() != theLoop->fQfwLoopSize))
//      {
//        loopDisplay->InitDisplay(theLoop->fQfwLoopSize, kTRUE);
//        rebinned = kTRUE;
//      }
//    }
//    if (rebinned)
//      boardDisplay->InitDisplay(-1, kTRUE);    // rebin overview histograms with true timeslices of subloops

    // now fill histograms from already unpacked data in ouput event:
//    boardDisplay->hHitDetRaw2DTrace->Reset("");
//    int loopoffset = 0;
//    /** All loops X slices/loop X channels */
//    for (int loop = 0; loop < theBoard->getNElements(); loop++)
//    {
//      THitDetLoop* loopData = theBoard->GetLoop(loop);
//      THitDetBoardLoopDisplay* loopDisplay = boardDisplay->GetLoopDisplay(loop);
//      loopDisplay->hHitDetRawTrace->Reset("");
//
//      for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
//        for (int ch = 0; ch < PEXOR_HitDetCHANS; ++ch)
//        {
//          Int_t value = loopData->fQfwTrace[ch].at(sl);
//          //printf("loop %d slice %d ch %d = %d\n", loop, sl ,ch ,value);
//
//          if (!fPar->fSimpleCompensation)
//          {
//            loopDisplay->hHitDetRawTrace->SetBinContent(ch + 1 + sl * PEXOR_HitDetCHANS, value);
//            boardDisplay->hHitDetRaw2DTrace->Fill(loopoffset + sl, ch, value);
//
//            loopDisplay->hHitDetRaw->AddBinContent(ch + 1 + sl * PEXOR_HitDetCHANS, value);
//            boardDisplay->hHitDetRaw2D->Fill(loopoffset + sl, ch, value);
//            //printf("      loopoffset= %d\n", loopoffset);
//
//
//
//
//          }
//        }
//      if (fPar->fSimpleCompensation)
//      {
//        for (int ch = 0; ch < PEXOR_HitDetCHANS; ++ch)
//        {
//
//          if (loopData->fQfwLoopSize < 5)
//            continue;
//
//          Double_t sum = 0;
//          for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
//            sum += loopData->fQfwTrace[ch].at(sl);
//          sum = sum / loopData->fQfwLoopSize;
//
//          for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
//          {
//            Double_t value = loopData->fQfwTrace[ch].at(sl) - sum;
//
//            loopDisplay->hHitDetRawTrace->SetBinContent(ch + 1 + sl * PEXOR_HitDetCHANS, value);
//            boardDisplay->hHitDetRaw2DTrace->Fill(loopoffset + sl, ch, value);
//
//            loopDisplay->hHitDetRaw->AddBinContent(ch + 1 + sl * PEXOR_HitDetCHANS, value);
//            boardDisplay->hHitDetRaw2D->Fill(loopoffset + sl, ch, value);
//          }    // sl
//        }    //ch
//      }
//      loopoffset += loopData->fQfwLoopSize;


//    }    //loop
//
//    //boardDisplay->hHitDetRawErrTr->Reset("");
//    /* errorcount values: - per HitDet CHIPS*/
//    for (int qfw = 0; qfw < PEXOR_HitDetNUM; ++qfw)
//    {
//      boardDisplay->hHitDetRawErr->SetBinContent(1 + qfw, theBoard->GetErrorScaler(qfw));
//      //boardDisplay->hHitDetRawErrTr->SetBinContent(1 + qfw, theBoard->GetErrorScaler(qfw));
//      //printf("FFFFFF Fill Error counter qfw %d with 0x%x\n", qfw , theBoard->GetErrorScaler(qfw));
//
//    }
//    // need to update this here, since histograms are initialized after offset is retrieved!
//    for (int c = 0; c < PEXOR_HitDetCHANS; ++c)
//      boardDisplay->hHitDetOffsets->SetBinContent(c+1,theBoard->GetOffset(c));
//
//



  }    // i board
  return kTRUE;
}


