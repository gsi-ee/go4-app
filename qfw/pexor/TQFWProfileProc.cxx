#include "TQFWProfileProc.h"

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

//***********************************************************
TQFWProfileProc::TQFWProfileProc() :
    TGo4EventProcessor()
{
}

//***********************************************************
// this one is used in standard factory
TQFWProfileProc::TQFWProfileProc(const char* name) :
    TGo4EventProcessor(name)
{
  TGo4Log::Info("TQFWProfileProc: Create instance %s", name);

  SetMakeWithAutosave(kTRUE);
  //// init user analysis objects:

  fParam = dynamic_cast<TQFWProfileParam*>(MakeParameter("QFWProfileParam", "TQFWProfileParam", "set_QfwProfilePar.C"));
  if (fParam)
    fParam->SetEventConfig();

  // setup grid displays from config:
  for (int i = 0; i < fParam->fNumGrids; ++i)
  {
    UInt_t uniqueid = fParam->fGridDeviceID[i];
    fGrids.push_back(new TQFWGridDisplay(uniqueid));
  }

  // setup grid displays from config:
  for (int i = 0; i < fParam->fNumCups; ++i)
  {
    UInt_t uniqueid = fParam->fCupDeviceID[i];
    fCups.push_back(new TQFWCupDisplay(uniqueid));
  }

  //InitDisplay(PEXOR_QFWSLICES);
}

//***********************************************************
TQFWProfileProc::~TQFWProfileProc()
{
  TGo4Log::Info("TQFWProfileProc: Delete instance");
  for (unsigned i = 0; i < fGrids.size(); ++i)
  {
    delete fGrids[i];
  }
  for (unsigned i = 0; i < fCups.size(); ++i)
  {
    delete fCups[i];
  }

}

void TQFWProfileProc::InitDisplay(int timeslices, Bool_t replace)
{
  cout << "**** TQFWProfileProc: Init Display for " << timeslices << " time slices. " << endl;
  if (replace)    //TGo4Analysis::Instance()->
    SetMakeWithAutosave(kFALSE);

  for (unsigned i = 0; i < fGrids.size(); ++i)
  {
    fGrids[i]->SetGridEvent(fOutput);
    fGrids[i]->InitDisplay(timeslices);
  }
  for (unsigned i = 0; i < fCups.size(); ++i)
  {
    fCups[i]->SetCupEvent(fOutput);
    fCups[i]->InitDisplay(timeslices);
  }

}

