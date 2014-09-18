// written by macro saveparam.C at Thu Mar 27 12:22:33 2014
void set_QFWProfileParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_QFWProfileParam can execute only in analysis" << std::endl;
   return;
#endif
   TQFWProfileParam* param1 = (TQFWProfileParam*) go4->GetParameter("QFWProfileParam","TQFWProfileParam");

   if (param1==0) {
      TGo4Log::Error("Could not find parameter QFWProfileParam of class TQFWProfileParam");
      return;
   }

   TGo4Log::Info("Set parameter QFWProfileParam as saved at Thu Mar 27 12:22:33 2014");

   //param1->fSlowMotionStart = 0;
   //param1->fMeasureBackground = kFALSE;
   //param1->fCorrectBackground = kFALSE;

// uncomment lines below to override stored defaults

// CHANGE SETUF OF CUPS HERE:
   param1->fNumCups = 1;

   param1->fCupDeviceID[0] = 61;

   param1->fCupBoardID[0][0] = 13;
   param1->fCupBoardID[0][1] = 13;

   param1->fCupChannel[0][0] = 0;
   param1->fCupChannel[0][1] = 1;

// CHANGE SETUF OF GRIDS HERE:

   param1->fNumGrids = 5;
   param1->fGridDeviceID[0] = 41;
   param1->fGridDeviceID[1] = 42;

   param1->fGridDeviceID[2] = 51;		// a 2 wire air plane detector (from Andreas)

   
   param1->fGridDeviceID[3] = 91;		// used for raw data dumping of sfp0 and sfp1
   param1->fGridDeviceID[4] = 92;		// used for raw data dumping of sfp2 and sfp3



// mapping grid/wire to frontends  [gridindex][wireindex]

int grid = 0;	// define grid variable


// Grid 41
// 2 x 31 wires SEM Grid
// located at beam dump (with HV)
// only X plane is connected,
// connected to sfp1 (0-30)
   
   grid = 0;
   for (int wire = 0; wire < 31; ++wire)
   {
      param1->fGridBoardID_X[grid][wire] = 11;
      param1->fGridChannel_X[grid][wire] = wire;
   }

   param1->fGridPosition_X[grid][0]  = -45.000000;
   param1->fGridPosition_X[grid][1]  = -40.500000;
   param1->fGridPosition_X[grid][2]  = -36.000000;
   param1->fGridPosition_X[grid][3]  = -31.500000;
   param1->fGridPosition_X[grid][4]  = -27.000000;
   param1->fGridPosition_X[grid][5]  = -22.500000;
   param1->fGridPosition_X[grid][6]  = -19.500000;
   param1->fGridPosition_X[grid][7]  = -16.500000;
   param1->fGridPosition_X[grid][8]  = -13.500000;
   param1->fGridPosition_X[grid][9]  = -10.500000;
   param1->fGridPosition_X[grid][10] = -7.500000;
   param1->fGridPosition_X[grid][11] = -6.000000;
   param1->fGridPosition_X[grid][12] = -4.500000;
   param1->fGridPosition_X[grid][13] = -3.000000;
   param1->fGridPosition_X[grid][14] = -1.500000;
   param1->fGridPosition_X[grid][15] = 0.000000;
   param1->fGridPosition_X[grid][16] = 1.500000;
   param1->fGridPosition_X[grid][17] = 3.000000;
   param1->fGridPosition_X[grid][18] = 4.500000;
   param1->fGridPosition_X[grid][19] = 6.000000;
   param1->fGridPosition_X[grid][20] = 7.500000;
   param1->fGridPosition_X[grid][21] = 10.500000;
   param1->fGridPosition_X[grid][22] = 13.500000;
   param1->fGridPosition_X[grid][23] = 16.500000;
   param1->fGridPosition_X[grid][24] = 19.500000;
   param1->fGridPosition_X[grid][25] = 22.500000;
   param1->fGridPosition_X[grid][26] = 27.000000;
   param1->fGridPosition_X[grid][27] = 31.500000;
   param1->fGridPosition_X[grid][28] = 36.000000;
   param1->fGridPosition_X[grid][29] = 40.500000;
   param1->fGridPosition_X[grid][30] = 45.000000;
