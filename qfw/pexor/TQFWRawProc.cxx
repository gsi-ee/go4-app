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

//***********************************************************
TQFWRawProc::TQFWRawProc() :
    TGo4EventProcessor()
{
}

//***********************************************************
// this one is used in standard factory
TQFWRawProc::TQFWRawProc(const char* name) :
    TGo4EventProcessor(name)
{
  TGo4Log::Info("TQFWRawProc: Create instance %s", name);

  SetMakeWithAutosave(kTRUE);
//// init user analysis objects:

  fPar = dynamic_cast<TQFWRawParam*>(MakeParameter("QFWRawParam", "TQFWRawParam", "set_QfwPar.C"));
  if (fPar)
    fPar->SetConfigBoards();

  if (fPar->fSimpleCompensation)
    TGo4Log::Info("Apply simple compensation - at least 5 values required");

  for (unsigned i = 0; i < TQFWRawEvent::fgConfigQFWBoards.size(); ++i)
  {
    UInt_t uniqueid = TQFWRawEvent::fgConfigQFWBoards[i];
    fBoards.push_back(new TQFWBoardDisplay(uniqueid));
  }

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
  QFWRawEvent = (TQFWRawEvent*) target;
  QFWRawEvent->SetValid(kFALSE);    // not store
  Int_t triggersum = 0;    // sums up all "software trigger" channels (for free running acquired data)
  TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
  if (source == 0)
  {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  if (source->GetTrigger() > 11)
  {
    cout << "**** TQFWRawProc: Skip trigger event" << endl;

    return kFALSE;
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
      else if ((*pdata & 0xff) != 0x34)    // regular channel data
      {
        //GO4_STOP_ANALYSIS_MESSAGE("Wrong optic format - 0x34 are expected0-7 bits not as expected");
        //TGo4Log::Error("Wrong optic format 0x%x - 0x34 are expected0-7 bits not as expected", (*pdata & 0xff));
        GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong optic format 0x%x - 0x34 are expected0-7 bits not as expected",
            (*pdata & 0xff));
        // avoid that we run second step on invalid raw event!
        //return kFALSE;
      }

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
      TQFWBoardDisplay* boardDisplay = GetBoardDisplay(brdid);
      if (boardDisplay == 0)
      {
        GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
        return kFALSE;
      }

      pdata += 1;
      theBoard->fQfwSetup = *pdata;
      //TGo4Log::Info("QFW SEtup %d", theBoard->fQfwSetup);
      pdata += 4;
      for (int loop = 0; loop < theBoard->getNElements(); loop++)
      {
        TQFWLoop* theLoop = theBoard->GetLoop(loop);
        if (theLoop == 0)
        {
          TGo4Log::Error("Configuration error: Loop index %d  emtpy subevent for boardid:%d", loop, brdid);
          continue;
        }

        theLoop->fQfwSetup = theBoard->fQfwSetup;    // propagate setup id to subevent

        theLoop->fQfwLoopSize = *pdata++;

//      if (theLoop->fQfwLoopSize >= PEXOR_QFWSLICES)
//      {
//        TGo4Log::Error("TQFWRawProc: found very large slice size %d max %d -  Please check set up!",
//            theLoop->fQfwLoopSize, PEXOR_QFWSLICES);
//	 return kFALSE;
//      }

      }    // first loop loop


      for (int loop = 0; loop < theBoard->getNElements(); loop++)
      {
        TQFWLoop* theLoop = theBoard->GetLoop(loop);
        theLoop->fQfwLoopTime = *pdata++;
      }    // second loop loop

      // TODO: are here some useful fields
      pdata += 21;
      /** All loops X slices/loop X channels */

      for (int loop = 0; loop < theBoard->getNElements(); loop++)
      {
        TQFWLoop* loopData = theBoard->GetLoop(loop);
        for (int sl = 0; sl < loopData->fQfwLoopSize; ++sl)
          for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
          {
            Int_t value = *pdata++;
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

      /* errorcount values: - per QFW CHIPS*/
      for (int qfw = 0; qfw < PEXOR_QFWNUM; ++qfw)
      {
        theBoard->SetErrorScaler(qfw, (UInt_t) (*pdata++));
      }

      // eventcounter is trailing word, if we find it we are done with this board:
      while (*pdata != eventcounter)
      {
        pdata++;
        //printf ("trailer : 0x%x\n",pdata);
        if ((*pdata & 0xFFFF0000) == 0xadd00000)
        {
          TGo4Log::Error("already found padding word 0x%x before trailer!", pdata);
          return kFALSE;    // leave subevent loop if no more data available
        }

        if (pdata > psubevt->GetDataField() + lwords)
        {
          TGo4Log::Error("Could not find trailing word 0x%x until end of subevent!", eventcounter);
          return kFALSE;    // leave subevent loop if no more data available
        }

      }    // while

      //TGo4Log::Info("!!!!!!!!!!! found  trailer 0x%x",*pdata);
      pdata++;
      //printf("event counter 0x%x\n ",eventcounter);
      QFWRawEvent->fSequenceNumber = eventcounter;

//    while(pdata < psubevt->GetDataField() + opticlen/4)
//    {
//      //printf("skipping word 0x%x\n ",*pdata);
//      pdata++; // skip rest of payload, try next device data
//
//    }
//

    }    // while pdata - psubevt->GetDataField() <lwords

  }    // while subevents


  if (fPar->fSelectTriggerEvents)
         {
            //TGo4Log::Info("Triggersum of event %d is %d", QFWRawEvent->fSequenceNumber, triggersum);
            if((triggersum<fPar->fTriggerHighThreshold) && (triggersum> fPar->fTriggerLowThreshold))
            {
              GO4_SKIP_EVENT_MESSAGE("Skip event of seqnr %d with triggersum %d!", QFWRawEvent->fSequenceNumber, triggersum); // debug
              // GO4_SKIP_EVENT; // no debug mode
            }
         }
  FillDisplays(); // we only fill histograms for the events that are selected by trigger condition
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
      if (loopDisplay->GetTimeSlices() != theLoop->fQfwLoopSize)
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
          Int_t value = loopData->fQfwTrace[ch].at(sl);
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

    boardDisplay->hQFWRawErrTr->Reset("");
    /* errorcount values: - per QFW CHIPS*/
    for (int qfw = 0; qfw < PEXOR_QFWNUM; ++qfw)
    {
      boardDisplay->hQFWRawErr->SetBinContent(1 + qfw, theBoard->GetErrorScaler(qfw));
      boardDisplay->hQFWRawErrTr->AddBinContent(1 + qfw, theBoard->GetErrorScaler(qfw));
    }

  }    // i board
  return kTRUE;
}