//-----------------------------------------------------------
// event function
Bool_t TQFWProfileProc::BuildEvent(TGo4EventElement* target)
{
  // called by framework from TQFWRawEvent to fill it

  if (fOutput == 0)
  {
    fOutput = (TQFWProfileEvent*) target;
    InitDisplay(PEXOR_QFWSLICES, kTRUE);    // recreate histograms with true wire numbers from output event
  }

  TQFWRawEvent* QFWRawEvent = (TQFWRawEvent*) GetInputEvent();
  fOutput->SetValid(kFALSE);    // not store

  // FillGrids(QFWRawEvent);

  QFWRawEvent->SetValid(kTRUE);    // to store
  TString mtitle;
  // first loop over grids:

  for (unsigned g = 0; g < fGrids.size(); ++g)
  {
    TQFWGridDisplay* gridDisplay = fGrids[g];
    Int_t gridid = gridDisplay->GetDevId();
    TQFWGrid* gridData = fOutput->GetGrid(gridid);
    TQFWBoard* boardData = 0;
    Int_t oldboardid = -1;
    Bool_t gridfirst=kTRUE;
    for (unsigned l = 0; l < gridDisplay->GetNumLoops(); ++l)
    {
      TQFWGridLoopDisplay* loopDisplay = gridDisplay->GetLoopDisplay(l);
      Bool_t loopfirst=kTRUE;

      for (unsigned x = 0; x < gridData->GetNumXWires(); ++x)
      {
        TQFWChannelMap xmap = gridData->GetXChannelMap(x);
        if (oldboardid != xmap.fBoardID)
        {
          //TGo4Log::Info("Grid %d Retrieving X Board of id %d",gridid, xmap.fBoardID);
          boardData = QFWRawEvent->GetBoard(xmap.fBoardID);
          oldboardid = xmap.fBoardID;
          if (boardData == 0)
          {
            TGo4Log::Error("Configuration error: Board id %d does not exist as subevent!", xmap.fBoardID);
            return kFALSE;
          }
         if(gridfirst)
           {
             gridDisplay->AdjustDisplay(boardData);    // TODO: h
             gridfirst=kFALSE;
           }
        }
        TQFWLoop* loopData = boardData->GetLoop(l);
        if (loopData == 0)
        {
          TGo4Log::Error("Configuration error: Board id %d does not contain loopdata %d as subevent!", xmap.fBoardID,
              l);
          return kFALSE;
        }
        if(loopfirst)
        {
          loopDisplay->AdjustDisplay(loopData);
          loopfirst=kFALSE;
        }
        Int_t xchan = xmap.fQFWChannel;
        std::vector<Int_t> & trace = loopData->fQfwTrace[xchan];
        UInt_t sum = 0;
        for (unsigned t = 0; t < trace.size(); ++t)
        {
          // TODO: offset accumulation/correction also store in output event
          //                        // JAM: newly added correction and display of background offset:
          //                        if(fPar->fMeasureBackground)
          //                           {
          //                              fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
          //                           }
          //                        hBeamXSliceOffs[grid]->SetBinContent(1 + xpos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
          /////////////////////
          sum += trace[t];
          loopDisplay->hBeamXSlice->SetBinContent(1 + x, 1 + t, trace[t]);
          UInt_t prev = loopDisplay->hBeamAccXSlice->GetBinContent(1 + x, 1 + t);
          loopDisplay->hBeamAccXSlice->SetBinContent(1 + x, 1 + t, prev + trace[t]);
          //                     }
          //
          gridDisplay->hBeamX->SetBinContent(1 + x, sum);
          gridDisplay->hBeamAccX->AddBinContent(1 + x, sum);

        }    // trace t

      }    // x wires

      for (unsigned y = 0; y < gridData->GetNumYWires(); ++y)
      {
        TQFWChannelMap ymap = gridData->GetYChannelMap(y);
        if (oldboardid != ymap.fBoardID)
        {
          //TGo4Log::Info("Grid %d Retrieving Y Board of id %d", gridid, ymap.fBoardID);
          boardData = QFWRawEvent->GetBoard(ymap.fBoardID);
          oldboardid = ymap.fBoardID;
          if (boardData == 0)
          {
            TGo4Log::Error("Configuration error: Board id %d does not exist as subevent!", ymap.fBoardID);
            return kFALSE;
          }
          gridDisplay->AdjustDisplay(boardData);    // TODO: h

        }

        TQFWLoop* loopData = boardData->GetLoop(l);
        if (loopData == 0)
        {
          TGo4Log::Error("Configuration error: Board id %d does not contain loopdata %d as subevent!", ymap.fBoardID,
              l);
          return kFALSE;
        }
        Int_t ychan = ymap.fQFWChannel;
        std::vector<Int_t> & trace = loopData->fQfwTrace[ychan];
        UInt_t sum = 0;
        for (unsigned t = 0; t < trace.size(); ++t)
        {
          // TODO: offset accumulation/correction also store in output event
          //                        // JAM: newly added correction and display of background offset:
          //                        if(fPar->fMeasureBackground)
          //                           {
          //                              fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
          //                           }
          //                        hBeamXSliceOffs[grid]->SetBinContent(1 + xpos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
          /////////////////////
          sum += trace[t];
          loopDisplay->hBeamYSlice->SetBinContent(1 + y, 1 + t, trace[t]);
          UInt_t prev = loopDisplay->hBeamAccYSlice->GetBinContent(1 + y, 1 + t);
          loopDisplay->hBeamAccYSlice->SetBinContent(1 + y, 1 + t, prev + trace[t]);
          //                     }
          //
          gridDisplay->hBeamY->SetBinContent(1 + y, sum);
          gridDisplay->hBeamAccY->AddBinContent(1 + y, sum);

        }    // trace t

      }    // y wires

    }    // loops

    
   // put here mean value calculations and profiles:
   
   
     gridData->fBeamMeanX=gridDisplay->hBeamX->GetMean();

     gridData->fBeamMeanY=gridDisplay->hBeamY->GetMean();

     gridData->fBeamRMSX=gridDisplay->hBeamX->GetRMS();

     gridData->fBeamRMSY=gridDisplay->hBeamY->GetRMS();

      gridDisplay->hBeamMeanXY->Fill(gridData->fBeamMeanX,gridData->fBeamMeanY);
      gridDisplay->hBeamRMSX->Fill(gridData->fBeamRMSX);
      gridDisplay->hBeamRMSY->Fill(gridData->fBeamRMSY);
      
      // second loop loop to get mean values of traces:
      for (unsigned l = 0; l < gridDisplay->GetNumLoops(); ++l)
      {
	TQFWGridLoopDisplay* loopDisplay=gridDisplay->GetLoopDisplay(l);
     // evaluate here mean value and sigma of profile counts
    //first x direction:
      Int_t cmax=loopDisplay->cBeamXSliceCond->GetCMax(loopDisplay->hBeamXSlice);
      TH1I haux("temp","temp",cmax,0,cmax); // auxiliary histogram to calculate mean and rms of counts
     for(int wire=0;wire<gridData->GetNumXWires();++wire)
     {
        for(int time=0; time<loopDisplay->GetTimeSlices();++time)
        {
           if(loopDisplay->cBeamXSliceCond->Test(wire,time))
              {
                 haux.Fill(loopDisplay->hBeamXSlice->GetBinContent(wire+1,time+1));
              }
        }

     }
     Double_t MeanCountsX=haux.GetMean();
     Double_t RMSCountsX=haux.GetRMS();
     mtitle.Form("%s mean=%.2E sigma=%.2E", loopDisplay->hBeamXSlice->GetTitle(), MeanCountsX, RMSCountsX);
     loopDisplay->hBeamXSlice->SetTitle(mtitle.Data());
//
//      // y direction:
     
     
     
      Int_t cmay=loopDisplay->cBeamYSliceCond->GetCMax(loopDisplay->hBeamYSlice);
      TH1I hauy("temp2","temp2",cmay,0,cmay); // auxiliary histogram to calculate mean and rms of counts
     for(int wire=0;wire<gridData->GetNumYWires();++wire)
     {
        for(int time=0; time<loopDisplay->GetTimeSlices();++time)
        {
           if(loopDisplay->cBeamYSliceCond->Test(wire,time))
              {
                 hauy.Fill(loopDisplay->hBeamYSlice->GetBinContent(wire+1,time+1));
              }
        }

     }
     Double_t MeanCountsY=hauy.GetMean();
     Double_t RMSCountsY=hauy.GetRMS();
     mtitle.Form("%s mean=%.2E sigma=%.2E", loopDisplay->hBeamYSlice->GetTitle(), MeanCountsY, RMSCountsY);
     loopDisplay->hBeamYSlice->SetTitle(mtitle.Data());
     
     
     
     


     loopDisplay->hBeamMeanCountsX->Fill(MeanCountsX);
     loopDisplay->hBeamMeanCountsY->Fill(MeanCountsY);
     loopDisplay->hBeamRMSCountsX->Fill(RMSCountsX);
     loopDisplay->hBeamRMSCountsY->Fill(RMSCountsY);

      } // loops
    
    
    
    
    
    
  }    // grids

  
  
 
  
  
  
  
  
  //if(fPar->fSlowMotionStart>0)
  // if(evnum>fPar->fSlowMotionStart)
  //   GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode at event %d",evnum);

  return kTRUE;
}