//   param1->fGridPosition_X[grid][31] = 49.500000;
   
   param1->fGridMinWire_X[grid] = 0;
   param1->fGridMaxWire_X[grid] = 31;
   param1->fGridMinWire_Y[grid] = -1;
   param1->fGridMaxWire_Y[grid] = -1;


// Grid 42
// 2 x 31 wires SEM Grid
// located at HTP 
// X and Y plane are connected,
// connected to sfp0 (0-30) (X)  and sfp2 (0-30) (Y)
 
   grid = 1;
   for (int wire = 0; wire < 31; ++wire)
   {
      param1->fGridBoardID_X[grid][wire] = 10;
      param1->fGridChannel_X[grid][wire] = wire;

      param1->fGridBoardID_Y[grid][wire] = 12;
      param1->fGridChannel_Y[grid][wire] = wire;
   }

   param1->fGridPosition_X[grid][0]  = -45.000000;
   param1->fGridPosition_X[grid][1]  = -40.500000;
   param1->fGridPosition_X[grid][2]  = -36.000000;
   param1->fGridPosition_X[grid][3]  = -31.500000;
   param1->fGridPosition_X[grid][4]  = -27.000000;
   param1->fGridPosition_X[grid][5]  = -22.500000;
   param1->fGridPosition_X[grid][6]  = -19.500000;
   param1->fGridPosition_X[grid][7]  = -16.500000;
   param1->fGridPosition_X[grid][8]  = -13.500000;
   param1->fGridPosition_X[grid][9]  = -10.500000;
   param1->fGridPosition_X[grid][10] = -7.500000;
   param1->fGridPosition_X[grid][11] = -6.000000;
   param1->fGridPosition_X[grid][12] = -4.500000;
   param1->fGridPosition_X[grid][13] = -3.000000;
   param1->fGridPosition_X[grid][14] = -1.500000;
   param1->fGridPosition_X[grid][15] = 0.000000;
   param1->fGridPosition_X[grid][16] = 1.500000;
   param1->fGridPosition_X[grid][17] = 3.000000;
   param1->fGridPosition_X[grid][18] = 4.500000;
   param1->fGridPosition_X[grid][19] = 6.000000;
   param1->fGridPosition_X[grid][20] = 7.500000;
   param1->fGridPosition_X[grid][21] = 10.500000;
   param1->fGridPosition_X[grid][22] = 13.500000;
   param1->fGridPosition_X[grid][23] = 16.500000;
   param1->fGridPosition_X[grid][24] = 19.500000;
   param1->fGridPosition_X[grid][25] = 22.500000;
   param1->fGridPosition_X[grid][26] = 27.000000;
   param1->fGridPosition_X[grid][27] = 31.500000;
   param1->fGridPosition_X[grid][28] = 36.000000;
   param1->fGridPosition_X[grid][29] = 40.500000;
   param1->fGridPosition_X[grid][30] = 45.000000;
