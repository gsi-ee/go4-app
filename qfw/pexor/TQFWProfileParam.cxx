// $Id: TQFWProfileParam.cxx 557 2010-01-27 15:11:43Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TQFWProfileParam.h"
#include "Riostream.h"

TQFWProfileParam::TQFWProfileParam() :
    TGo4Parameter()
{

}

TQFWProfileParam::TQFWProfileParam(const char* name) :
    TGo4Parameter(name), fSlowMotionStart(0)
{
  InitProfileMapping();
  ResetOffsetMeasurement();
  fMeasureBackground = kFALSE;
  fCorrectBackground = kTRUE;
}

void TQFWProfileParam::InitProfileMapping()
{
  // init to non valid here:
  fNumGrids = 0;
  for (int grid = 0; grid < PEXOR_QFW_GRIDS; ++grid)
  {
    fGridDeviceID[grid] = -1;
    for (int wire = 0; wire < PEXOR_QFW_WIRES; ++wire)
    {

      fGridBoardID_X[grid][wire] = -1;
      fGridBoardID_Y[grid][wire] = -1;
      fGridChannel_X[grid][wire] = -1;
      fGridChannel_Y[grid][wire] = -1;

    }
  }

  fNumCups = 0;
  for (int cup = 0; cup < PEXOR_QFW_CUPS; ++cup)
  {
    fCupDeviceID[cup] = -1;
    for (int seg = 0; seg < PEXOR_QFW_CUPSEGMENTS; ++seg)
    {

      fCupBoardID[cup][seg] = -1;
      fCupChannel[cup][seg] = -1;
    }
  }

// here manually type in the actual mapping. this could be reset by setup.C macro later:

  fNumGrids = 1;
  // [grid index] = unique grid id
  //fGridDeviceID[0]=42;
  
  // grid index 0 is our segmented xy grid
  // we assume 1:1 mapping of qfwboard channels to grid wires
  int grid = 0;
  fGridDeviceID[grid] = 42;
  for (int wire = 0; wire < PEXOR_QFW_WIRES; ++wire)
  {

//    fGridBoardID_X[grid][wire] = 10;
//    fGridBoardID_Y[grid][wire] = 11;
//    fGridChannel_X[grid][wire] = wire;
//    fGridChannel_Y[grid][wire] = wire;
//

    // this is setup at x2:
    fGridBoardID_X[grid][wire] = 10;
       fGridBoardID_Y[grid][wire] = 10;
   //    fGridChannel_X[grid][wire] = wire;
   //    fGridChannel_Y[grid][wire] = 31-wire;
       fGridChannel_X[grid][wire] = wire+16;
       fGridChannel_Y[grid][wire] = wire;
// end setup x2


  }
  // only 31 wire grid at HIT beamtime, disable last channel:
  fGridChannel_X[grid][31] = -1;
  fGridChannel_Y[grid][31] = -1;
  
  // grid index 1 is mapped cup scalers, x direction only
  //  grid = 1;
//  fGridDeviceID[grid] = 55;
//  for (int wire = 0; wire < PEXOR_QFWCHANS; ++wire)
//  {
//
//    fGridBoardID_X[grid][wire] = 12;
//    fGridBoardID_Y[grid][wire] = 12;
//    fGridChannel_X[grid][wire] = wire;
//    fGridChannel_Y[grid][wire] = -1;
//
//  }
//

// use "faraday cup" object to handle segmented capacitor plates
  fNumCups = 1;
   // [grid index] = unique grid id
//  fCupDeviceID[0]=66;
//  fCupBoardID[0][0] = 12;
//  fCupChannel[0][0] = 0;
//  fCupBoardID[0][1] = 12;
//  fCupChannel[0][1] = 1;


  fCupDeviceID[0]=66;
  fCupBoardID[0][0] = 11;
  fCupChannel[0][0] = 0;
  fCupBoardID[0][1] = 11;
  fCupChannel[0][1] = 1;

  
  //         [grid index][wire] = board  ;  [grid index][wire] = channel
//   fGridBoardID_X[0][0]=1; fGridChannel_X[0][0]=0;
//   fGridBoardID_X[0][1]=1; fGridChannel_X[0][1]=1;
//   fGridBoardID_X[0][2]=1; fGridChannel_X[0][2]=2;
//   fGridBoardID_X[0][3]=1; fGridChannel_X[0][3]=3;
//   fGridBoardID_X[0][4]=1; fGridChannel_X[0][4]=4;

}

