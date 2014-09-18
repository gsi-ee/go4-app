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

  fParam =
      dynamic_cast<TQFWProfileParam*>(MakeParameter("QFWProfileParam", "TQFWProfileParam", "set_QFWProfileParam.C"));
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

  // here we try to dump the mean wire statistics for Michael W.:

  for (unsigned g = 0; g < fGrids.size(); ++g)
   {
     // mind the different indices!
     TQFWGridDisplay* gridDisplay = fGrids[g];    // g      =vector index of display component
     Int_t gridid = gridDisplay->GetDevId();    // gridid =unique hardware id
     //TQFWGrid* gridData = fOutput->GetGrid(gridid);
     printf("############# Grid %d \n",gridid);
     for (int l = 0; l < gridDisplay->GetNumLoops(); ++l)
     {
       TQFWGridLoopDisplay* loopDisplay = gridDisplay->GetLoopDisplay(l);
       printf("#\tLoop %d\n",l);
       printf("#\t\tX-Direction:\n");
       Double_t deltaS=0;
       if(loopDisplay->hBeamMeanCountsX->GetMean()!=0) deltaS=loopDisplay->hBeamRMSCountsX->GetRMS()/loopDisplay->hBeamMeanCountsX->GetMean();
       printf("#\t\t\tTotal - \tMean:%f \tSigma:%f \tdSigma/Mean:%f\n",loopDisplay->hBeamMeanCountsX->GetMean(), loopDisplay->hBeamRMSCountsX->GetRMS(),deltaS);
       //for (int x = 0; x < gridData->GetNumXWires(); ++x)
       printf("#\t\t\tWire: \tMean: \t\tSigma: \tdSigma/Mean\n");
       printf("#--------------------------------------------------------\n");
       for (int x = 0; x < PEXOR_QFW_WIRES; ++x)
           {
             Double_t deltaS=0;
             if(loopDisplay->hBeamMeanCountsGridX[x]->GetMean()!=0) deltaS=loopDisplay->hBeamRMSCountsGridX[x]->GetRMS()/loopDisplay->hBeamMeanCountsGridX[x]->GetMean();
             printf("\t\t\t%d\t%f\t%f\t%f\n",x, loopDisplay->hBeamMeanCountsGridX[x]->GetMean(), loopDisplay->hBeamRMSCountsGridX[x]->GetRMS(),deltaS);
           }
       printf("#\t\tY-Direction:\n");
       deltaS=0;
       if(loopDisplay->hBeamMeanCountsY->GetMean()!=0) deltaS=loopDisplay->hBeamRMSCountsY->GetRMS()/loopDisplay->hBeamMeanCountsY->GetMean();

       printf("#\t\t\tTotal: \tMean:%f \tSigma:%f\tdSigma/Mean:%f\n",loopDisplay->hBeamMeanCountsY->GetMean(), loopDisplay->hBeamRMSCountsY->GetRMS(),deltaS);
       printf("#\t\t\tWire: \tMean: \t\tSigma: \tdSigma/Mean\n");
       printf("#--------------------------------------------------------\n");
           //for (int y = 0; y < gridData->GetNumYWires(); ++y)
       for (int y = 0; y < PEXOR_QFW_WIRES; ++y)
       {
         Double_t deltaS=0;
         if(loopDisplay->hBeamMeanCountsGridY[y]->GetMean()!=0) deltaS=loopDisplay->hBeamRMSCountsGridY[y]->GetRMS()/loopDisplay->hBeamMeanCountsGridY[y]->GetMean();
         printf("\t\t\t%d\t%f\t%f\t%f\n",y, loopDisplay->hBeamMeanCountsGridY[y]->GetMean(), loopDisplay->hBeamRMSCountsGridY[y]->GetRMS(),deltaS);
       }

     }
   }

  cout << endl;

  //////////////////
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
    fGrids[i]->SetProfileParam(fParam);
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

  TString mtitle;
  // first loop over grids:

  for (unsigned g = 0; g < fGrids.size(); ++g)
  {
    // mind the different indices!
    TQFWGridDisplay* gridDisplay = fGrids[g];    // g      =vector index of display component
    Int_t gridid = gridDisplay->GetDevId();    // gridid =unique hardware id
    Int_t gix = fParam->FindGridIndex(gridid);    // gix    =index in grid parameter array
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
        Double_t sum = 0;
        Double_t CperCount = loopData->GetCoulombPerCount();    // unit C
        Double_t TperSlice=1.0e-6 * loopData->GetMicroSecsPerTimeSlice();
        Double_t TperLoop = TperSlice * loopData->fQfwLoopSize;    // unit s

        for (unsigned t = 0; t < trace.size(); ++t)
        {
          if (fParam->fMeasureBackground)
          {
            fParam->AddXOffsetMeasurement(g, l, x, trace[t]);
          }
          // new: since bin number does not correspond to wire number anymore, have to find out bin for wire
          Int_t tracebin = loopDisplay->hBeamXSliceOffs->FindBin(x,t);
          Int_t binx=0; Int_t bint=0; Int_t dummy=0;
          loopDisplay->hBeamXSliceOffs->GetBinXYZ(tracebin, binx, bint, dummy);

          loopDisplay->hBeamXSliceOffs->SetBinContent(binx, bint, fParam->fQFWOffsetsX[g][l][x]);    // show current averaged offset
          //loopDisplay->hBeamXSliceOffs->Fill(x, t, fParam->fQFWOffsetsX[g][l][x]);
// 	  cout <<"BuildEvent: for g,l,x,t, trace[t]=("<<g<<","<<l<<","<<x<<","<<t<<",";
// 	  cout <<trace[t] << endl; 
          Double_t value = fParam->GetCorrectedXValue(g, l, x, trace[t]);
          sum += value;
          //loopDisplay->hBeamXSlice->SetBinContent(1 + x, 1 + t, value);
          loopDisplay->hBeamXSlice->SetBinContent(binx, bint, value); // assume all traces are scaled with same bindims
          //loopDisplay->hBeamXSlice->Fill(x, t, value);


          Double_t prev = loopDisplay->hBeamAccXSlice->GetBinContent(binx, bint);
          loopDisplay->hBeamAccXSlice->SetBinContent(binx, bint, prev + value);
//          Double_t prev = loopDisplay->hBeamAccXSlice->GetBinContent(1 + x, 1 + t);
//          loopDisplay->hBeamAccXSlice->SetBinContent(1 + x, 1 + t, prev + value);


          // charge and current traces:
          Double_t slicecharge=CperCount * value;
          Double_t slicecurrent=0;

          if(TperSlice) slicecurrent=slicecharge / TperSlice;

#ifdef QFW_STORECURRENTS
          gridData->fXCurrent.push_back(slicecurrent);
#endif

          // NOTE: we should not fill underflow/overflow bins for charge,
          // otherwise we are not able to calculate average current correctly!
          if (((fParam->fGridMinWire_X[gix] < 0) || (x >= fParam->fGridMinWire_X[gix]))
              && ((fParam->fGridMaxWire_X[gix] < 0) || (x < fParam->fGridMaxWire_X[gix])))
          {
            loopDisplay->hBeamChargeXSlice->SetBinContent(binx, bint, slicecharge);
            Double_t cprev = loopDisplay->hBeamAccChargeXSlice->GetBinContent(binx, bint);
            loopDisplay->hBeamAccChargeXSlice->SetBinContent(binx, bint, cprev + slicecharge);

            loopDisplay->hBeamCurrentXSlice->SetBinContent(binx, bint, slicecurrent);
            Int_t numsamples = 1
                + (Int_t) ((loopDisplay->hBeamAccChargeXSlice->GetEntries() - 1)
                    / loopDisplay->hBeamAccChargeXSlice->GetNbinsX() / loopDisplay->hBeamAccChargeXSlice->GetNbinsY());    // divide number of entries by bins to get number of charge measurements per segment
            Double_t chargesum = loopDisplay->hBeamAccChargeXSlice->GetBinContent(binx, bint);
            Double_t currentaverage = chargesum / numsamples / TperSlice;
            loopDisplay->hBeamAveCurrentXSlice->SetBinContent(binx, bint, currentaverage);

          }

        }    // trace t

        loopDisplay->hBeamLoopX->Fill(x, sum);
        loopDisplay->hBeamAccLoopX->Fill(x, sum);
        gridDisplay->hBeamX->Fill(x, sum);    // we need Fill instead SetBinContent to evaluate statistics
        gridDisplay->hBeamAccX->Fill(x, sum);
        // this one will not evaluate statistics! :
//        gridDisplay->hBeamX->AddBinContent(1 + x, sum);
//        gridDisplay->hBeamAccX->AddBinContent(1 + x, sum);

        // here position calibrated histograms:
        if (gix >= 0)
        {
          Double_t xpos = fParam->fGridPosition_X[gix][x];
          loopDisplay->hPosLoopX->Fill(xpos, sum);
          loopDisplay->hPosAccLoopX->Fill(xpos, sum);
          gridDisplay->hPosX->Fill(xpos, sum);    // we need Fill instead SetBinContent to evaluate statistics
          gridDisplay->hPosAccX->Fill(xpos, sum);

          // calibrated displays for charge and current-

          Double_t charge = CperCount * sum;
          Double_t current = 0;
          if (TperLoop)
            current = charge / TperLoop;    // unit A

          // NOTE: we should not fill underflow/overflow bins for charge,
          // otherwise we are not able to calculate average current correctly!
          // (need to know number charge samples in chargesum histogram)
          // => we check for wire range here!
          if (((fParam->fGridMinWire_X[gix] < 0) || (x >= fParam->fGridMinWire_X[gix]))
              && ((fParam->fGridMaxWire_X[gix] < 0) || (x < fParam->fGridMaxWire_X[gix])))
          {
            // only fill these if inside wire range or if full wire range is selected

            loopDisplay->hPosQLoopX->Fill(xpos, charge);
            loopDisplay->hPosQAccLoopX->Fill(xpos, charge);
            gridDisplay->hPosQ_X->Fill(xpos, charge);
            gridDisplay->hPosQAcc_X->Fill(xpos, charge);

            loopDisplay->hPosILoopX->Fill(xpos, current);

            // average current is bin content of accumulated charge by number of samples by loop time
            Int_t numsamples = 1
                + (Int_t) (( loopDisplay->hPosQAccLoopX->GetEntries() -1) / loopDisplay->hPosQAccLoopX->GetNbinsX());    // divide number of entries by bins to get number of charge measurements per wire
            Int_t xposbin = loopDisplay->hPosQAccLoopX->FindBin(xpos);
//            printf("XXXXXXXX I- grid %d loop%d xpos:%f xposbin:%d, entries:%f bins:%d numsamples=%d \n", g, l, xpos, xposbin,
//                loopDisplay->hPosQAccLoopX->GetEntries(), loopDisplay->hPosQAccLoopX->GetNbinsX(), numsamples);

            Double_t chargesum = loopDisplay->hPosQAccLoopX->GetBinContent(xposbin);
            Double_t currentaverage = chargesum / numsamples / TperLoop;
//            printf("----- chargesum: %e C currentaverage:%e A current:%e \n", chargesum, currentaverage, current);
            loopDisplay->hPosIAveLoopX->Fill(xpos, currentaverage);
          }

        }

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
        Double_t sum = 0;
        Double_t CperCount = loopData->GetCoulombPerCount();    // unit C
        Double_t TperSlice= 1.0e-6 * loopData->GetMicroSecsPerTimeSlice();
        Double_t TperLoop =  TperSlice * loopData->fQfwLoopSize;    // unit s
        for (unsigned t = 0; t < trace.size(); ++t)
        {

          if (fParam->fMeasureBackground)
          {
            fParam->AddYOffsetMeasurement(g, l, y, trace[t]);
          }

          // new: since bin number does not correspond to wire number anymore, have to find out bin for wire
          Int_t tracebin = loopDisplay->hBeamYSliceOffs->FindBin(y,t);
          Int_t biny=0; Int_t bint=0; Int_t dummy=0;
          loopDisplay->hBeamYSliceOffs->GetBinXYZ(tracebin, biny, bint, dummy);

          loopDisplay->hBeamYSliceOffs->SetBinContent(biny, bint, fParam->fQFWOffsetsY[g][l][y]);    // show current averaged offset
          Double_t value = fParam->GetCorrectedYValue(g, l, y, trace[t]);
          sum += value;
          loopDisplay->hBeamYSlice->SetBinContent(biny, bint, value);

          Double_t prev = loopDisplay->hBeamAccYSlice->GetBinContent(biny, bint);
          loopDisplay->hBeamAccYSlice->SetBinContent(biny, bint, prev + value);


          // charge and current traces:
           Double_t slicecharge=CperCount * value;
           Double_t slicecurrent=0;
           if(TperSlice) slicecurrent=slicecharge / TperSlice;


 #ifdef QFW_STORECURRENTS
           gridData->fYCurrent.push_back(slicecurrent);
 #endif


           if (((fParam->fGridMinWire_Y[gix] < 0) || (y >= fParam->fGridMinWire_Y[gix]))
              && ((fParam->fGridMaxWire_Y[gix] < 0) || (y < fParam->fGridMaxWire_Y[gix])))
          {
            loopDisplay->hBeamChargeYSlice->SetBinContent(biny, bint, slicecharge);
            Double_t cprev = loopDisplay->hBeamAccChargeYSlice->GetBinContent(biny, bint);
            loopDisplay->hBeamAccChargeYSlice->SetBinContent(biny, bint, cprev + slicecharge);
            loopDisplay->hBeamCurrentYSlice->SetBinContent(biny, bint, slicecurrent);
            Int_t numsamples = 1
                + (Int_t) ((loopDisplay->hBeamAccChargeYSlice->GetEntries() - 1)
                    / loopDisplay->hBeamAccChargeYSlice->GetNbinsX() / loopDisplay->hBeamAccChargeYSlice->GetNbinsY());    // divide number of entries by bins to get number of charge measurements per segment
            Double_t chargesum = loopDisplay->hBeamAccChargeYSlice->GetBinContent(biny, bint);
            Double_t currentaverage = chargesum / numsamples / TperSlice;
            loopDisplay->hBeamAveCurrentYSlice->SetBinContent(biny, bint, currentaverage);
          }


        }    // trace t
        loopDisplay->hBeamLoopY->Fill(y, sum);
        loopDisplay->hBeamAccLoopY->Fill(y, sum);
        gridDisplay->hBeamY->Fill(y, sum);    // we need Fill instead SetBinContent to evaluate statistics
        gridDisplay->hBeamAccY->Fill(y, sum);

        // here position calibrated histograms:
        if (gix >= 0)
        {
          Double_t ypos = fParam->fGridPosition_Y[gix][y];
          loopDisplay->hPosLoopY->Fill(ypos, sum);
          loopDisplay->hPosAccLoopY->Fill(ypos, sum);
          gridDisplay->hPosY->Fill(ypos, sum);    // we need Fill instead SetBinContent to evaluate statistics
          gridDisplay->hPosAccY->Fill(ypos, sum);

          Double_t charge = CperCount * sum;
          Double_t current = 0;
          if (TperLoop)
            current = charge / TperLoop;    // unit A

          // NOTE: we should not fill underflow/overflow bins for charge,
          // otherwise we are not able to calculate average current correctly!
          // (need to know number charge samples in chargesum histogram)
          // => we check for wire range here!
          if (((fParam->fGridMinWire_Y[gix] < 0) || (y >= fParam->fGridMinWire_Y[gix]))
              && ((fParam->fGridMaxWire_Y[gix] < 0) || (y < fParam->fGridMaxWire_Y[gix])))
          {

            loopDisplay->hPosQLoopY->Fill(ypos, charge);
            loopDisplay->hPosQAccLoopY->Fill(ypos, charge);
            gridDisplay->hPosQ_Y->Fill(ypos, charge);
            gridDisplay->hPosQAcc_Y->Fill(ypos, charge);
            loopDisplay->hPosILoopY->Fill(ypos, current);

            // average current is bin content of accumulated charge by number of samples by loop time
            Int_t numsamples = 1
                + (Int_t) ((loopDisplay->hPosQAccLoopY->GetEntries()-1) / loopDisplay->hPosQAccLoopY->GetNbinsX());    // divide number of entries by bins to get number of charge measurements per wire
            Int_t yposbin = loopDisplay->hPosQAccLoopY->FindBin(ypos);
//            printf("YYYYYYYY I- grid %d loop%d ypos:%f yposbin:%d, entries:%f bins:%d numsamples=%d \n", g, l, ypos, yposbin,
//                loopDisplay->hPosQAccLoopY->GetEntries(), loopDisplay->hPosQAccLoopY->GetNbinsX(), numsamples);

            Double_t chargesum = loopDisplay->hPosQAccLoopY->GetBinContent(yposbin);
            Double_t currentaverage = chargesum / numsamples / TperLoop;
//            printf("----- chargesum: %e C currentaverage:%e A current:%e \n", chargesum, currentaverage, current);
            loopDisplay->hPosIAveLoopY->Fill(ypos, currentaverage);

          }    // wire range

        }    //gix

      }    // y wires

    }    // loops
    
    // second loop loop to get mean values of traces:
    for (int l = 0; l < gridDisplay->GetNumLoops(); ++l)
    {
      TQFWGridLoopDisplay* loopDisplay = gridDisplay->GetLoopDisplay(l);
      // evaluate here mean value and sigma of profile counts

      Double_t MeanCountsX = 0, RMSCountsX = 0,  MeanCountsY = 0, RMSCountsY = 0 ;
      Double_t MeanCountsWireX[PEXOR_QFW_WIRES],  RMSCountsWireX[PEXOR_QFW_WIRES],  MeanCountsWireY[PEXOR_QFW_WIRES],  RMSCountsWireY[PEXOR_QFW_WIRES];
      //first x direction:
      Int_t cmax = loopDisplay->cBeamXSliceCond->GetCMax(loopDisplay->hBeamXSlice);
      if(cmax>0)
      {
      TH1I haux("temp", "temp", 2*cmax, -cmax, cmax);    // auxiliary histogram to calculate mean and rms of counts
      TH1I hauxWire("tempwire", "tempwire", 2*cmax, -cmax, cmax); // per wire calculation of 
      for (int wire = 0; wire < gridData->GetNumXWires(); ++wire)
      {
        MeanCountsWireY[wire] = 0;
        RMSCountsWireY[wire]=0;

	hauxWire.Reset("");
        for (int time = 0; time < loopDisplay->GetTimeSlices(); ++time)
        {
          if (loopDisplay->cBeamXSliceCond->Test(wire, time))
          {
            haux.Fill(loopDisplay->hBeamXSlice->GetBinContent(wire + 1, time + 1));
            hauxWire.Fill(loopDisplay->hBeamXSlice->GetBinContent(wire + 1, time + 1));
          }
        }
        MeanCountsWireX[wire] = hauxWire.GetMean();
        RMSCountsWireX[wire] =  hauxWire.GetRMS();
      }

      MeanCountsX = haux.GetMean();
      RMSCountsX = haux.GetRMS();
      }
      else
      {
        // optionally show debug
        //printf("XXXXXXXX: cmax=%d <0, do not evaluate profile stats \n", cmax);

      }

      mtitle.Form("%s mean=%.2E sigma=%.2E", loopDisplay->hBeamXSlice->GetTitle(), MeanCountsX, RMSCountsX);
      loopDisplay->hBeamXSlice->SetTitle(mtitle.Data());
//
//      // y direction:

      Int_t cmay = loopDisplay->cBeamYSliceCond->GetCMax(loopDisplay->hBeamYSlice);
      if(cmay>0)
      {
      TH1I hauy("temp2", "temp2", 2*cmay, -cmay, cmay);    // auxiliary histogram to calculate mean and rms of counts
      TH1I hauyWire("temp2wire", "temp2wire", 2*cmay, -cmay, cmay); // per wire calculation of 
      for (int wire = 0; wire < gridData->GetNumYWires(); ++wire)
      {
        MeanCountsWireY[wire] = 0;
        RMSCountsWireY[wire]=0;
	hauyWire.Reset("");
        for (int time = 0; time < loopDisplay->GetTimeSlices(); ++time)
        {
          if (loopDisplay->cBeamYSliceCond->Test(wire, time))
          {
            hauy.Fill(loopDisplay->hBeamYSlice->GetBinContent(wire + 1, time + 1));
            hauyWire.Fill(loopDisplay->hBeamYSlice->GetBinContent(wire + 1, time + 1));
          }
        }

        MeanCountsWireY[wire] = hauyWire.GetMean();
        RMSCountsWireY[wire] =  hauyWire.GetRMS();
      }
        MeanCountsY = hauy.GetMean();
        RMSCountsY =  hauy.GetRMS();
      }
      else
      {
                // optionally
                //printf("YYYYYYY: cmay=%d <=0, do not evaluate profile stats \n", cmay);
      }


      mtitle.Form("%s mean=%.2E sigma=%.2E", loopDisplay->hBeamYSlice->GetTitle(), MeanCountsY, RMSCountsY);
      loopDisplay->hBeamYSlice->SetTitle(mtitle.Data());

      loopDisplay->hBeamMeanCountsX->Fill(MeanCountsX);
      loopDisplay->hBeamMeanCountsY->Fill(MeanCountsY);
      loopDisplay->hBeamRMSCountsX->Fill(RMSCountsX);
      loopDisplay->hBeamRMSCountsY->Fill(RMSCountsY);

      for(int w=0; w< PEXOR_QFW_WIRES; ++w)
         {
            loopDisplay->hBeamMeanCountsGridX[w]->Fill(MeanCountsWireX[w]);
            loopDisplay->hBeamMeanCountsGridY[w]->Fill(MeanCountsWireY[w]);
            loopDisplay->hBeamRMSCountsGridX[w]->Fill(RMSCountsWireX[w]);
            loopDisplay->hBeamRMSCountsGridY[w]->Fill(RMSCountsWireY[w]);
         }






    }    // loops
    