//   param1->fGridPosition_X[grid][31] = 49.500000;

   param1->fGridPosition_Y[grid][0]  = -45.000000;
   param1->fGridPosition_Y[grid][1]  = -40.500000;
   param1->fGridPosition_Y[grid][2]  = -36.000000;
   param1->fGridPosition_Y[grid][3]  = -31.500000;
   param1->fGridPosition_Y[grid][4]  = -27.000000;
   param1->fGridPosition_Y[grid][5]  = -22.500000;
   param1->fGridPosition_Y[grid][6]  = -19.500000;
   param1->fGridPosition_Y[grid][7]  = -16.500000;
   param1->fGridPosition_Y[grid][8]  = -13.500000;
   param1->fGridPosition_Y[grid][9]  = -10.500000;
   param1->fGridPosition_Y[grid][10] = -7.500000;
   param1->fGridPosition_Y[grid][11] = -6.000000;
   param1->fGridPosition_Y[grid][12] = -4.500000;
   param1->fGridPosition_Y[grid][13] = -3.000000;
   param1->fGridPosition_Y[grid][14] = -1.500000;
   param1->fGridPosition_Y[grid][15] = 0.000000;
   param1->fGridPosition_Y[grid][16] = 1.500000;
   param1->fGridPosition_Y[grid][17] = 3.000000;
   param1->fGridPosition_Y[grid][18] = 4.500000;
   param1->fGridPosition_Y[grid][19] = 6.000000;
   param1->fGridPosition_Y[grid][20] = 7.500000;
   param1->fGridPosition_Y[grid][21] = 10.500000;
   param1->fGridPosition_Y[grid][22] = 13.500000;
   param1->fGridPosition_Y[grid][23] = 16.500000;
   param1->fGridPosition_Y[grid][24] = 19.500000;
   param1->fGridPosition_Y[grid][25] = 22.500000;
   param1->fGridPosition_Y[grid][26] = 27.000000;
   param1->fGridPosition_Y[grid][27] = 31.500000;
   param1->fGridPosition_Y[grid][28] = 36.000000;
   param1->fGridPosition_Y[grid][29] = 40.500000;
   param1->fGridPosition_Y[grid][30] = 45.000000;
//   param1->fGridPosition_Y[grid][31] = 49.500000;
   
   param1->fGridMinWire_X[grid] = 0;
   param1->fGridMaxWire_X[grid] = 31;
   param1->fGridMinWire_Y[grid] = 0;
   param1->fGridMaxWire_Y[grid] = 31;


// Grid 51
// 2 wir eair plane detector of Andreas (from sfp3m, ch 0-1)
   
   grid = 2;
   for (int wire = 0; wire < 2; ++wire)
   {
      param1->fGridBoardID_X[grid][wire] = 13;
      param1->fGridChannel_X[grid][wire] = wire;

      param1->fGridPosition_X[grid][wire] = wire;
   }

   param1->fGridMinWire_X[grid] = 0;
   param1->fGridMaxWire_X[grid] = 2;
   param1->fGridMinWire_Y[grid] = -1;
   param1->fGridMaxWire_Y[grid] = -1;


   
// Grid 91
// dummy grid for raw data dumping of sfp0 and sfp1
   
   grid = 3;
   for (int wire = 0; wire < 32; ++wire)
   {
      param1->fGridBoardID_X[grid][wire] = 10;
      param1->fGridChannel_X[grid][wire] = wire;

      param1->fGridBoardID_Y[grid][wire] = 11;
      param1->fGridChannel_Y[grid][wire] = wire;

      param1->fGridPosition_X[grid][wire] = wire;
      param1->fGridPosition_Y[grid][wire] = wire;
   }

   param1->fGridMinWire_X[grid] = 0;
   param1->fGridMaxWire_X[grid] = 32;
   param1->fGridMinWire_Y[grid] = 0;
   param1->fGridMaxWire_Y[grid] = 32;


// Grid 92
// dummy grid for raw data dumping of sfp2 and sfp3
   
   grid = 4;
   for (int wire = 0; wire < 32; ++wire)
   {
      param1->fGridBoardID_X[grid][wire] = 12;
      param1->fGridChannel_X[grid][wire] = wire;

      param1->fGridBoardID_Y[grid][wire] = 13;
      param1->fGridChannel_Y[grid][wire] = wire;

      param1->fGridPosition_X[grid][wire] = wire;
      param1->fGridPosition_Y[grid][wire] = wire;
   }

   param1->fGridMinWire_X[grid] = 0;
   param1->fGridMaxWire_X[grid] = 32;
   param1->fGridMinWire_Y[grid] = 0;
   param1->fGridMaxWire_Y[grid] = 32;

}