Bool_t TQFWProfileParam::SetEventConfig()
{
  TQFWProfileEvent::fParameter = this;
  return kTRUE;
}

Bool_t TQFWProfileParam::AddXOffsetMeasurement(Int_t grid, Int_t loop, Int_t wire, UInt_t value)
{
  if (!CheckGridBoundaries(grid, loop, wire))
  {
    cout << "**** TQFWProfileParam::AddXOffsetMeasurement has illegal indices - (" << grid << "," << loop << "," << wire
        << ")" << endl;
    return kFALSE;
  }
  fMeasurementCountsX[grid][loop][wire]++;
  fQFWOffsetXSums[grid][loop][wire] += value;
  if (fMeasurementCountsX[grid][loop][wire])
    fQFWOffsetsX[grid][loop][wire] = (Double_t) fQFWOffsetXSums[grid][loop][wire]
        / (Double_t) fMeasurementCountsX[grid][loop][wire];

  return kTRUE;
}

Bool_t TQFWProfileParam::AddYOffsetMeasurement(Int_t grid, Int_t loop, Int_t wire, UInt_t value)
{
  if (!CheckGridBoundaries(grid, loop, wire))
  {
    cout << "**** TQFWProfileParam::AddYOffsetMeasurement has illegal indices - (" << grid << "," << loop << "," << wire
        << ")" << endl;
    return kFALSE;
  }
  fMeasurementCountsY[grid][loop][wire]++;
  fQFWOffsetYSums[grid][loop][wire] += value;
  if (fMeasurementCountsY[grid][loop][wire])
    fQFWOffsetsY[grid][loop][wire] = (Double_t) fQFWOffsetYSums[grid][loop][wire]
        / (Double_t) fMeasurementCountsY[grid][loop][wire];

  return kTRUE;
}


Bool_t TQFWProfileParam::AddCupOffsetMeasurement(Int_t cup, Int_t loop, Int_t seg, UInt_t value)
{
  if (!CheckCupBoundaries(cup, loop, seg))
  {
    cout << "**** TQFWProfileParam::AddCupOffsetMeasurement has illegal indices - (" << cup << "," << loop << "," << seg
        << ")" << endl;
    return kFALSE;
  }
  fMeasurementCountsCup[cup][loop][seg]++;
  fQFWOffsetCupSums[cup][loop][seg] += value;
  if (fMeasurementCountsCup[cup][loop][seg])
    fQFWOffsetsCup[cup][loop][seg] = (Double_t) fQFWOffsetCupSums[cup][loop][seg]
        / (Double_t) fMeasurementCountsCup[cup][loop][seg];

  return kTRUE;
}


void TQFWProfileParam::ResetOffsetMeasurement()
{
  for (int grid = 0; grid < PEXOR_QFW_GRIDS; ++grid)
  {
    for (int loop = 0; loop < PEXOR_QFWLOOPS; ++loop)
    {
      for (int wire = 0; wire < PEXOR_QFW_WIRES; ++wire)
      {
        fMeasurementCountsX[grid][loop][wire] = 0;
        fQFWOffsetXSums[grid][loop][wire] = 0;
        fQFWOffsetsX[grid][loop][wire] = 0;
        fMeasurementCountsY[grid][loop][wire] = 0;
        fQFWOffsetYSums[grid][loop][wire] = 0;
        fQFWOffsetsY[grid][loop][wire] = 0;

      }
    }
  }

  for (int cup = 0; cup < PEXOR_QFW_CUPS; ++cup)
  {
    for (int loop = 0; loop < PEXOR_QFWLOOPS; ++loop)
    {
      for (int seg = 0; seg < PEXOR_QFW_CUPSEGMENTS; ++seg)
      {
        fQFWOffsetsCup[cup][loop][seg] = 0;
        fQFWOffsetCupSums[cup][loop][seg] = 0;
        fMeasurementCountsCup[cup][loop][seg] = 0;    // counts offset values (grid,wire,loop)
      }
    }
  }
}

Double_t TQFWProfileParam::GetCorrectedXValue(Int_t grid, Int_t loop, Int_t wire, UInt_t count)
{
  if (!fCorrectBackground)
    return count;
  if (fMeasureBackground)
    return count;    // no correction evaluation during measurement
  if (!CheckGridBoundaries(grid, loop, wire))
  {
    cout << "**** TQFWProfileParam::GetCorrectedXValue has illegal indices - (" << grid << "," << loop << "," << wire
        << ")" << endl;
    return count;
  }
  return ((Double_t) count - fQFWOffsetsX[grid][loop][wire]);

}