// put here mean value calculations and profiles:

    // TODO: separate this profiles regarding different loops that can have different timeslices and measuerement ranges!

    gridData->fBeamMeanX = gridDisplay->hBeamX->GetMean();
    gridData->fBeamMeanY = gridDisplay->hBeamY->GetMean();
    gridData->fBeamRMSX = gridDisplay->hBeamX->GetRMS();
    gridData->fBeamRMSY = gridDisplay->hBeamY->GetRMS();
// todo: do we really need this values in output event?

    // JAMDEBUG
//    printf("Xmean:%e YMean:%e XRMS:%e YRMS:%e\n",
//          gridData->fBeamMeanX, gridData->fBeamMeanY,
//          gridData->fBeamRMSX, gridData->fBeamRMSY);
//    //ENDDEBUG
    gridDisplay->hBeamMeanXY->Fill(gridData->fBeamMeanX, gridData->fBeamMeanY);
    gridDisplay->hBeamRMSX->Fill(gridData->fBeamRMSX);
    gridDisplay->hBeamRMSY->Fill(gridData->fBeamRMSY);

    Double_t posMeanX = gridDisplay->hPosX->GetMean();
    Double_t posMeanY = gridDisplay->hPosY->GetMean();
    Double_t posRMSX = gridDisplay->hPosX->GetRMS();
    Double_t posRMSY = gridDisplay->hPosY->GetRMS();

    gridDisplay->hPosMeanXY->Fill(posMeanX, posMeanY);
    gridDisplay->hPosRMSX->Fill(posRMSX);
    gridDisplay->hPosRMSY->Fill(posRMSY);