//void TQFWProfileProc::FillGrids(TQFWRawEvent* out)
//{
//
//   Bool_t dostop=kFALSE;
//   Double_t mtime=out->fQfwMTime * 20 / 1000; // measurement time in us
//   Double_t premtime = 0; // measurement time in us
//
//   /* evaluate measurement setup*/
//   TString setup;
//   switch(out->fQfwSetup)
//   {
//      case 0:
//         setup.Form("(-) [ 2.5pF & 0.25pC]");
//      break;
//
//      case 1:
//         setup.Form("(-) [25.0pF & 2.50pC]");
//      break;
//
//      case 2:
//         setup.Form("(+) [ 2.5pF & 0.25pC]");
//      break;
//
//      case 3:
//         setup.Form("(+) [25.0pF & 2.50pC]");
//      break;
//
//      case 0x10:
//         setup.Form("1000uA (-) [ 2.5pF & 0.25pC]");
//      break;
//
//      case 0x11:
//         setup.Form("1000uA (-) [25.0pF & 2.50pC]");
//      break;
//
//      case 0x12:
//         setup.Form("1000uA (+) [ 2.5pF & 0.25pC]");
//      break;
//
//      case 0x13:
//         setup.Form("1000uA (+) [25.0pF & 2.50pC]");
//         break;
//
//      default:
//         setup.Form("unknown setup %d", out->fQfwSetup);
//         break;
//
//
//
//   };
//
//
//
//
//
//// APPEND TIME RANGES:
//   TString mtitle;
//   mtitle.Form("%s dt=%.2E us", setup.Data(), mtime);
//
//   for (int grid = 0;  grid< PEXOR_QFWGRIDS; ++grid)
//      {
//      hBeamX[grid]->Reset("");
//      hBeamY[grid]->Reset("");
//      hBeamXSlice[grid]->Reset("");
//      hBeamYSlice[grid]->Reset("");
//      hBeamXSliceOffs[grid]->Reset("");
//      hBeamYSliceOffs[grid]->Reset("");
//      hBeamX[grid]->SetTitle(mtitle.Data());
//      hBeamY[grid]->SetTitle(mtitle.Data());
//      hBeamXSlice[grid]->SetTitle(mtitle.Data());
//      hBeamYSlice[grid]->SetTitle(mtitle.Data());
//      mtitle.Form("%s dt=%.2E us", setup.Data(),premtime);
//
//
//
//
//   for (int brd = 0;  brd< PEXOR_QFWBOARDS; ++brd)
//   {
//
//
//
//      for (int q = 0; q < PEXOR_QFWNUM; ++q)
//      {
//         for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
//         {
//            int xpos = gBeamX[grid][brd][q][ch];
//            int ypos = gBeamY[grid][brd][q][ch];
//            if (xpos >= 0)
//            {
//               UInt_t sum=0;
//               for (int sl = 0; sl < PEXOR_QFWSLICES; ++sl)
//                     {
//                        // JAM: newly added correction and display of background offset:
//                        if(fPar->fMeasureBackground)
//                           {
//                              fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
//                           }
//                        hBeamXSliceOffs[grid]->SetBinContent(1 + xpos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
//                        sum+=fPar->GetCorrectedQFWValue(brd,q,ch,sl, out->fQfw[brd][q][ch][sl]);
//                        hBeamXSlice[grid]->SetBinContent(1 + xpos, 1+sl, fPar->GetCorrectedQFWValue(brd,q,ch,sl, out->fQfw[brd][q][ch][sl]));
//                        UInt_t prev=hBeamAccXSlice[grid]->GetBinContent(1 + xpos,1+sl);
//                        hBeamAccXSlice[grid]->SetBinContent(1 + xpos,1+sl, prev+ fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
//                     }
//
//               hBeamX[grid]->SetBinContent(1 + xpos, sum);
//               hBeamAccX[grid]->AddBinContent(1 + xpos, sum);
//            }
//            if (ypos >= 0)
//            {
//               UInt_t sum=0;
//               for (int sl = 0; sl < PEXOR_QFWSLICES; ++sl)
//                     {
//                        if(fPar->fMeasureBackground)
//                           {
//                              fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
//                           }
//                        hBeamYSliceOffs[grid]->SetBinContent(1 + ypos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
//                        sum+=fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
//                        hBeamYSlice[grid]->SetBinContent(1 + ypos, 1+sl, fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
//                        UInt_t prev=hBeamAccYSlice[grid]->GetBinContent(1 + ypos,1+sl);
//                        hBeamAccYSlice[grid]->SetBinContent(1 + ypos, 1+sl, prev + fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
//                     }
//
//               hBeamY[grid]->SetBinContent(1 + ypos, sum);
//               hBeamAccY[grid]->AddBinContent(1 + ypos, sum);
//
//               if(grid==0 && ypos==6)
//                  {
//                  // condition to find broken wire:
//                  if(sum<100)
//                  {
//                     dostop=kTRUE;
//                  }
//               }
//
//            }
//         }
//
//      }
//
//   }// board loop
//
//
//
//
//
//
//      out->fBeamMeanX[grid]=hBeamX[grid]->GetMean();
//
//      out->fBeamMeanY[grid]=hBeamY[grid]->GetMean();
//
//      out->fBeamRMSX[grid]=hBeamX[grid]->GetRMS();
//
//      out->fBeamRMSY[grid]=hBeamY[grid]->GetRMS();
//
//
//      hBeamMeanXY[grid]->Fill(out->fBeamMeanX[grid],out->fBeamMeanY[grid]);
//
//      hBeamRMSX[grid]->Fill(out->fBeamRMSX[grid]);
//      hBeamRMSY[grid]->Fill(out->fBeamRMSY[grid]);
//
//      // evaluate here mean value and sigma of profile counts
//      // first x direction:
//      Int_t cmax=cBeamXSliceCond[grid]->GetCMax(hBeamXSlice[grid]);
//      TH1I haux("temp","temp",cmax,0,cmax); // auxiliary histogram to calculate mean and rms of counts
//      for(int x=0;x<gBeamWiresX[grid];++x)
//      {
//         for(int y=0; y<PEXOR_QFWSLICES;++y)
//         {
//            if(cBeamXSliceCond[grid]->Test(x,y))
//               {
//                  haux.Fill(hBeamXSlice[grid]->GetBinContent(x+1,y+1));
//               }
//         }
//
//      }
//      Double_t MeanCountsX=haux.GetMean();
//      Double_t RMSCountsX=haux.GetRMS();
//      mtitle.Form("%s mean=%.2E sigma=%.2E", hBeamXSlice[grid]->GetTitle(), MeanCountsX, RMSCountsX);
//      hBeamXSlice[grid]->SetTitle(mtitle.Data());
//
//      // y direction:
//      Int_t cmay=cBeamYSliceCond[grid]->GetCMax(hBeamYSlice[grid]);
//      TH1I hauy("temp2","temp2",cmay,0,cmay); // auxiliary histogram to calculate mean and rms of counts
//      for(int x=0;x<gBeamWiresY[grid];++x)
//      {
//         for(int y=0; y<PEXOR_QFWSLICES;++y)
//         {
//            if(cBeamYSliceCond[grid]->Test(x,y))
//               {
//                  hauy.Fill(hBeamYSlice[grid]->GetBinContent(x+1,y+1));
//               }
//         }
//
//      }
//      Double_t MeanCountsY=hauy.GetMean();
//      Double_t RMSCountsY=hauy.GetRMS();
//      mtitle.Form("%s mean=%.2E sigma=%.2E", hBeamYSlice[grid]->GetTitle(), MeanCountsY, RMSCountsY);
//      hBeamYSlice[grid]->SetTitle(mtitle.Data());
//
//      hBeamMeanCountsX[grid]->Fill(MeanCountsX);
//      hBeamMeanCountsY[grid]->Fill(MeanCountsY);
//      hBeamRMSCountsX[grid]->Fill(RMSCountsX);
//      hBeamRMSCountsY[grid]->Fill(RMSCountsY);
//
//   }// grid loop
//
//
//   if(dostop)
//      {
//         TGo4Analysis::Instance()->SendObjectToGUI(hBeamYSlice[0]);
//         TGo4Analysis::Instance()->SendObjectToGUI(hBeamYSlice[0]);
//         GO4_STOP_ANALYSIS_MESSAGE("Found wire 7 below 100 counts");
//
//      }
//
//}

