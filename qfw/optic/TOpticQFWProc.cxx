#include "TOpticQFWProc.h"

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

#include "TOpticQFWEvent.h"
#include "TOpticQFWParam.h"

//***********************************************************
TOpticQFWProc::TOpticQFWProc() :
   TGo4EventProcessor()
{
}
//***********************************************************
TOpticQFWProc::~TOpticQFWProc()
{
   TGo4Log::Info("TOpticQFWProc: Delete instance");
}
//***********************************************************
// this one is used in standard factory
TOpticQFWProc::TOpticQFWProc(const char* name) :
   TGo4EventProcessor(name)
{
   TGo4Log::Info("TOpticQFWProc: Create instance %s", name);

   SetMakeWithAutosave(kTRUE);
   //// init user analysis objects:

   fPar = dynamic_cast<TOpticQFWParam*>(MakeParameter("QFWSetup", "TOpticQFWParam", "set_QfwPar.C"));

   if (fPar->fSimpleCompensation) TGo4Log::Info("Apply simple compensation - at least 5 values required");

   // fPar->PrintParameter();
   // MapGrids();

   InitDisplay(OPTIC_QFWSLICES);
}


void TOpticQFWProc::InitDisplay(int timeslices, Bool_t replace)
{
   cout << "**** TOpticQFWProc: Init Display for "<<timeslices <<" time slices. "<< endl;
   if(replace) //TGo4Analysis::Instance()->
         SetMakeWithAutosave(kFALSE);
   // Creation of histograms (or take them from autosave)
   TString obname;
   TString obtitle;
   TString foldername;
   TString binlabel;

   for (int brd = 0; brd < OPTIC_QFWBOARDS; ++brd) {
      for (int loop = 0; loop < OPTIC_QFWLOOPS; ++loop)
      {
         obname.Form("Board%d/Brd%d-Loop%d-Sum", brd,brd,loop);
         obtitle.Form("QFW Board %d Loop %d Accumulated", brd, loop);
         int qbins = timeslices * OPTIC_QFWCHANS ;
         hOpticQFW[brd][loop] = MakeTH1('I', obname.Data(), obtitle.Data(), qbins, 0,
               qbins, "QFW scaler sums");

         obname.Form("Board%d/Brd%d-Loop%d-Trace", brd,brd,loop);
         obtitle.Form("QFW Board %d Loop %d Trace", brd, loop);
         hOpticQFWTrace[brd][loop] = MakeTH1('I', obname.Data(), obtitle.Data(),
               qbins, 0, qbins, "QFW scalers");

         for (int sl = 0; sl < timeslices; ++sl)
            for (int ch = 0; ch < OPTIC_QFWCHANS; ++ch) {
               // binlabel.Form("SL%d-%d", sl, ch);
               //hOpticQFW[brd][loop]->GetXaxis()->SetBinLabel(1 + sl*OPTIC_QFWCHANS + ch, binlabel.Data());
               //hOpticQFWTrace[brd][loop]->GetXaxis()->SetBinLabel(1 + sl*OPTIC_QFWCHANS + ch, binlabel.Data());
            }
      } // loops

      hOpticQFW2D[brd] = MakeTH2('I', Form("Board%d/Brd%d-2D-Sum", brd, brd), Form("QFW Board %d accumulated for all loops", brd), OPTIC_QFWLOOPS*timeslices, 0,OPTIC_QFWLOOPS*timeslices, OPTIC_QFWCHANS, 0, OPTIC_QFWCHANS, "loop", "ch");

      hOpticQFW2DTrace[brd] = MakeTH2('I', Form("Board%d/Brd%d-2D-Trace", brd, brd), Form("QFW Board %d trace for all loops", brd), OPTIC_QFWLOOPS*timeslices, 0,OPTIC_QFWLOOPS*timeslices, OPTIC_QFWCHANS, 0, OPTIC_QFWCHANS, "loop", "ch");

      hOpticQFWErr[brd] = MakeTH1('I', Form("Board%d/Brd%dErr", brd, brd), Form("QFW errors in board %d", brd), OPTIC_QFWNUM, 0, OPTIC_QFWNUM);

      hOpticQFWErrTr[brd] = MakeTH1('I', Form("Board%d/Brd%dErrTr", brd, brd), Form("QFW errors in board %d (snap shot)", brd), OPTIC_QFWNUM, 0, OPTIC_QFWNUM);

      obname.Form("QFW_Rawscalers_Brd%d",brd);
      pOpticQfws[brd] = GetPicture(obname.Data());
      if (pOpticQfws[brd] == 0)   {
         obtitle.Form("Overview all QFW Accumulated Board%d",brd);
         pOpticQfws[brd] = new TGo4Picture(obname.Data(), obtitle.Data());

          pOpticQfws[brd]->SetDivision(OPTIC_QFWLOOPS, 1);
         for (int loop = 0; loop < OPTIC_QFWLOOPS; ++loop) {
              pOpticQfws[brd]->Pic(loop, 0)->AddObject(hOpticQFW[brd][loop]);
            pOpticQfws[brd]->Pic(loop, 0)->SetFillAtt(5, 3001);
         }
         AddPicture(pOpticQfws[brd], Form("Board%d",brd));
      }

      obname.Form("QFW_Rawscalers_Brd%d_Tr",brd);
      pOpticQfwsTrace[brd] = GetPicture(obname.Data());
      if (pOpticQfwsTrace[brd] == 0)  {
         obtitle.Form("Overview all QFW Trace Board%d",brd);
         pOpticQfwsTrace[brd] = new TGo4Picture(obname.Data(), obtitle.Data());

         pOpticQfwsTrace[brd]->SetDivision(OPTIC_QFWLOOPS, 1);
         for (int loop = 0; loop < OPTIC_QFWLOOPS; ++loop) {
            pOpticQfwsTrace[brd]->Pic(loop, 0)->AddObject(hOpticQFWTrace[brd][loop]);
            pOpticQfwsTrace[brd]->Pic(loop, 0)->SetFillAtt(5, 3001);
         }
         AddPicture(pOpticQfwsTrace[brd], Form("Board%d",brd));
      }

   } // board

   ////////////////////////////////////_________________________________

   return;

   /* xy beam display*/
   obname.Form("Beam/GridAccu");
   obtitle.Form("Beam grid current accumulate");

   for (int grid = 0; grid < OPTIC_QFWGRIDS; ++grid)
         {

   hBeamX[grid] = MakeTH1('I', Form("Beam/Grid%2d/Profile_X_G%d",grid,grid), Form("X Profile Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid],
         "Wire");
   hBeamY[grid] = MakeTH1('I', Form("Beam/Grid%2d/Profile_Y_G%d",grid,grid), Form("Y Profile Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid],
         "Wire");
   hBeamXSlice[grid] = MakeTH2('I', Form("Beam/Grid%2d/Profile_X_Time_G%d",grid,grid), Form("X Profile vs Time slices Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], timeslices,0, timeslices,
            "Wire","Time Slice");
   hBeamYSlice[grid] = MakeTH2('I',  Form("Beam/Grid%2d/Profile_Y_Time_G%d",grid,grid), Form("Y Profile vs Time slices Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], timeslices,0, timeslices,
         "Wire","Time Slice");

   hBeamXSliceOffs[grid] = MakeTH2('I', Form("Beam/Grid%2d/Profile_X_Time_Offset_G%d",grid,grid), Form("X Profile vs Time slices average offset Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], timeslices,0, timeslices,
               "Wire","Time Slice");
   hBeamYSliceOffs[grid] = MakeTH2('I', Form("Beam/Grid%2d/Profile_Y_Time_Offset_G%d",grid,grid), Form("Y Profile vs Time slices average offset Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], timeslices,0, timeslices,
            "Wire","Time Slice");


   hBeamAccX[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfileSum_X_G%d",grid,grid), Form("X Profile accumulated Grid%2d",grid), gBeamWiresX[grid],
         0, gBeamWiresX[grid], "Wire");
   hBeamAccY[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfileSum_Y_G%d",grid,grid), Form("Y Profile accumulated Grid%2d",grid),
         gBeamWiresY[grid], 0, gBeamWiresY[grid], "Wire");

   hBeamAccXSlice[grid] = MakeTH2('I', Form("Beam/Grid%2d/ProfileSum_X_Time_G%d",grid,grid), Form("X Profile vs Time slices accum Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], timeslices,0, timeslices,
               "Wire","Time Slice");
   hBeamAccYSlice[grid] = MakeTH2('I', Form("Beam/Grid%2d/ProfileSum_Y_Time_G%d",grid,grid), Form("Y Profile vs Time slices accum Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], timeslices,0, timeslices,
            "Wire","Time Slice");

   obname.Form("Beam Display Grid%2d",grid);
   pBeamProfiles[grid] = GetPicture(obname.Data());
   if (pBeamProfiles[grid] == 0)
   {
      obtitle.Form("Overview beam projections Grid%2d",grid);
      foldername.Form("Beam/Grid%2d",grid);
      pBeamProfiles[grid] = new TGo4Picture(obname.Data(), obtitle.Data());
      pBeamProfiles[grid]->SetDivision(2, 2);
      pBeamProfiles[grid]->Pic(0, 0)->AddObject(hBeamX[grid]);
      pBeamProfiles[grid]->Pic(0, 0)->SetFillAtt(3, 3001);
      pBeamProfiles[grid]->Pic(0, 1)->AddObject(hBeamY[grid]);
      pBeamProfiles[grid]->Pic(0, 1)->SetFillAtt(4, 3001);
      pBeamProfiles[grid]->Pic(1, 0)->AddObject(hBeamAccX[grid]);
      pBeamProfiles[grid]->Pic(1, 0)->SetFillAtt(3, 3001);
      pBeamProfiles[grid]->Pic(1, 1)->AddObject(hBeamAccY[grid]);
      pBeamProfiles[grid]->Pic(1, 1)->SetFillAtt(4, 3001);
      AddPicture(pBeamProfiles[grid], foldername.Data());

   }

   hBeamMeanXY[grid]=MakeTH2('I', Form("Beam/Grid%2d/Meanpos_G%d",grid,grid), Form("Beam Mean position scaler Grid%2d",grid), gBeamWiresX[grid], 0,gBeamWiresX[grid],gBeamWiresY[grid], 0,gBeamWiresY[grid],"X","Y");

    hBeamRMSX[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_X_G%d",grid,grid), Form("X Profile RMS Grid%2d",grid),
            10*gBeamWiresX[grid], 0, gBeamWiresX[grid], "Wire spacings");

    hBeamRMSY[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_Y_G%d",grid,grid), Form("Y Profile RMS Grid%2d",grid),
            10*gBeamWiresX[grid], 0, gBeamWiresY[grid], "Wire spacings");

    obname.Form("Beam RMS Grid%2d",grid);
    pBeamRMS[grid]= GetPicture(obname.Data());
       if (pBeamRMS == 0)
       {
          obtitle.Form("Beam RMS distribution Grid%2d",grid);
          foldername.Form("Beam/Grid%2d",grid);
          pBeamRMS[grid] = new TGo4Picture(obname.Data(), obtitle.Data());
          pBeamRMS[grid]->SetDivision(1, 2);
          pBeamRMS[grid]->Pic(0, 0)->AddObject(hBeamRMSX[grid]);
          pBeamRMS[grid]->Pic(0, 0)->SetFillAtt(3, 3002);
          pBeamRMS[grid]->Pic(0, 1)->AddObject(hBeamRMSY[grid]);
          pBeamRMS[grid]->Pic(0, 1)->SetFillAtt(4, 3002);
          AddPicture(pBeamRMS[grid], foldername.Data());

       }


   /* mean count of beam profile part: */

       cBeamXSliceCond[grid] = MakeWinCond(Form("Beam/Grid%2d/XSliceCond_G%d",grid,grid), 0,gBeamWiresX[grid], 0, timeslices, hBeamXSlice[grid]->GetName());
       cBeamYSliceCond[grid] = MakeWinCond(Form("Beam/Grid%2d/YSliceCond_G%d",grid,grid), 0,gBeamWiresY[grid], 0, timeslices, hBeamYSlice[grid]->GetName());


   hBeamMeanCountsX[grid]=MakeTH1('I', Form("Beam/Grid%2d/Mean_Counts_X_G%d",grid,grid), Form("X Counts Mean Grid%2d",grid),
            1000, 0, 100, "Mean counts");

   hBeamMeanCountsY[grid]=MakeTH1('I', Form("Beam/Grid%2d/Mean_Counts_Y_G%d",grid,grid), Form("Y Counts Mean Grid%2d",grid),
            1000, 0, 100, "Mean counts");

   hBeamRMSCountsX[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_Counts_X_G%d",grid,grid), Form("X Counts RMS Grid%2d",grid),
            1000, 0, 100, "RMS counts");

   hBeamRMSCountsY[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_Counts_Y_G%d",grid,grid), Form("Y Counts RMS Grid%2d",grid),
            1000, 0, 100, "RMS counts");

   } // grid

}


//-----------------------------------------------------------
// event function
Bool_t TOpticQFWProc::BuildEvent(TGo4EventElement* target)
{
   // called by framework from TOpticQFWEvent to fill it
   TOpticQFWEvent* OpticQFWEvent = (TOpticQFWEvent*) target;
   OpticQFWEvent->SetValid(kFALSE); // not store

   TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
   if (source == 0)
   {
      cout << "AnlProc: no input event !" << endl;
      return kFALSE;
   }
   if (source->GetTrigger() > 11)
   {
      cout << "**** TOpticQFWProc: Skip trigger event" << endl;

      return kFALSE;
   }
   // first we fill the TOpticQFWEvent with data from MBS source
   // we have up to two subevents, crate 1 and 2
   // Note that one has to loop over all subevents and select them by
   // crate number:   psubevt->GetSubcrate(),
   // procid:         psubevt->GetProcid(),
   // and/or control: psubevt->GetControl()
   // here we use only crate number

   source->ResetIterator();
   TGo4MbsSubEvent* psubevt(0);
   while ((psubevt = source->NextSubEvent()) != 0)
   { // loop over subevents
      Int_t *pdata = psubevt->GetDataField();
      Int_t lwords = psubevt->GetIntLen();

      Int_t dma_padd = (*pdata & 0xff00) >> 8;
      Int_t cnt(0);
      while (cnt < dma_padd) {
         if ((*pdata & 0xffff0000) != 0xadd00000) {
            TGo4Log::Error("Wrong padding format - missing add0");
            return kFALSE;
         }
         if (((*pdata & 0xff00) >> 8) != dma_padd) {
            TGo4Log::Error("Wrong padding format - 8-15 bits are not the same");
            return kFALSE;
         }
         if ((*pdata & 0xff) != cnt) {
            TGo4Log::Error("Wrong padding format - 0-7 bits not as expected");
            return kFALSE;
         }
         pdata++;
         cnt++;
      }

      if ((*pdata & 0xff) != 0x34) {
         TGo4Log::Error("Wrong optic format - 0x34 are expected0-7 bits not as expected");
         return kFALSE;
      }


      // unsigned trig_type   = (*pdata & 0xf00) >> 8;
      // unsigned sfp_id      = (*pdata & 0xf000) >> 12;
      // unsigned device_id   = (*pdata & 0xff0000) >> 16;
      // unsigned channel_id  = (*pdata & 0xff000000) >> 24;
      pdata++;

      Int_t opticlen = *pdata++;
      if (opticlen > lwords*4) {
         TGo4Log::Error("Mismatch with subevent len %d and optic len %d", lwords*4, opticlen);
         return kFALSE;
      }

      // TODO - later board id should be calculated from SFP and other ids
      int brd = 0;

      // TODO: are here some useful fields
      pdata+=5;

      for (int loop=0; loop<OPTIC_QFWLOOPS; loop++) {
         OpticQFWEvent->fQfwLoopSize[brd][loop] = *pdata++; // new: dynamic time slice number

         // printf("Loop %d = size %d\n", loop, OpticQFWEvent->fQfwLoopSize[brd][loop]);
         if (OpticQFWEvent->fQfwLoopSize[brd][loop]>=OPTIC_QFWSLICES) {
            TGo4Log::Error("TOpticQFWProc: found very large slice size %d max %d -  Please check set up!",
                  OpticQFWEvent->fQfwLoopSize[brd][loop], OPTIC_QFWSLICES);
            return kFALSE;
         }
      }

      for (int loop=0; loop<OPTIC_QFWLOOPS; loop++) {
         OpticQFWEvent->fQfwLoopTime[brd][loop] = *pdata++;
         // printf("Loop %d = time %x\n", loop, OpticQFWEvent->fQfwLoopTime[brd][loop]);
      }

      // TODO: are here some useful fields
      pdata += 21;

      hOpticQFW2DTrace[brd]->Reset("");

      /** All loops X slices/loop X channels */
      for (int loop = 0;loop<OPTIC_QFWLOOPS;loop++) {
         hOpticQFWTrace[brd][loop]->Reset("");

         for (int sl = 0; sl < OpticQFWEvent->fQfwLoopSize[brd][loop]; ++sl)
            for (int ch = 0; ch < OPTIC_QFWCHANS; ++ch) {
               Int_t value = *pdata++;
               OpticQFWEvent->fQfw[brd][loop][sl][ch] = value;

               // printf("loop %d slice %d ch %d = %d\n", loop, sl ,ch ,value);

               if (!fPar->fSimpleCompensation) {
                  hOpticQFWTrace[brd][loop]->SetBinContent(ch + 1 + sl*OPTIC_QFWCHANS, value);
                  hOpticQFW2DTrace[brd]->Fill(loop * OPTIC_QFWSLICES + sl, ch, value);

                  hOpticQFW[brd][loop]->AddBinContent(ch + 1 + sl*OPTIC_QFWCHANS, value);
                  hOpticQFW2D[brd]->Fill(loop * OPTIC_QFWSLICES + sl, ch, value);
               }
            }


         if (fPar->fSimpleCompensation)
            for (int ch = 0; ch < OPTIC_QFWCHANS; ++ch) {

               if (OpticQFWEvent->fQfwLoopSize[brd][loop]<5) continue;

               Double_t sum = 0;
               for (int sl = 0; sl < OpticQFWEvent->fQfwLoopSize[brd][loop]; ++sl)
                  sum += OpticQFWEvent->fQfw[brd][loop][sl][ch];
               sum = sum / OpticQFWEvent->fQfwLoopSize[brd][loop];

               for (int sl = 0; sl < OpticQFWEvent->fQfwLoopSize[brd][loop]; ++sl) {
                  Double_t value = OpticQFWEvent->fQfw[brd][loop][sl][ch] - sum;

                  hOpticQFWTrace[brd][loop]->SetBinContent(ch + 1 + sl*OPTIC_QFWCHANS, value);
                  hOpticQFW2DTrace[brd]->Fill(loop * OPTIC_QFWSLICES + sl, ch, value);

                  hOpticQFW[brd][loop]->AddBinContent(ch + 1 + sl*OPTIC_QFWCHANS, value);
                  hOpticQFW2D[brd]->Fill(loop * OPTIC_QFWSLICES + sl, ch, value);
               }
            }
      }

      hOpticQFWErrTr[brd]->Reset("");

      /* errorcount values: - per QFW CHIPS*/
      for (int qfw = 0; qfw < OPTIC_QFWNUM; ++qfw) {
         OpticQFWEvent->fQfwErr[brd][qfw] = *pdata++;
         hOpticQFWErr[brd]->SetBinContent(1 + qfw, OpticQFWEvent->fQfwErr[brd][qfw]);
         hOpticQFWErrTr[brd]->AddBinContent(1 + qfw, OpticQFWEvent->fQfwErr[brd][qfw]);
      }

      if (pdata > psubevt->GetDataField() + lwords) {
         TGo4Log::Error("QFW event from board %d is too long", brd);
         return kFALSE; // leave subevent loop if no more data available
      }
      //GO4_STOP_ANALYSIS_MESSAGE(
      //               "**** TOpticQFWProc: found wordcount=%d lwords=%d ",
      //               wordcount, lwords);
   } // while subevents


   // FillGrids(OpticQFWEvent);

   OpticQFWEvent->SetValid(kTRUE); // to store


   //if(fPar->fSlowMotionStart>0)
   // if(evnum>fPar->fSlowMotionStart)
   //   GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode at event %d",evnum);


   return kTRUE;
}

void TOpticQFWProc::FillGrids(TOpticQFWEvent* out)
{

   Bool_t dostop=kFALSE;
   Double_t mtime=out->fQfwMTime * 20 / 1000; // measurement time in us
   Double_t premtime = 0; // measurement time in us

   /* evaluate measurement setup*/
   TString setup;
   switch(out->fQfwSetup)
   {
      case 0:
         setup.Form("(-) [ 2.5pF & 0.25pC]");
      break;

      case 1:
         setup.Form("(-) [25.0pF & 2.50pC]");
      break;

      case 2:
         setup.Form("(+) [ 2.5pF & 0.25pC]");
      break;

      case 3:
         setup.Form("(+) [25.0pF & 2.50pC]");
      break;

      case 0x10:
         setup.Form("1000uA (-) [ 2.5pF & 0.25pC]");
      break;

      case 0x11:
         setup.Form("1000uA (-) [25.0pF & 2.50pC]");
      break;

      case 0x12:
         setup.Form("1000uA (+) [ 2.5pF & 0.25pC]");
      break;

      case 0x13:
         setup.Form("1000uA (+) [25.0pF & 2.50pC]");
         break;

      default:
         setup.Form("unknown setup %d", out->fQfwSetup);
         break;



   };





// APPEND TIME RANGES:
   TString mtitle;
   mtitle.Form("%s dt=%.2E us", setup.Data(), mtime);

   for (int grid = 0;  grid< OPTIC_QFWGRIDS; ++grid)
      {
      hBeamX[grid]->Reset("");
      hBeamY[grid]->Reset("");
      hBeamXSlice[grid]->Reset("");
      hBeamYSlice[grid]->Reset("");
      hBeamXSliceOffs[grid]->Reset("");
      hBeamYSliceOffs[grid]->Reset("");
      hBeamX[grid]->SetTitle(mtitle.Data());
      hBeamY[grid]->SetTitle(mtitle.Data());
      hBeamXSlice[grid]->SetTitle(mtitle.Data());
      hBeamYSlice[grid]->SetTitle(mtitle.Data());
      mtitle.Form("%s dt=%.2E us", setup.Data(),premtime);




   for (int brd = 0;  brd< OPTIC_QFWBOARDS; ++brd)
   {



      for (int q = 0; q < OPTIC_QFWNUM; ++q)
      {
         for (int ch = 0; ch < OPTIC_QFWCHANS; ++ch)
         {
            int xpos = gBeamX[grid][brd][q][ch];
            int ypos = gBeamY[grid][brd][q][ch];
            if (xpos >= 0)
            {
               UInt_t sum=0;
               for (int sl = 0; sl < OPTIC_QFWSLICES; ++sl)
                     {
                        // JAM: newly added correction and display of background offset:
                        if(fPar->fMeasureBackground)
                           {
                              fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
                           }
                        hBeamXSliceOffs[grid]->SetBinContent(1 + xpos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
                        sum+=fPar->GetCorrectedQFWValue(brd,q,ch,sl, out->fQfw[brd][q][ch][sl]);
                        hBeamXSlice[grid]->SetBinContent(1 + xpos, 1+sl, fPar->GetCorrectedQFWValue(brd,q,ch,sl, out->fQfw[brd][q][ch][sl]));
                        UInt_t prev=hBeamAccXSlice[grid]->GetBinContent(1 + xpos,1+sl);
                        hBeamAccXSlice[grid]->SetBinContent(1 + xpos,1+sl, prev+ fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
                     }

               hBeamX[grid]->SetBinContent(1 + xpos, sum);
               hBeamAccX[grid]->AddBinContent(1 + xpos, sum);
            }
            if (ypos >= 0)
            {
               UInt_t sum=0;
               for (int sl = 0; sl < OPTIC_QFWSLICES; ++sl)
                     {
                        if(fPar->fMeasureBackground)
                           {
                              fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
                           }
                        hBeamYSliceOffs[grid]->SetBinContent(1 + ypos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
                        sum+=fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
                        hBeamYSlice[grid]->SetBinContent(1 + ypos, 1+sl, fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
                        UInt_t prev=hBeamAccYSlice[grid]->GetBinContent(1 + ypos,1+sl);
                        hBeamAccYSlice[grid]->SetBinContent(1 + ypos, 1+sl, prev + fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
                     }

               hBeamY[grid]->SetBinContent(1 + ypos, sum);
               hBeamAccY[grid]->AddBinContent(1 + ypos, sum);

               if(grid==0 && ypos==6)
                  {
                  // condition to find broken wire:
                  if(sum<100)
                  {
                     dostop=kTRUE;
                  }
               }

            }
         }

      }

   }// board loop






      out->fBeamMeanX[grid]=hBeamX[grid]->GetMean();

      out->fBeamMeanY[grid]=hBeamY[grid]->GetMean();

      out->fBeamRMSX[grid]=hBeamX[grid]->GetRMS();

      out->fBeamRMSY[grid]=hBeamY[grid]->GetRMS();


      hBeamMeanXY[grid]->Fill(out->fBeamMeanX[grid],out->fBeamMeanY[grid]);

      hBeamRMSX[grid]->Fill(out->fBeamRMSX[grid]);
      hBeamRMSY[grid]->Fill(out->fBeamRMSY[grid]);

      // evaluate here mean value and sigma of profile counts
      // first x direction:
      Int_t cmax=cBeamXSliceCond[grid]->GetCMax(hBeamXSlice[grid]);
      TH1I haux("temp","temp",cmax,0,cmax); // auxiliary histogram to calculate mean and rms of counts
      for(int x=0;x<gBeamWiresX[grid];++x)
      {
         for(int y=0; y<OPTIC_QFWSLICES;++y)
         {
            if(cBeamXSliceCond[grid]->Test(x,y))
               {
                  haux.Fill(hBeamXSlice[grid]->GetBinContent(x+1,y+1));
               }
         }

      }
      Double_t MeanCountsX=haux.GetMean();
      Double_t RMSCountsX=haux.GetRMS();
      mtitle.Form("%s mean=%.2E sigma=%.2E", hBeamXSlice[grid]->GetTitle(), MeanCountsX, RMSCountsX);
      hBeamXSlice[grid]->SetTitle(mtitle.Data());

      // y direction:
      Int_t cmay=cBeamYSliceCond[grid]->GetCMax(hBeamYSlice[grid]);
      TH1I hauy("temp2","temp2",cmay,0,cmay); // auxiliary histogram to calculate mean and rms of counts
      for(int x=0;x<gBeamWiresY[grid];++x)
      {
         for(int y=0; y<OPTIC_QFWSLICES;++y)
         {
            if(cBeamYSliceCond[grid]->Test(x,y))
               {
                  hauy.Fill(hBeamYSlice[grid]->GetBinContent(x+1,y+1));
               }
         }

      }
      Double_t MeanCountsY=hauy.GetMean();
      Double_t RMSCountsY=hauy.GetRMS();
      mtitle.Form("%s mean=%.2E sigma=%.2E", hBeamYSlice[grid]->GetTitle(), MeanCountsY, RMSCountsY);
      hBeamYSlice[grid]->SetTitle(mtitle.Data());

      hBeamMeanCountsX[grid]->Fill(MeanCountsX);
      hBeamMeanCountsY[grid]->Fill(MeanCountsY);
      hBeamRMSCountsX[grid]->Fill(RMSCountsX);
      hBeamRMSCountsY[grid]->Fill(RMSCountsY);

   }// grid loop


   if(dostop)
      {
         TGo4Analysis::Instance()->SendObjectToGUI(hBeamYSlice[0]);
         TGo4Analysis::Instance()->SendObjectToGUI(hBeamYSlice[0]);
         GO4_STOP_ANALYSIS_MESSAGE("Found wire 7 below 100 counts");

      }

}


void TOpticQFWProc::MapGrids()
{

   cout << "**** TOpticQFWProc: Setting up beam coordinate mapping:" << endl;
   // map both for scalers and prescalers


   // first we reset all mappings:
   for (int g = 0; g < OPTIC_QFWGRIDS; ++g)
      {

      gBeamWiresX[g]=16;
      gBeamWiresY[g]=16;

      for (int b = 0; b < OPTIC_QFWBOARDS; ++b)
         {
            for (int qfw = 0; qfw < OPTIC_QFWNUM; ++qfw)
            {
               for (int c = 0; c < OPTIC_QFWCHANS; ++c)
               {
                  gBeamX[g][b][qfw][c] = -1; // reset unused channels
                  gBeamY[g][b][qfw][c] = -1;
               } // channels
            } // qfw

         } // boards
      } // grids


   // HERE IS THE GRID-MAPPING:
   // gbeamX[gridnumber ][boardnumber][qfwnumber][scaler-channelid]= X_wirenumber
   // gbeamY[gridnumber ][boardnumber][qfwnumber][scaler-channelid]= Y_wirenumber

   // to begin with we assume both boards have same grid mapping JAM
   for (int gb = 0; gb < OPTIC_QFWBOARDS; ++gb)
            {
         // [gridnumber ][boardnumber][qfwnumber][scaler-channelid]
         gBeamX[gb][gb][0][0] = 0;
         gBeamX[gb][gb][0][1] = 1;
         gBeamX[gb][gb][0][2] = 2;
         gBeamX[gb][gb][0][3] = 3;
         gBeamX[gb][gb][1][0] = 4;
         gBeamX[gb][gb][1][1] = 5;
         gBeamX[gb][gb][1][2] = 6;
         gBeamX[gb][gb][1][3] = 7;
         gBeamX[gb][gb][2][0] = 8;
         gBeamX[gb][gb][2][1] = 9;
         gBeamX[gb][gb][2][2] = 10;
         gBeamX[gb][gb][2][3] = 11;
         gBeamX[gb][gb][3][0] = 12;
         gBeamX[gb][gb][3][1] = 13;
         gBeamX[gb][gb][3][2] = 14;
         gBeamX[gb][gb][3][3] = 15;

         gBeamY[gb][gb][4][0] = 15;
         gBeamY[gb][gb][4][1] = 14;
         gBeamY[gb][gb][4][2] = 13;
         gBeamY[gb][gb][4][3] = 12;
         gBeamY[gb][gb][5][0] = 11;
         gBeamY[gb][gb][5][1] = 10;
         gBeamY[gb][gb][5][2] = 9;
         gBeamY[gb][gb][5][3] = 8;
         gBeamY[gb][gb][6][0] = 7;
         gBeamY[gb][gb][6][1] = 6;
         gBeamY[gb][gb][6][2] = 5;
         gBeamY[gb][gb][6][3] = 4;
         gBeamY[gb][gb][7][0] = 3;
         gBeamY[gb][gb][7][1] = 2;
         gBeamY[gb][gb][7][2] = 1;
         gBeamY[gb][gb][7][3] = 0;

            } // gb
}