// TODO: move above plots to each loop display.
// maybe we keep overall plots here only if we account it with the current calibration!

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
      Double_t chargesum = 0;
      for (int x = 0; x < cupData->GetNumSegments(); ++x)
      {
        segmentcharge[x] = 0;
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
        Double_t sum = 0;
        Double_t CperCount = loopData->GetCoulombPerCount();    // unit C
        Double_t TperSlice = 1.0e-6 * loopData->GetMicroSecsPerTimeSlice();
        Double_t TperLoop = TperSlice * loopData->fQfwLoopSize;    // unit s


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
          // charge and current traces:
          Double_t slicecharge=CperCount * value;
          Double_t slicecurrent=0;
          if(TperSlice) slicecurrent=slicecharge / TperSlice;
          loopDisplay->hCupChargeSlice->SetBinContent(1 + x, 1 + t, slicecharge);
          Double_t cprev = loopDisplay->hCupAccChargeSlice->GetBinContent(1 + x, 1 + t);
          loopDisplay->hCupAccChargeSlice->SetBinContent(1 + x, 1 + t, cprev + slicecharge);


          loopDisplay->hCupCurrentSlice->SetBinContent(1 + x, 1 + t, slicecurrent);
          Int_t numsamples = 1
                              + (Int_t) (( loopDisplay->hCupAccChargeSlice->GetEntries() -1) / loopDisplay->hCupAccChargeSlice->GetNbinsX()/ loopDisplay->hCupAccChargeSlice->GetNbinsY());    // divide number of entries by bins to get number of charge measurements per segment
          Double_t chargesum = loopDisplay->hCupAccChargeSlice->GetBinContent(1+x, 1+t);
          Double_t currentaverage = chargesum / numsamples / TperSlice;
          loopDisplay->hCupAveCurrentSlice->SetBinContent(1 + x, 1 + t, currentaverage);





#ifdef QFW_STORECURRENTS
          cupData->fCurrent.push_back(slicecurrent);
#endif
        }    // trace t

        cupDisplay->hCupScaler->AddBinContent(1 + x, sum);
        cupDisplay->hCupAccScaler->AddBinContent(1 + x, sum);

        Double_t charge = CperCount * sum;
        Double_t current = 0;
        if (TperLoop)
                    current = charge / TperLoop;    // unit A


        segmentcharge[x] = charge;
        chargesum += charge;

        cupDisplay->hCupScaler->AddBinContent(1 + x, sum); // TODO better use Fill here?
        cupDisplay->hCupAccScaler->AddBinContent(1 + x, sum);


        loopDisplay->hCupLoopScaler->Fill(x,sum);
        loopDisplay->hCupAccLoopScaler->Fill(x,sum);
        loopDisplay->hCupLoopCharge->Fill(x,charge);
        loopDisplay->hCupAccLoopCharge->Fill(x,charge);
        loopDisplay->hCupLoopCurrent->Fill(x,current);

        // average current is accum. charge by number of samples by measurement time:

        Int_t numsamples = 1
                     + (Int_t) (( loopDisplay->hCupAccLoopCharge->GetEntries() -1) / loopDisplay->hCupAccLoopCharge->GetNbinsX());    // divide number of entries by bins to get number of charge measurements per wire

        Double_t chargesum = loopDisplay->hCupAccLoopCharge->GetBinContent(x+1);
        Double_t currentaverage = chargesum / numsamples / TperLoop;
        loopDisplay->hCupAveLoopCurrent->Fill(x,currentaverage);

      }    // segments

      // here ratio histograms between segments:
      for (unsigned seg = 0; seg < loopDisplay->hSegmentRatio.size(); ++seg)
      {
        if (seg >= (unsigned) cupData->GetNumSegments())
        {
          TGo4Log::Error("Configuration error: Display segments index %d exceeds number of data segments %d ", seg,
              cupData->GetNumSegments());
          continue;
        }
        Double_t value = 0;
        if (chargesum)
          value = segmentcharge[seg] / chargesum;
        loopDisplay->hSegmentRatio[seg]->Fill(value);
        loopDisplay->hAccSegmentRatio[seg]->Fill(value);
      }

    }    // loops

  }    // cups
  // end cup display

  fOutput->SetValid(kTRUE);    // to store

  if (fParam->fSlowMotionStart > 0)
    if (QFWRawEvent->fSequenceNumber > (Int_t) fParam->fSlowMotionStart)
      GO4_STOP_ANALYSIS_MESSAGE(
          "Stopped for slow motion mode at event of sequence number %d", QFWRawEvent->fSequenceNumber);

  return kTRUE;
}