//void TQFWProfileProc::MapGrids()
//{

//   cout << "**** TQFWProfileProc: Setting up beam coordinate mapping:" << endl;
//   // map both for scalers and prescalers
//
//
//   // first we reset all mappings:
//   for (int g = 0; g < PEXOR_QFWGRIDS; ++g)
//      {
//
//      gBeamWiresX[g]=16;
//      gBeamWiresY[g]=16;
//
//      for (int b = 0; b < PEXOR_QFWBOARDS; ++b)
//         {
//            for (int qfw = 0; qfw < PEXOR_QFWNUM; ++qfw)
//            {
//               for (int c = 0; c < PEXOR_QFWCHANS; ++c)
//               {
//                  gBeamX[g][b][qfw][c] = -1; // reset unused channels
//                  gBeamY[g][b][qfw][c] = -1;
//               } // channels
//            } // qfw
//
//         } // boards
//      } // grids
//
//
//   // HERE IS THE GRID-MAPPING:
//   // gbeamX[gridnumber ][boardnumber][qfwnumber][scaler-channelid]= X_wirenumber
//   // gbeamY[gridnumber ][boardnumber][qfwnumber][scaler-channelid]= Y_wirenumber
//
//   // to begin with we assume both boards have same grid mapping JAM
//   for (int gb = 0; gb < PEXOR_QFWBOARDS; ++gb)
//            {
//         // [gridnumber ][boardnumber][qfwnumber][scaler-channelid]
//         gBeamX[gb][gb][0][0] = 0;
//         gBeamX[gb][gb][0][1] = 1;
//         gBeamX[gb][gb][0][2] = 2;
//         gBeamX[gb][gb][0][3] = 3;
//         gBeamX[gb][gb][1][0] = 4;
//         gBeamX[gb][gb][1][1] = 5;
//         gBeamX[gb][gb][1][2] = 6;
//         gBeamX[gb][gb][1][3] = 7;
//         gBeamX[gb][gb][2][0] = 8;
//         gBeamX[gb][gb][2][1] = 9;
//         gBeamX[gb][gb][2][2] = 10;
//         gBeamX[gb][gb][2][3] = 11;
//         gBeamX[gb][gb][3][0] = 12;
//         gBeamX[gb][gb][3][1] = 13;
//         gBeamX[gb][gb][3][2] = 14;
//         gBeamX[gb][gb][3][3] = 15;
//
//         gBeamY[gb][gb][4][0] = 15;
//         gBeamY[gb][gb][4][1] = 14;
//         gBeamY[gb][gb][4][2] = 13;
//         gBeamY[gb][gb][4][3] = 12;
//         gBeamY[gb][gb][5][0] = 11;
//         gBeamY[gb][gb][5][1] = 10;
//         gBeamY[gb][gb][5][2] = 9;
//         gBeamY[gb][gb][5][3] = 8;
//         gBeamY[gb][gb][6][0] = 7;
//         gBeamY[gb][gb][6][1] = 6;
//         gBeamY[gb][gb][6][2] = 5;
//         gBeamY[gb][gb][6][3] = 4;
//         gBeamY[gb][gb][7][0] = 3;
//         gBeamY[gb][gb][7][1] = 2;
//         gBeamY[gb][gb][7][2] = 1;
//         gBeamY[gb][gb][7][3] = 0;
//
//            } // gb
//}
