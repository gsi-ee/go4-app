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
    for (int wire = 0; wire < PEXOR_APFEL_WIRES; ++wire)
       {

         // high gain grid (index 0):
         fGridBoardID_X[0][wire] = 10; // x direction at febex0 -> board id 10
         fGridBoardID_Y[0][wire] = 11; // y direction at febex1 -> board id 11
         // high gain grid (index 1):
         fGridBoardID_X[1][wire] = 10; // x direction at febex0 -> board id 10
         fGridBoardID_Y[1][wire] = 11;// y direction at febex1 -> board id 11
       }

    // mapping of wires to frontend channels:
    int grid=0; // first high gain
    fGridChannel_X[grid][9] = 0;
    fGridChannel_X[grid][10] = 2;
    fGridChannel_X[grid][12] = 10;
    fGridChannel_X[grid][13] = 4;
    fGridChannel_X[grid][14] = 6;
    fGridChannel_X[grid][15] = 14;

    fGridChannel_Y[grid][1] = 4;
    fGridChannel_Y[grid][3] = 14;
    fGridChannel_Y[grid][4] = 12;
    fGridChannel_Y[grid][5] = 10;
    fGridChannel_Y[grid][6] = 8;
    fGridChannel_Y[grid][7] = 0;
    fGridChannel_Y[grid][8] = 2;

    grid=1; // secondly low gain:
    fGridChannel_X[grid][9] = 1;

    fGridChannel_X[grid][10] = 3;
    fGridChannel_X[grid][12] = 11;
    fGridChannel_X[grid][13] = 5;
    fGridChannel_X[grid][14] = 7;
    fGridChannel_X[grid][15] = 15;

    fGridChannel_Y[grid][1] = 5;
    fGridChannel_Y[grid][3] = 15;
    fGridChannel_Y[grid][4] = 13;
    fGridChannel_Y[grid][5] = 11;
    fGridChannel_Y[grid][6] = 9;
    fGridChannel_Y[grid][7] = 1;
    fGridChannel_Y[grid][8] = 3;



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