Double_t TQFWProfileParam::GetCorrectedYValue(Int_t grid, Int_t loop, Int_t wire, UInt_t count)
{
  if (!fCorrectBackground)
    return count;
  if (fMeasureBackground)
    return count;    // no correction evaluation during measurement
  if (!CheckGridBoundaries(grid, loop, wire))
  {
    cout << "**** TQFWProfileParam::GetCorrectedYValue has illegal indices - (" << grid << "," << loop << "," << wire
        << ")" << endl;
    return count;
  }
  return ((Double_t) count - fQFWOffsetsY[grid][loop][wire]);
}

Double_t TQFWProfileParam::GetCorrectedCupValue(Int_t cup, Int_t loop, Int_t segment, UInt_t count)
{
  if (!fCorrectBackground)
    return count;
  if (fMeasureBackground)
    return count;    // no correction evaluation during measurement
  if (!CheckCupBoundaries(cup, loop, segment))
  {
    cout << "**** TQFWProfileParam::GetCorrectedCupValue has illegal indices - (" << cup << "," << loop << ","
        << segment << ")" << endl;
    return count;
  }
  return ((Double_t) count - fQFWOffsetsCup[cup][loop][segment]);
}

Bool_t TQFWProfileParam::CheckGridBoundaries(Int_t grid, Int_t loop, Int_t wire)
{
  if ((grid < 0) || (loop < 0) || (wire < 0))
    return kFALSE;
  if ((grid < PEXOR_QFW_GRIDS) && (loop < PEXOR_QFWLOOPS) && (wire < PEXOR_QFW_WIRES))
    return kTRUE;
  return kFALSE;
}

Bool_t TQFWProfileParam::CheckCupBoundaries(Int_t cup, Int_t loop, Int_t segment)
{
  if ((cup < 0) || (loop < 0) || (segment < 0))
    return kFALSE;
  if ((cup < PEXOR_QFW_CUPS) && (loop < PEXOR_QFWLOOPS) && (segment < PEXOR_QFW_CUPSEGMENTS))
    return kTRUE;
  return kFALSE;
}

Bool_t TQFWProfileParam::UpdateFrom(TGo4Parameter *pp)
{
  TQFWProfileParam* from = dynamic_cast<TQFWProfileParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  // if(!TGo4Parameter::UpdateFrom(pp)) return kFALSE; // will automatically copy 2d arrays
  // JAM NOTE: we need to implement copy ourselves, since go4 default cannot handle 3d arrays

  cout << "**** TQFWProfileParam::UpdateFrom ... " << endl;

  if (!fMeasureBackground && from->fMeasureBackground)
  {
    cout << "TQFWRawParam::UpdateFrom is resetting background measurement!!!" << endl;
    //ResetOffsetMeasurement();
  }
  fMeasureBackground = from->fMeasureBackground;
  fCorrectBackground = from->fCorrectBackground;
  fSlowMotionStart = from->fSlowMotionStart;

  // configuration objects are here
  fNumGrids = from->fNumGrids;
  for (int grid = 0; grid < PEXOR_QFW_GRIDS; ++grid)
  {
    fGridDeviceID[grid] = from->fGridDeviceID[grid];
    for (int wire = 0; wire < PEXOR_QFW_WIRES; ++wire)
    {

      fGridBoardID_X[grid][wire] = from->fGridBoardID_X[grid][wire];
      fGridBoardID_Y[grid][wire] = from->fGridBoardID_Y[grid][wire];
      fGridChannel_X[grid][wire] = from->fGridChannel_X[grid][wire];
      fGridChannel_Y[grid][wire] = from->fGridChannel_Y[grid][wire];

    }
  }

  fNumCups = from->fNumCups;
  for (int cup = 0; cup < PEXOR_QFW_CUPS; ++cup)
  {
    fCupDeviceID[cup] = from->fCupDeviceID[cup];
    for (int seg = 0; seg < PEXOR_QFW_CUPSEGMENTS; ++seg)
    {

      fCupBoardID[cup][seg] = from->fCupBoardID[cup][seg];
      fCupChannel[cup][seg] = from->fCupChannel[cup][seg];
    }
  }
// do not copy background correction arrays!

  return SetEventConfig();

}
