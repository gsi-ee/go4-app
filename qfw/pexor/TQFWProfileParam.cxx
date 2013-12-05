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

TQFWProfileParam::TQFWProfileParam() : TGo4Parameter()
{

}

TQFWProfileParam::TQFWProfileParam(const char* name) :
    TGo4Parameter(name)
{
  InitProfileMapping();
  ResetOffsetMeasurement();
  fMeasureBackground=kFALSE;
  fCorrectBackground=kTRUE;
}

void TQFWProfileParam::InitProfileMapping()
{
  // init to non valid here:
  fNumGrids=0;
  for (int grid = 0; grid < PEXOR_QFW_GRIDS; ++grid)
  {
    fGridDeviceID[grid]=-1;
    for (int wire = 0; wire < PEXOR_QFW_WIRES; ++wire)
    {


      fGridBoardID_X[grid][wire] = -1;
      fGridBoardID_Y[grid][wire] = -1;
      fGridChannel_X[grid][wire] = -1;
      fGridChannel_Y[grid][wire] = -1;

    }
  }

  fNumCups=0;
  for (int cup = 0; cup < PEXOR_QFW_CUPS; ++cup)
  {
    fCupDeviceID[cup]=-1;
    for (int seg = 0; seg < PEXOR_QFW_CUPSEGMENTS; ++seg)
    {

      fCupBoardID[cup][seg]=-1;
      fCupChannel[cup][seg]=-1;
    }
  }

// here manually type in the actual mapping. this could be reset by setup.C macro later:

  fNumGrids=2;
  // [grid index] = unique grid id
  //fGridDeviceID[0]=42;

    // grid index 0 is our segmented xy grid
    // we assume 1:1 mapping of qfwboard channels to grid wires
    int grid=0;
    fGridDeviceID[grid]=42;
    for (int wire = 0; wire < PEXOR_QFWCHANS; ++wire)
    {


      fGridBoardID_X[grid][wire] = 10;
      fGridBoardID_Y[grid][wire] = 11;
      fGridChannel_X[grid][wire] = wire;
      fGridChannel_Y[grid][wire] = wire;

    }
  
  
   // grid index 1 is mapped cup scalers, x direction only
   // todo: use several cup scaler objects instead (not read yet)
    grid=1;
    fGridDeviceID[grid]=55;
    for (int wire = 0; wire < PEXOR_QFWCHANS; ++wire)
    {


      fGridBoardID_X[grid][wire] = 12;
      fGridBoardID_Y[grid][wire] = 12;
      fGridChannel_X[grid][wire] = wire;
      fGridChannel_Y[grid][wire] = -1;

    }
  
  
  //         [grid index][wire] = board  ;  [grid index][wire] = channel
//   fGridBoardID_X[0][0]=1; fGridChannel_X[0][0]=0;
//   fGridBoardID_X[0][1]=1; fGridChannel_X[0][1]=1;
//   fGridBoardID_X[0][2]=1; fGridChannel_X[0][2]=2;
//   fGridBoardID_X[0][3]=1; fGridChannel_X[0][3]=3;
//   fGridBoardID_X[0][4]=1; fGridChannel_X[0][4]=4;
//   fGridBoardID_X[0][5]=1; fGridChannel_X[0][5]=5;
//   fGridBoardID_X[0][6]=1; fGridChannel_X[0][6]=6;
//   fGridBoardID_X[0][7]=1; fGridChannel_X[0][7]=7;
//   fGridBoardID_X[0][8]=1; fGridChannel_X[0][8]=8;
//      fGridBoardID_X[0][9]=1; fGridChannel_X[0][9]=9;
//      fGridBoardID_X[0][10]=1; fGridChannel_X[0][10]=10;
//      fGridBoardID_X[0][11]=1; fGridChannel_X[0][11]=11;
//      fGridBoardID_X[0][12]=1; fGridChannel_X[0][12]=12;
//       fGridBoardID_X[0][13]=1; fGridChannel_X[0][13]=13;
//       fGridBoardID_X[0][14]=1; fGridChannel_X[0][14]=14;
//       fGridBoardID_X[0][15]=1; fGridChannel_X[0][15]=15;
//       
//       
//       
//       
//       
// 
// 
//       fGridBoardID_Y[0][0]=1; fGridChannel_Y[0][0]=16;
//        fGridBoardID_Y[0][1]=1; fGridChannel_Y[0][1]=17;
//        fGridBoardID_Y[0][2]=1; fGridChannel_Y[0][2]=18;
//        fGridBoardID_Y[0][3]=1; fGridChannel_Y[0][3]=19;
//        fGridBoardID_Y[0][4]=1; fGridChannel_Y[0][4]=20;
//         fGridBoardID_Y[0][5]=1; fGridChannel_Y[0][5]=21;
//         fGridBoardID_Y[0][6]=1; fGridChannel_Y[0][6]=22;
//         fGridBoardID_Y[0][7]=1; fGridChannel_Y[0][7]=23;
//         fGridBoardID_Y[0][8]=1; fGridChannel_Y[0][8]=24;
//           fGridBoardID_Y[0][9]=1; fGridChannel_Y[0][9]=25;
//           fGridBoardID_Y[0][10]=1; fGridChannel_Y[0][10]=26;
//           fGridBoardID_Y[0][11]=1; fGridChannel_Y[0][11]=27;
//           fGridBoardID_Y[0][12]=1; fGridChannel_Y[0][12]=28;
//            fGridBoardID_Y[0][13]=1; fGridChannel_Y[0][13]=29;
//            fGridBoardID_Y[0][14]=1; fGridChannel_Y[0][14]=30;
//            fGridBoardID_Y[0][15]=1; fGridChannel_Y[0][15]=31;









//   fBoardID[0][0]=1; // test case: one qfw board at sfp0


}

