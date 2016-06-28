//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#include "TFeb3BasicParam.h"

#include "Riostream.h"

//***********************************************************
TFeb3BasicParam::TFeb3BasicParam() : TGo4Parameter("Parameter")
{
  InitMapping();
}
//***********************************************************
TFeb3BasicParam::TFeb3BasicParam(const char* name) : TGo4Parameter(name)
{
  InitMapping();
}
//***********************************************************
TFeb3BasicParam::~TFeb3BasicParam()
{
}
//***********************************************************

void TFeb3BasicParam::InitMapping()
{
  fDoGridMapping=kTRUE; // optionally disable this for debugging
  fGridTraceDownscale=PEXOR_TRACE_BINSCALE;
  fSlowMotionStart=0;
  fDoPeakFit=kFALSE;
  fFitMaxPeaks=5;
  fFitRootPF_Par=1;

   // init to non valid here:
   for (int sfp = 0; sfp < PEXOR_MAXSFP; ++sfp)
         {
         for (int dev = 0; dev < PEXOR_MAXDEVS; ++dev)
               {
                  fBoardID[sfp][dev]=-1;
               }
         }

    // here is actual mapping of frontend boards to sfp chains:
    fBoardID[0][0]=10; // febex0 -> 10
    fBoardID[0][1]=11; // febex1-> 11


    fNumGrids = 0;
      for (int grid = 0; grid < PEXOR_APFEL_GRIDS; ++grid)
      {
        fGridDeviceID[grid] = -1;
        for (int wire = 0; wire < PEXOR_APFEL_WIRES; ++wire)
        {

          fGridBoardID_X[grid][wire] = -1;
          fGridBoardID_Y[grid][wire] = -1;
          fGridChannel_X[grid][wire] = -1;
          fGridChannel_Y[grid][wire] = -1;

        }
      }


    // here constantly initialize the actual grid mapping. this could be reset by setup.C macro later
    // without setup macro, parameter editor can change setup interactively

    fNumGrids = 2;
    fGridDeviceID[0]=42; // high gain arbitrary id number
    fGridDeviceID[1]=43; // low gain arbitrary id number

    // mapping of wires and grids to frontend boards:
    for (int wire = 0; wire < 8; ++wire)
       {

         // high gain grid (index 0):
         fGridBoardID_X[0][wire] = 10; // x direction at febex0 -> board id 10
         fGridBoardID_X[0][wire+8] = 11; // y direction at febex1 -> board id 11
         // high gain grid (index 1):
         fGridBoardID_X[1][wire] = 10; // x direction at febex0 -> board id 10
         fGridBoardID_X[1][wire+8] = 11;// y direction at febex1 -> board id 11
       }

    // mapping of wires to frontend channels:
    int grid=0; // first high gain

    fGridChannel_X[grid][0] = 12;  //0
    fGridChannel_X[grid][1] = 14; //0
    fGridChannel_X[grid][2] = 8;//0
    fGridChannel_X[grid][3] = 10; //0
    fGridChannel_X[grid][4] = 6; //0
    fGridChannel_X[grid][5] = 4;//0
    fGridChannel_X[grid][6] = 2;  //0
    fGridChannel_X[grid][7] = 0; //0
    fGridChannel_X[grid][8] = 12;//0
    fGridChannel_X[grid][9] = 14; //0
    fGridChannel_X[grid][10] = 8; //0
    fGridChannel_X[grid][11] = 10;//0
    fGridChannel_X[grid][12] = 6;  //0
    fGridChannel_X[grid][13] = 4; //0
    fGridChannel_X[grid][14] = 2;//0
    fGridChannel_X[grid][15] = 0; //0

    grid=1; // secondly low gain:
    fGridChannel_X[grid][0] = 13;  //0
    fGridChannel_X[grid][1] = 15; //0
    fGridChannel_X[grid][2] = 9;//0
    fGridChannel_X[grid][3] = 11; //0
    fGridChannel_X[grid][4] = 7; //0
    fGridChannel_X[grid][5] = 5;//0
    fGridChannel_X[grid][6] = 3;  //0
    fGridChannel_X[grid][7] = 1; //0
    fGridChannel_X[grid][8] = 13;//0
    fGridChannel_X[grid][9] = 15; //0
    fGridChannel_X[grid][10] = 9; //0
    fGridChannel_X[grid][11] = 11;//0
    fGridChannel_X[grid][12] = 7;  //0
    fGridChannel_X[grid][13] = 5; //0
    fGridChannel_X[grid][14] = 3;//0
    fGridChannel_X[grid][15] = 1; //0

}


void TFeb3BasicParam::FindBoardIndices(Int_t boardid, Int_t &ch, Int_t &slave)
{
  for(Int_t sfp=0; sfp<PEXOR_MAXSFP; ++sfp)
  {
    for(Int_t dev=0; dev<PEXOR_MAXSFP; ++dev)
     {
        if (fBoardID[sfp][dev]==boardid) {
           ch=sfp;
           slave=dev;
           return;
        }
     }
  }
  ch=-1;
  slave=-1;
}





//----------------------------END OF GO4 SOURCE FILE ---------------------
