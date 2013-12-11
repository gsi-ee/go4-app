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
    TGo4EventProcessor(), fOutput(0)
{
}

//***********************************************************
// this one is used in standard factory
TQFWProfileProc::TQFWProfileProc(const char* name) :
    TGo4EventProcessor(name), fOutput(0)
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


  TString mtitle;
  // first loop over grids:

  for (unsigned g = 0; g < fGrids.size(); ++g)
  {
    TQFWGridDisplay* gridDisplay = fGrids[g];
    Int_t gridid = gridDisplay->GetDevId();
    TQFWGrid* gridData = fOutput->GetGrid(gridid);
    TQFWBoard* boardData = 0;
    Int_t oldboardid = -1;
    Bool_t gridfirst = kTRUE;
    for (int l = 0; l < gridDisplay->GetNumLoops(); ++l)
    {
      TQFWGridLoopDisplay* loopDisplay = gridDisplay->GetLoopDisplay(l);
      Bool_t loopfirst = kTRUE;

      for (int x = 0; x < gridData->GetNumXWires(); ++x)
      {
        TQFWChannelMap xmap = gridData->GetXChannelMap(x);
        if (oldboardid != (Int_t) xmap.fBoardID)
        {
          //TGo4Log::Info("Grid %d Retrieving X Board of id %d",gridid, xmap.fBoardID);
          boardData = QFWRawEvent->GetBoard(xmap.fBoardID);
          oldboardid = xmap.fBoardID;
          if (boardData == 0)
          {
            TGo4Log::Error("Configuration error: Board id %d does not exist as subevent!", xmap.fBoardID);
            return kFALSE;
          }
          if (gridfirst)
          {
            gridDisplay->AdjustDisplay(boardData);    // TODO: h
            gridfirst = kFALSE;
          }
        }
        TQFWLoop* loopData = boardData->GetLoop(l);
        if (loopData == 0)
        {
          TGo4Log::Error("Configuration error: Board id %d does not contain loopdata %d as subevent!", xmap.fBoardID,
              l);
          return kFALSE;
        }
        if (loopfirst)
        {
          loopDisplay->AdjustDisplay(loopData);
          loopfirst = kFALSE;
        }
        Int_t xchan = xmap.fQFWChannel;
        //printf("ProfileProc: processing board %d channel %d for grid %d X wire %d \n",
        //   xmap.fBoardID, xchan, gridid, x);
        if (xchan < 0)
          continue;    // skip non configured channels
        std::vector<Int_t> & trace = loopData->fQfwTrace[xchan];
        UInt_t sum = 0;
        for (unsigned t = 0; t < trace.size(); ++t)
        {
          if (fParam->fMeasureBackground)
          {
            fParam->AddXOffsetMeasurement(g, l, x, trace[t]);
          }
          loopDisplay->hBeamXSliceOffs->SetBinContent(1 + x, 1 + t, fParam->fQFWOffsetsX[g][l][x]);    // show current averaged offset
          Double_t value = fParam->GetCorrectedXValue(g, l, x, trace[t]);
          sum += value;
          loopDisplay->hBeamXSlice->SetBinContent(1 + x, 1 + t, value);

          Double_t prev = loopDisplay->hBeamAccXSlice->GetBinContent(1 + x, 1 + t);
          loopDisplay->hBeamAccXSlice->SetBinContent(1 + x, 1 + t, prev + value);
#ifdef QFW_STORECURRENTS
          gridData->fXCurrent.push_back(value);
#endif
//          sum += trace[t];
//          loopDisplay->hBeamXSlice->SetBinContent(1 + x, 1 + t, trace[t]);
//          prev = loopDisplay->hBeamAccXSlice->GetBinContent(1 + x, 1 + t);
//          loopDisplay->hBeamAccXSlice->SetBinContent(1 + x, 1 + t, prev + trace[t]);
//          //
        }    // trace t
        gridDisplay->hBeamX->SetBinContent(1 + x, sum);
        gridDisplay->hBeamAccX->AddBinContent(1 + x, sum);

      }    // x wires

      for (int y = 0; y < gridData->GetNumYWires(); ++y)
      {
        TQFWChannelMap ymap = gridData->GetYChannelMap(y);
        if (oldboardid != (Int_t) ymap.fBoardID)
        {
          //TGo4Log::Info("Grid %d Retrieving Y Board of id %d", gridid, ymap.fBoardID);
          boardData = QFWRawEvent->GetBoard(ymap.fBoardID);
          oldboardid = ymap.fBoardID;
          if (boardData == 0)
          {
            TGo4Log::Error("Configuration error: Board id %d does not exist as subevent!", ymap.fBoardID);
            return kFALSE;
          }
          // gridDisplay->AdjustDisplay(boardData);    // DO NOT do this again for y direction, will clear traces!

        }

        TQFWLoop* loopData = boardData->GetLoop(l);
        if (loopData == 0)
        {
          TGo4Log::Error("Configuration error: Board id %d does not contain loopdata %d as subevent!", ymap.fBoardID,
              l);
          return kFALSE;
        }
        Int_t ychan = ymap.fQFWChannel;
        //printf("ProfileProc: processing board %d channel %d for grid %d Y wire %d \n",
        //           ymap.fBoardID, ychan, gridid, y);
        if (ychan < 0)
          continue;    // skip non configured channels
        std::vector<Int_t> & trace = loopData->fQfwTrace[ychan];
        UInt_t sum = 0;
        for (unsigned t = 0; t < trace.size(); ++t)
        {

          if (fParam->fMeasureBackground)
          {
            fParam->AddYOffsetMeasurement(g, l, y, trace[t]);
          }
          loopDisplay->hBeamYSliceOffs->SetBinContent(1 + y, 1 + t, fParam->fQFWOffsetsX[g][l][y]);    // show current averaged offset
          Double_t value = fParam->GetCorrectedYValue(g, l, y, trace[t]);
          sum += value;
          loopDisplay->hBeamYSlice->SetBinContent(1 + y, 1 + t, value);
#ifdef QFW_STORECURRENTS
          gridData->fYCurrent.push_back(value);
#endif
          Double_t prev = loopDisplay->hBeamAccYSlice->GetBinContent(1 + y, 1 + t);
          loopDisplay->hBeamAccYSlice->SetBinContent(1 + y, 1 + t, prev + value);

        }    // trace t
        gridDisplay->hBeamY->SetBinContent(1 + y, sum);
        gridDisplay->hBeamAccY->AddBinContent(1 + y, sum);

      }    // y wires

    }    // loops
    
    // put here mean value calculations and profiles:

    gridData->fBeamMeanX = gridDisplay->hBeamX->GetMean();

    gridData->fBeamMeanY = gridDisplay->hBeamY->GetMean();

    gridData->fBeamRMSX = gridDisplay->hBeamX->GetRMS();

    gridData->fBeamRMSY = gridDisplay->hBeamY->GetRMS();

    gridDisplay->hBeamMeanXY->Fill(gridData->fBeamMeanX, gridData->fBeamMeanY);
    gridDisplay->hBeamRMSX->Fill(gridData->fBeamRMSX);
    gridDisplay->hBeamRMSY->Fill(gridData->fBeamRMSY);

    // second loop loop to get mean values of traces:
    for (int l = 0; l < gridDisplay->GetNumLoops(); ++l)
    {
      TQFWGridLoopDisplay* loopDisplay = gridDisplay->GetLoopDisplay(l);
      // evaluate here mean value and sigma of profile counts
      //first x direction:
      Int_t cmax = loopDisplay->cBeamXSliceCond->GetCMax(loopDisplay->hBeamXSlice);
      TH1I haux("temp", "temp", cmax, 0, cmax);    // auxiliary histogram to calculate mean and rms of counts
      for (int wire = 0; wire < gridData->GetNumXWires(); ++wire)
      {
        for (int time = 0; time < loopDisplay->GetTimeSlices(); ++time)
        {
          if (loopDisplay->cBeamXSliceCond->Test(wire, time))
          {
            haux.Fill(loopDisplay->hBeamXSlice->GetBinContent(wire + 1, time + 1));
          }
        }

      }
      Double_t MeanCountsX = haux.GetMean();
      Double_t RMSCountsX = haux.GetRMS();
      mtitle.Form("%s mean=%.2E sigma=%.2E", loopDisplay->hBeamXSlice->GetTitle(), MeanCountsX, RMSCountsX);
      loopDisplay->hBeamXSlice->SetTitle(mtitle.Data());
//
//      // y direction:

      Int_t cmay = loopDisplay->cBeamYSliceCond->GetCMax(loopDisplay->hBeamYSlice);
      TH1I hauy("temp2", "temp2", cmay, 0, cmay);    // auxiliary histogram to calculate mean and rms of counts
      for (int wire = 0; wire < gridData->GetNumYWires(); ++wire)
      {
        for (int time = 0; time < loopDisplay->GetTimeSlices(); ++time)
        {
          if (loopDisplay->cBeamYSliceCond->Test(wire, time))
          {
            hauy.Fill(loopDisplay->hBeamYSlice->GetBinContent(wire + 1, time + 1));
          }
        }

      }
      Double_t MeanCountsY = hauy.GetMean();
      Double_t RMSCountsY = hauy.GetRMS();
      mtitle.Form("%s mean=%.2E sigma=%.2E", loopDisplay->hBeamYSlice->GetTitle(), MeanCountsY, RMSCountsY);
      loopDisplay->hBeamYSlice->SetTitle(mtitle.Data());

      loopDisplay->hBeamMeanCountsX->Fill(MeanCountsX);
      loopDisplay->hBeamMeanCountsY->Fill(MeanCountsY);
      loopDisplay->hBeamRMSCountsX->Fill(RMSCountsX);
      loopDisplay->hBeamRMSCountsY->Fill(RMSCountsY);

    }    // loops
    
  }    // grids


  // TODO: here fill segmented cup displays
  
  /////////////////////////////////////////////////////////////7

  for (unsigned c = 0; c < fCups.size(); ++c)
   {
     TQFWCupDisplay* cupDisplay = fCups[c];
     Int_t cupid = cupDisplay->GetDevId();
     TQFWCup* cupData = fOutput->GetCup(cupid);
     TQFWBoard* boardData = 0;
     Int_t oldboardid = -1;
     Bool_t cupfirst = kTRUE;

     for (int l = 0; l < cupDisplay->GetNumLoops(); ++l)
     {
       TQFWCupLoopDisplay* loopDisplay = cupDisplay->GetLoopDisplay(l);
       Bool_t loopfirst = kTRUE;

       Double_t segmentcharge[cupData->GetNumSegments()];
       Double_t chargesum=0;
       for (int x = 0; x < cupData->GetNumSegments(); ++x)
       {
         segmentcharge[x]=0;
         TQFWChannelMap xmap = cupData->GetChannelMap(x);
         if (oldboardid != (Int_t) xmap.fBoardID)
         {
           //TGo4Log::Info("Grid %d Retrieving X Board of id %d",gridid, xmap.fBoardID);
           boardData = QFWRawEvent->GetBoard(xmap.fBoardID);
           oldboardid = xmap.fBoardID;
           if (boardData == 0)
           {
             TGo4Log::Error("Configuration error: Board id %d does not exist as subevent!", xmap.fBoardID);
             return kFALSE;
           }
           if (cupfirst)
           {
             cupDisplay->AdjustDisplay(boardData);    // TODO: h
             cupfirst = kFALSE;
           }
         }
         TQFWLoop* loopData = boardData->GetLoop(l);
         if (loopData == 0)
         {
           TGo4Log::Error("Configuration error: Board id %d does not contain loopdata %d as subevent!", xmap.fBoardID,
               l);
           return kFALSE;
         }
         if (loopfirst)
         {
           loopDisplay->AdjustDisplay(loopData);
           loopfirst = kFALSE;
         }
         Int_t xchan = xmap.fQFWChannel;
         //printf("ProfileProc: processing board %d channel %d for grid %d X wire %d \n",
         //   xmap.fBoardID, xchan, gridid, x);
         if (xchan < 0)
           continue;    // skip non configured channels
         std::vector<Int_t> & trace = loopData->fQfwTrace[xchan];
         UInt_t sum = 0;
         for (unsigned t = 0; t < trace.size(); ++t)
         {
           if (fParam->fMeasureBackground)
           {
             fParam->AddCupOffsetMeasurement(c, l, x, trace[t]);
           }
           loopDisplay->hCupSliceOffs->SetBinContent(1 + x, 1 + t, fParam->fQFWOffsetsCup[c][l][x]);    // show current averaged offset
           Double_t value = fParam->GetCorrectedCupValue(c, l, x, trace[t]);
           sum += value;
           loopDisplay->hCupSlice->SetBinContent(1 + x, 1 + t, value);

           Double_t prev = loopDisplay->hAccCupSlice->GetBinContent(1 + x, 1 + t);
           loopDisplay->hAccCupSlice->SetBinContent(1 + x, 1 + t, prev + value);
 #ifdef QFW_STORECURRENTS
           cupData->fCurrent.push_back(value);
 #endif
         }    // trace t
         cupDisplay->hCupScaler->SetBinContent(1 + x, sum);
         cupDisplay->hCupAccScaler->AddBinContent(1 + x, sum);
         segmentcharge[x]=sum;
         chargesum+=sum;

       }    // segments

       // here ratio histograms between segments:
     for(unsigned seg=0; seg<loopDisplay->hSegmentRatio.size();++seg)
            {
               if (seg>= (unsigned) cupData->GetNumSegments())
               {
                 TGo4Log::Error("Configuration error: Display segments index %d exceeds number of data segments %d ",
                     seg, cupData->GetNumSegments());
                 continue;
               }
               Double_t value=0;
               if(chargesum) value=segmentcharge[seg]/chargesum;
               loopDisplay->hSegmentRatio[seg]->Fill(value);
               loopDisplay->hAccSegmentRatio[seg]->Fill(value);
            }

     }    // loops





   }    // cups
  // end cup display


  fOutput->SetValid(kTRUE);    // to store

  if(fParam->fSlowMotionStart>0)
   if(QFWRawEvent->fSequenceNumber > (Int_t) fParam->fSlowMotionStart)
     GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode at event of sequence number %d",QFWRawEvent->fSequenceNumber);




  return kTRUE;
}