Bool_t TQFWProfileParam::SetEventConfig()
{
  TQFWProfileEvent::fParameter=this;
  return kTRUE;
}


Bool_t  TQFWProfileParam::AddXOffsetMeasurement(Int_t grid, Int_t loop , Int_t wire, UInt_t value)
{
  if(!CheckGridBoundaries(grid,loop,wire))
        {
            cout << "**** TQFWProfileParam::GetCorrectedXValue has illegal indices - ("<<grid<<","<<loop<<","<<wire<<")" << endl;
            return kFALSE;
        }
  fMeasurementCountsX[grid][loop][wire]++;
  fQFWOffsetXSums[grid][loop][wire]+=value;
  fQFWOffsetsX[grid][loop][wire]=fQFWOffsetXSums[grid][loop][wire]/fMeasurementCountsX[grid][loop][wire];

return kTRUE;
}

Bool_t TQFWProfileParam::AddYOffsetMeasurement(Int_t grid, Int_t loop, Int_t wire, UInt_t value)
{
  if (!CheckGridBoundaries(grid, loop, wire))
  {
    cout << "**** TQFWProfileParam::GetCorrectedXValue has illegal indices - (" << grid << "," << loop << "," << wire
        << ")" << endl;
    return kFALSE;
  }
  fMeasurementCountsY[grid][loop][wire]++;
  fQFWOffsetYSums[grid][loop][wire] += value;
  fQFWOffsetsY[grid][loop][wire] = fQFWOffsetYSums[grid][loop][wire] / fMeasurementCountsY[grid][loop][wire];

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
}




Double_t TQFWProfileParam::GetCorrectedXValue(Int_t grid, Int_t loop, Int_t wire, UInt_t count)
{
  if(!fCorrectBackground) return count;
  if(fMeasureBackground) return count; // no correction evaluation during measurement
 if(!CheckGridBoundaries( grid,loop,wire))
        {
            cout << "**** TQFWProfileParam::GetCorrectedXValue has illegal indices - ("<<grid<<","<<loop<<","<<wire<<")" << endl;
            return count;
        }
 return (count - fQFWOffsetsX[grid][loop][wire]);


}

Double_t TQFWProfileParam::GetCorrectedYValue(Int_t grid, Int_t loop, Int_t wire, UInt_t count)
{
  if(!fCorrectBackground) return count;
   if(fMeasureBackground) return count; // no correction evaluation during measurement
  if(!CheckGridBoundaries( grid,loop,wire))
         {
             cout << "**** TQFWProfileParam::GetCorrectedYValue has illegal indices - ("<<grid<<","<<loop<<","<<wire<<")" << endl;
             return count;
         }
  return (count - fQFWOffsetsY[grid][loop][wire]);
}




 Bool_t TQFWProfileParam::CheckGridBoundaries(Int_t grid, Int_t loop, Int_t wire)
{
  if ((grid < 0) || (loop < 0) || (wire < 0))
    return kFALSE;
  if ((grid < PEXOR_QFW_GRIDS) && (loop < PEXOR_QFWLOOPS) && (wire < PEXOR_QFW_WIRES))
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

  if(!fMeasureBackground && from->fMeasureBackground)
  {
      cout << "TQFWRawParam::UpdateFrom is resetting background measurement!!!"<< endl;
      //ResetOffsetMeasurement();
  }
  fMeasureBackground=from->fMeasureBackground;
  fCorrectBackground=from->fCorrectBackground;


      // configuration objects are here
fNumGrids=from->fNumGrids;
      for (int grid = 0; grid < PEXOR_QFW_GRIDS; ++grid)
        {
          fGridDeviceID[grid]=from->fGridDeviceID[grid];
          for (int wire = 0; wire < PEXOR_QFW_WIRES; ++wire)
          {


            fGridBoardID_X[grid][wire] = from->fGridBoardID_X[grid][wire];
            fGridBoardID_Y[grid][wire] = from->fGridBoardID_Y[grid][wire];
            fGridChannel_X[grid][wire] = from->fGridChannel_X[grid][wire];
            fGridChannel_Y[grid][wire] = from->fGridChannel_Y[grid][wire];

          }
        }

        fNumCups=from->fNumCups;
        for (int cup = 0; cup < PEXOR_QFW_CUPS; ++cup)
        {
          fCupDeviceID[cup]=from->fCupDeviceID[cup];
          for (int seg = 0; seg < PEXOR_QFW_CUPSEGMENTS; ++seg)
          {

            fCupBoardID[cup][seg]=from->fCupBoardID[cup][seg];
            fCupChannel[cup][seg]=from->fCupChannel[cup][seg];
          }
        }
// do not copy background correction arrays!

        return SetEventConfig();

}
