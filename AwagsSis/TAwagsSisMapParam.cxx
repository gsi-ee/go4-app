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
#include "TAwagsSisMapParam.h"
#include "TAwagsSisBasicEvent.h"

#include "Riostream.h"
using namespace std;

//***********************************************************
TAwagsSisMapParam::TAwagsSisMapParam() : TGo4Parameter("Parameter")
{

}
//***********************************************************
TAwagsSisMapParam::TAwagsSisMapParam(const char* name) : TGo4Parameter(name)
{
  fSlowMotion=kFALSE;
  fStopAtEachSpill=kFALSE;
  fMaxChamber=CSA_MAXCHAMBERS;
  for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
   {
     for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
     {
       fSFP [dev][wire]=-1;
       fSlave [dev][wire]=-1;
       fChannel[dev][wire]=-1;
     }
     fMaxWire[dev]=CSA_MAXCHAMBERS;
   }

  InitAwagsMapping();
  }
//***********************************************************
TAwagsSisMapParam::~TAwagsSisMapParam()
{
}
//***********************************************************


void TAwagsSisMapParam::InitAwagsMapping()
{
  // some fake mapping to see something JAM

   fMaxChamber=2; // total number of units/chambers/grids, resp.
  fMaxWire[0]=16; // number of wires/strips for unit of index
  fMaxWire[1]=16;

  for (Int_t wire = 0; wire < N_CHA; ++wire)
       {
         fSFP [0][wire]=0;
         fSlave [0][wire]=0;
         fChannel[0][wire]=wire;
         fSFP [1][wire]=0;
         fSlave [1][wire]=1;
         fChannel[1][wire]=wire;
       }



  // CSA mapping from Sven 10-Dec-2019:
//  fSFP[0][0] = 0; fSlave[0][0] = 3; fChannel[0][0] = 0;
//  fSFP[0][1] = 0; fSlave[0][1] = 2; fChannel[0][1] = 8;
//  fSFP[0][2] = 0; fSlave[0][2] = 0; fChannel[0][2] = 0;
//  fSFP[0][3] = 0; fSlave[0][3] = 1; fChannel[0][3] = 8;
//  fSFP[0][4] = 0; fSlave[0][4] = 3; fChannel[0][4] = 1;
//  fSFP[0][5] = 0; fSlave[0][5] = 2; fChannel[0][5] = 9;
//  fSFP[0][6] = 0; fSlave[0][6] = 0; fChannel[0][6] = 1;
//  fSFP[0][7] = 0; fSlave[0][7] = 1; fChannel[0][7] = 9;
//  fSFP[0][8] = 0; fSlave[0][8] = 3; fChannel[0][8] = 2;
//  fSFP[0][9] = 0; fSlave[0][9] = 2; fChannel[0][9] = 10;
//  fSFP[0][10] = 0; fSlave[0][10] = 0; fChannel[0][10] = 2;
//  fSFP[0][11] = 0; fSlave[0][11] = 1; fChannel[0][11] = 10;
//  fSFP[0][12] = 0; fSlave[0][12] = 3; fChannel[0][12] = 3;
//  fSFP[0][13] = 0; fSlave[0][13] = 2; fChannel[0][13] = 11;
//  fSFP[0][14] = 0; fSlave[0][14] = 0; fChannel[0][14] = 3;
//  fSFP[0][15] = 0; fSlave[0][15] = 1; fChannel[0][15] = 11;
//  fSFP[0][16] = 0; fSlave[0][16] = 3; fChannel[0][16] = 4;
//  fSFP[0][17] = 0; fSlave[0][17] = 2; fChannel[0][17] = 12;
//  fSFP[0][18] = 0; fSlave[0][18] = 0; fChannel[0][18] = 4;
//  fSFP[0][19] = 0; fSlave[0][19] = 1; fChannel[0][19] = 12;
//  fSFP[0][20] = 0; fSlave[0][20] = 3; fChannel[0][20] = 5;
//  fSFP[0][21] = 0; fSlave[0][21] = 2; fChannel[0][21] = 13;
//  fSFP[0][22] = 0; fSlave[0][22] = 0; fChannel[0][22] = 5;
//  fSFP[0][23] = 0; fSlave[0][23] = 1; fChannel[0][23] = 13;
//  fSFP[0][24] = 0; fSlave[0][24] = 3; fChannel[0][24] = 6;
//  fSFP[0][25] = 0; fSlave[0][25] = 2; fChannel[0][25] = 14;
//  fSFP[0][26] = 0; fSlave[0][26] = 0; fChannel[0][26] = 6;
//  fSFP[0][27] = 0; fSlave[0][27] = 1; fChannel[0][27] = 14;
//  fSFP[0][28] = 0; fSlave[0][28] = 3; fChannel[0][28] = 7;
//  fSFP[0][29] = 0; fSlave[0][29] = 2; fChannel[0][29] = 15;
//  fSFP[0][30] = 0; fSlave[0][30] = 0; fChannel[0][30] = 7;
//  fSFP[0][31] = 0; fSlave[0][31] = 1; fChannel[0][31] = 15;
//  fSFP[0][32] = 0; fSlave[0][32] = 3; fChannel[0][32] = 8;
//  fSFP[0][33] = 0; fSlave[0][33] = 2; fChannel[0][33] = 0;
//  fSFP[0][34] = 0; fSlave[0][34] = 0; fChannel[0][34] = 8;
//  fSFP[0][35] = 0; fSlave[0][35] = 1; fChannel[0][35] = 0;
//  fSFP[0][36] = 0; fSlave[0][36] = 3; fChannel[0][36] = 9;
//  fSFP[0][37] = 0; fSlave[0][37] = 2; fChannel[0][37] = 1;
//  fSFP[0][38] = 0; fSlave[0][38] = 0; fChannel[0][38] = 9;
//  fSFP[0][39] = 0; fSlave[0][39] = 1; fChannel[0][39] = 1;
//  fSFP[0][40] = 0; fSlave[0][40] = 3; fChannel[0][40] = 10;
//  fSFP[0][41] = 0; fSlave[0][41] = 2; fChannel[0][41] = 2;
//  fSFP[0][42] = 0; fSlave[0][42] = 0; fChannel[0][42] = 10;
//  fSFP[0][43] = 0; fSlave[0][43] = 1; fChannel[0][43] = 2;
//  fSFP[0][44] = 0; fSlave[0][44] = 3; fChannel[0][44] = 11;
//  fSFP[0][45] = 0; fSlave[0][45] = 2; fChannel[0][45] = 3;
//  fSFP[0][46] = 0; fSlave[0][46] = 0; fChannel[0][46] = 11;
//  fSFP[0][47] = 0; fSlave[0][47] = 1; fChannel[0][47] = 3;
//  fSFP[0][48] = 0; fSlave[0][48] = 3; fChannel[0][48] = 12;
//  fSFP[0][49] = 0; fSlave[0][49] = 2; fChannel[0][49] = 4;
//  fSFP[0][50] = 0; fSlave[0][50] = 0; fChannel[0][50] = 12;
//  fSFP[0][51] = 0; fSlave[0][51] = 1; fChannel[0][51] = 4;
//  fSFP[0][52] = 0; fSlave[0][52] = 3; fChannel[0][52] = 13;
//  fSFP[0][53] = 0; fSlave[0][53] = 2; fChannel[0][53] = 5;
//  fSFP[0][54] = 0; fSlave[0][54] = 0; fChannel[0][54] = 13;
//  fSFP[0][55] = 0; fSlave[0][55] = 1; fChannel[0][55] = 5;
//  fSFP[0][56] = 0; fSlave[0][56] = 3; fChannel[0][56] = 14;
//  fSFP[0][57] = 0; fSlave[0][57] = 2; fChannel[0][57] = 6;
//
//
//  fSFP[0][58] = 0; fSlave[0][58] = 0; fChannel[0][58] = 14;
//  fSFP[0][59] = 0; fSlave[0][59] = 1; fChannel[0][59] = 6;
//  fSFP[0][60] = 0; fSlave[0][60] = 3; fChannel[0][60] = 15;
//  fSFP[0][61] = 0; fSlave[0][61] = 2; fChannel[0][61] = 7;
//  fSFP[0][62] = 0; fSlave[0][62] = 0; fChannel[0][62] = 15;
//  fSFP[0][63] = 0; fSlave[0][63] = 1; fChannel[0][63] = 7;
//  fSFP[0][64] = 0; fSlave[0][64] = 7; fChannel[0][64] = 0;
//  fSFP[0][65] = 0; fSlave[0][65] = 6; fChannel[0][65] = 8;
//  fSFP[0][66] = 0; fSlave[0][66] = 4; fChannel[0][66] = 0;
//  fSFP[0][67] = 0; fSlave[0][67] = 5; fChannel[0][67] = 8;
//  fSFP[0][68] = 0; fSlave[0][68] = 7; fChannel[0][68] = 1;
//  fSFP[0][69] = 0; fSlave[0][69] = 6; fChannel[0][69] = 9;
//  fSFP[0][70] = 0; fSlave[0][70] = 4; fChannel[0][70] = 1;
//  fSFP[0][71] = 0; fSlave[0][71] = 5; fChannel[0][71] = 9;
//  fSFP[0][72] = 0; fSlave[0][72] = 7; fChannel[0][72] = 2;
//  fSFP[0][73] = 0; fSlave[0][73] = 6; fChannel[0][73] = 10;
//  fSFP[0][74] = 0; fSlave[0][74] = 4; fChannel[0][74] = 2;
//  fSFP[0][75] = 0; fSlave[0][75] = 5; fChannel[0][75] = 10;
//  fSFP[0][76] = 0; fSlave[0][76] = 7; fChannel[0][76] = 3;
//  fSFP[0][77] = 0; fSlave[0][77] = 6; fChannel[0][77] = 11;
//  fSFP[0][78] = 0; fSlave[0][78] = 4; fChannel[0][78] = 3;
//  fSFP[0][79] = 0; fSlave[0][79] = 5; fChannel[0][79] = 11;
//  fSFP[0][80] = 0; fSlave[0][80] = 7; fChannel[0][80] = 4;
//  fSFP[0][81] = 0; fSlave[0][81] = 6; fChannel[0][81] = 12;
//  fSFP[0][82] = 0; fSlave[0][82] = 4; fChannel[0][82] = 4;
//  fSFP[0][83] = 0; fSlave[0][83] = 5; fChannel[0][83] = 12;
//  fSFP[0][84] = 0; fSlave[0][84] = 7; fChannel[0][84] = 5;
//  fSFP[0][85] = 0; fSlave[0][85] = 6; fChannel[0][85] = 13;
//  fSFP[0][86] = 0; fSlave[0][86] = 4; fChannel[0][86] = 5;
//  fSFP[0][87] = 0; fSlave[0][87] = 5; fChannel[0][87] = 13;
//  fSFP[0][88] = 0; fSlave[0][88] = 7; fChannel[0][88] = 6;
//  fSFP[0][89] = 0; fSlave[0][89] = 6; fChannel[0][89] = 14;
//  fSFP[0][90] = 0; fSlave[0][90] = 4; fChannel[0][90] = 6;
//  fSFP[0][91] = 0; fSlave[0][91] = 5; fChannel[0][91] = 14;
//  fSFP[0][92] = 0; fSlave[0][92] = 7; fChannel[0][92] = 7;
//  fSFP[0][93] = 0; fSlave[0][93] = 6; fChannel[0][93] = 15;
//  fSFP[0][94] = 0; fSlave[0][94] = 4; fChannel[0][94] = 7;
//  fSFP[0][95] = 0; fSlave[0][95] = 5; fChannel[0][95] = 15;
//  fSFP[0][96] = 0; fSlave[0][96] = 7; fChannel[0][96] = 8;
//  fSFP[0][97] = 0; fSlave[0][97] = 6; fChannel[0][97] = 0;
//  fSFP[0][98] = 0; fSlave[0][98] = 4; fChannel[0][98] = 8;
//  fSFP[0][99] = 0; fSlave[0][99] = 5; fChannel[0][99] = 0;
//  fSFP[0][100] = 0; fSlave[0][100] = 7; fChannel[0][100] = 9;
//  fSFP[0][101] = 0; fSlave[0][101] = 6; fChannel[0][101] = 1;
//  fSFP[0][102] = 0; fSlave[0][102] = 4; fChannel[0][102] = 9;
//  fSFP[0][103] = 0; fSlave[0][103] = 5; fChannel[0][103] = 1;
//  fSFP[0][104] = 0; fSlave[0][104] = 7; fChannel[0][104] = 10;
//  fSFP[0][105] = 0; fSlave[0][105] = 6; fChannel[0][105] = 2;
//  fSFP[0][106] = 0; fSlave[0][106] = 4; fChannel[0][106] = 10;
//  fSFP[0][107] = 0; fSlave[0][107] = 5; fChannel[0][107] = 2;
//  fSFP[0][108] = 0; fSlave[0][108] = 7; fChannel[0][108] = 11;
//  fSFP[0][109] = 0; fSlave[0][109] = 6; fChannel[0][109] = 3;
//  fSFP[0][110] = 0; fSlave[0][110] = 4; fChannel[0][110] = 11;
//  fSFP[0][111] = 0; fSlave[0][111] = 5; fChannel[0][111] = 3;
//  fSFP[0][112] = 0; fSlave[0][112] = 7; fChannel[0][112] = 12;
//  fSFP[0][113] = 0; fSlave[0][113] = 6; fChannel[0][113] = 4;
//  fSFP[0][114] = 0; fSlave[0][114] = 4; fChannel[0][114] = 12;
//  fSFP[0][115] = 0; fSlave[0][115] = 5; fChannel[0][115] = 4;
//  fSFP[0][116] = 0; fSlave[0][116] = 7; fChannel[0][116] = 13;
//  fSFP[0][117] = 0; fSlave[0][117] = 6; fChannel[0][117] = 5;
//  fSFP[0][118] = 0; fSlave[0][118] = 4; fChannel[0][118] = 13;
//  fSFP[0][119] = 0; fSlave[0][119] = 5; fChannel[0][119] = 5;
//  fSFP[0][120] = 0; fSlave[0][120] = 7; fChannel[0][120] = 14;
//  fSFP[0][121] = 0; fSlave[0][121] = 6; fChannel[0][121] = 6;
//  fSFP[0][122] = 0; fSlave[0][122] = 4; fChannel[0][122] = 14;
//  fSFP[0][123] = 0; fSlave[0][123] = 5; fChannel[0][123] = 6;
//  fSFP[0][124] = 0; fSlave[0][124] = 7; fChannel[0][124] = 15;
//  fSFP[0][125] = 0; fSlave[0][125] = 6; fChannel[0][125] = 7;
//  fSFP[0][126] = 0; fSlave[0][126] = 4; fChannel[0][126] = 15;
//  fSFP[0][127] = 0; fSlave[0][127] = 5; fChannel[0][127] = 7;
//  fSFP[0][128] = 1; fSlave[0][128] = 3; fChannel[0][128] = 0;
//  fSFP[0][129] = 1; fSlave[0][129] = 2; fChannel[0][129] = 8;
//  fSFP[0][130] = 1; fSlave[0][130] = 0; fChannel[0][130] = 0;
//  fSFP[0][131] = 1; fSlave[0][131] = 1; fChannel[0][131] = 8;
//  fSFP[0][132] = 1; fSlave[0][132] = 3; fChannel[0][132] = 1;
//  fSFP[0][133] = 1; fSlave[0][133] = 2; fChannel[0][133] = 9;
//  fSFP[0][134] = 1; fSlave[0][134] = 0; fChannel[0][134] = 1;
//  fSFP[0][135] = 1; fSlave[0][135] = 1; fChannel[0][135] = 9;
//  fSFP[0][136] = 1; fSlave[0][136] = 3; fChannel[0][136] = 2;
//  fSFP[0][137] = 1; fSlave[0][137] = 2; fChannel[0][137] = 10;
//  fSFP[0][138] = 1; fSlave[0][138] = 0; fChannel[0][138] = 2;
//  fSFP[0][139] = 1; fSlave[0][139] = 1; fChannel[0][139] = 10;
//  fSFP[0][140] = 1; fSlave[0][140] = 3; fChannel[0][140] = 3;
//  fSFP[0][141] = 1; fSlave[0][141] = 2; fChannel[0][141] = 11;
//  fSFP[0][142] = 1; fSlave[0][142] = 0; fChannel[0][142] = 3;
//  fSFP[0][143] = 1; fSlave[0][143] = 1; fChannel[0][143] = 11;
//  fSFP[0][144] = 1; fSlave[0][144] = 3; fChannel[0][144] = 4;
//  fSFP[0][145] = 1; fSlave[0][145] = 2; fChannel[0][145] = 12;
//  fSFP[0][146] = 1; fSlave[0][146] = 0; fChannel[0][146] = 4;
//  fSFP[0][147] = 1; fSlave[0][147] = 1; fChannel[0][147] = 12;
//  fSFP[0][148] = 1; fSlave[0][148] = 3; fChannel[0][148] = 5;
//  fSFP[0][149] = 1; fSlave[0][149] = 2; fChannel[0][149] = 13;
//  fSFP[0][150] = 1; fSlave[0][150] = 0; fChannel[0][150] = 5;
//  fSFP[0][151] = 1; fSlave[0][151] = 1; fChannel[0][151] = 13;
//  fSFP[0][152] = 1; fSlave[0][152] = 3; fChannel[0][152] = 6;
//  fSFP[0][153] = 1; fSlave[0][153] = 2; fChannel[0][153] = 14;
//  fSFP[0][154] = 1; fSlave[0][154] = 0; fChannel[0][154] = 6;
//  fSFP[0][155] = 1; fSlave[0][155] = 1; fChannel[0][155] = 14;
//  fSFP[0][156] = 1; fSlave[0][156] = 3; fChannel[0][156] = 7;
//  fSFP[0][157] = 1; fSlave[0][157] = 2; fChannel[0][157] = 15;
//  fSFP[0][158] = 1; fSlave[0][158] = 0; fChannel[0][158] = 7;
//  fSFP[0][159] = 1; fSlave[0][159] = 1; fChannel[0][159] = 15;
//  fSFP[0][160] = 1; fSlave[0][160] = 3; fChannel[0][160] = 8;
//  fSFP[0][161] = 1; fSlave[0][161] = 2; fChannel[0][161] = 0;
//  fSFP[0][162] = 1; fSlave[0][162] = 0; fChannel[0][162] = 8;
//  fSFP[0][163] = 1; fSlave[0][163] = 1; fChannel[0][163] = 0;
//  fSFP[0][164] = 1; fSlave[0][164] = 3; fChannel[0][164] = 9;
//  fSFP[0][165] = 1; fSlave[0][165] = 2; fChannel[0][165] = 1;
//  fSFP[0][166] = 1; fSlave[0][166] = 0; fChannel[0][166] = 9;
//  fSFP[0][167] = 1; fSlave[0][167] = 1; fChannel[0][167] = 1;
//  fSFP[0][168] = 1; fSlave[0][168] = 3; fChannel[0][168] = 10;
//  fSFP[0][169] = 1; fSlave[0][169] = 2; fChannel[0][169] = 2;
//  fSFP[0][170] = 1; fSlave[0][170] = 0; fChannel[0][170] = 10;
//  fSFP[0][171] = 1; fSlave[0][171] = 1; fChannel[0][171] = 2;
//  fSFP[0][172] = 1; fSlave[0][172] = 3; fChannel[0][172] = 11;
//  fSFP[0][173] = 1; fSlave[0][173] = 2; fChannel[0][173] = 3;
//  fSFP[0][174] = 1; fSlave[0][174] = 0; fChannel[0][174] = 11;
//  fSFP[0][175] = 1; fSlave[0][175] = 1; fChannel[0][175] = 3;
//  fSFP[0][176] = 1; fSlave[0][176] = 3; fChannel[0][176] = 12;
//  fSFP[0][177] = 1; fSlave[0][177] = 2; fChannel[0][177] = 4;
//  fSFP[0][178] = 1; fSlave[0][178] = 0; fChannel[0][178] = 12;
//  fSFP[0][179] = 1; fSlave[0][179] = 1; fChannel[0][179] = 4;
//  fSFP[0][180] = 1; fSlave[0][180] = 3; fChannel[0][180] = 13;
//  fSFP[0][181] = 1; fSlave[0][181] = 2; fChannel[0][181] = 5;
//  fSFP[0][182] = 1; fSlave[0][182] = 0; fChannel[0][182] = 13;
//  fSFP[0][183] = 1; fSlave[0][183] = 1; fChannel[0][183] = 5;
//  fSFP[0][184] = 1; fSlave[0][184] = 3; fChannel[0][184] = 14;
//  fSFP[0][185] = 1; fSlave[0][185] = 2; fChannel[0][185] = 6;
//  fSFP[0][186] = 1; fSlave[0][186] = 0; fChannel[0][186] = 14;
//  fSFP[0][187] = 1; fSlave[0][187] = 1; fChannel[0][187] = 6;
//  fSFP[0][188] = 1; fSlave[0][188] = 3; fChannel[0][188] = 15;
//  fSFP[0][189] = 1; fSlave[0][189] = 2; fChannel[0][189] = 7;
//  fSFP[0][190] = 1; fSlave[0][190] = 0; fChannel[0][190] = 15;
//  fSFP[0][191] = 1; fSlave[0][191] = 1; fChannel[0][191] = 7;
//  fSFP[0][192] = 1; fSlave[0][192] = 7; fChannel[0][192] = 0;
//  fSFP[0][193] = 1; fSlave[0][193] = 6; fChannel[0][193] = 8;
//  fSFP[0][194] = 1; fSlave[0][194] = 4; fChannel[0][194] = 0;
//  fSFP[0][195] = 1; fSlave[0][195] = 5; fChannel[0][195] = 8;
//  fSFP[0][196] = 1; fSlave[0][196] = 7; fChannel[0][196] = 1;
//  fSFP[0][197] = 1; fSlave[0][197] = 6; fChannel[0][197] = 9;
//  fSFP[0][198] = 1; fSlave[0][198] = 4; fChannel[0][198] = 1;
//  fSFP[0][199] = 1; fSlave[0][199] = 5; fChannel[0][199] = 9;
//  fSFP[0][200] = 1; fSlave[0][200] = 7; fChannel[0][200] = 2;
//  fSFP[0][201] = 1; fSlave[0][201] = 6; fChannel[0][201] = 10;
//  fSFP[0][202] = 1; fSlave[0][202] = 4; fChannel[0][202] = 2;
//  fSFP[0][203] = 1; fSlave[0][203] = 5; fChannel[0][203] = 10;
//  fSFP[0][204] = 1; fSlave[0][204] = 7; fChannel[0][204] = 3;
//  fSFP[0][205] = 1; fSlave[0][205] = 6; fChannel[0][205] = 11;
//  fSFP[0][206] = 1; fSlave[0][206] = 4; fChannel[0][206] = 3;
//  fSFP[0][207] = 1; fSlave[0][207] = 5; fChannel[0][207] = 11;
//  fSFP[0][208] = 1; fSlave[0][208] = 7; fChannel[0][208] = 4;
//  fSFP[0][209] = 1; fSlave[0][209] = 6; fChannel[0][209] = 12;
//  fSFP[0][210] = 1; fSlave[0][210] = 4; fChannel[0][210] = 4;
//  fSFP[0][211] = 1; fSlave[0][211] = 5; fChannel[0][211] = 12;
//  fSFP[0][212] = 1; fSlave[0][212] = 7; fChannel[0][212] = 5;
//  fSFP[0][213] = 1; fSlave[0][213] = 6; fChannel[0][213] = 13;
//  fSFP[0][214] = 1; fSlave[0][214] = 4; fChannel[0][214] = 5;
//  fSFP[0][215] = 1; fSlave[0][215] = 5; fChannel[0][215] = 13;
//  fSFP[0][216] = 1; fSlave[0][216] = 7; fChannel[0][216] = 6;
//  fSFP[0][217] = 1; fSlave[0][217] = 6; fChannel[0][217] = 14;
//  fSFP[0][218] = 1; fSlave[0][218] = 4; fChannel[0][218] = 6;
//  fSFP[0][219] = 1; fSlave[0][219] = 5; fChannel[0][219] = 14;
//  fSFP[0][220] = 1; fSlave[0][220] = 7; fChannel[0][220] = 7;
//  fSFP[0][221] = 1; fSlave[0][221] = 6; fChannel[0][221] = 15;
//  fSFP[0][222] = 1; fSlave[0][222] = 4; fChannel[0][222] = 7;
//  fSFP[0][223] = 1; fSlave[0][223] = 5; fChannel[0][223] = 15;
//  fSFP[0][224] = 1; fSlave[0][224] = 7; fChannel[0][224] = 8;
//  fSFP[0][225] = 1; fSlave[0][225] = 6; fChannel[0][225] = 0;
//  fSFP[0][226] = 1; fSlave[0][226] = 4; fChannel[0][226] = 8;
//  fSFP[0][227] = 1; fSlave[0][227] = 5; fChannel[0][227] = 0;
//  fSFP[0][228] = 1; fSlave[0][228] = 7; fChannel[0][228] = 9;
//  fSFP[0][229] = 1; fSlave[0][229] = 6; fChannel[0][229] = 1;
//  fSFP[0][230] = 1; fSlave[0][230] = 4; fChannel[0][230] = 9;
//  fSFP[0][231] = 1; fSlave[0][231] = 5; fChannel[0][231] = 1;
//  fSFP[0][232] = 1; fSlave[0][232] = 7; fChannel[0][232] = 10;
//  fSFP[0][233] = 1; fSlave[0][233] = 6; fChannel[0][233] = 2;
//  fSFP[0][234] = 1; fSlave[0][234] = 4; fChannel[0][234] = 10;
//  fSFP[0][235] = 1; fSlave[0][235] = 5; fChannel[0][235] = 2;
//  fSFP[0][236] = 1; fSlave[0][236] = 7; fChannel[0][236] = 11;
//  fSFP[0][237] = 1; fSlave[0][237] = 6; fChannel[0][237] = 3;
//  fSFP[0][238] = 1; fSlave[0][238] = 4; fChannel[0][238] = 11;
//  fSFP[0][239] = 1; fSlave[0][239] = 5; fChannel[0][239] = 3;
//  fSFP[0][240] = 1; fSlave[0][240] = 7; fChannel[0][240] = 12;
//  fSFP[0][241] = 1; fSlave[0][241] = 6; fChannel[0][241] = 4;
//  fSFP[0][242] = 1; fSlave[0][242] = 4; fChannel[0][242] = 12;
//  fSFP[0][243] = 1; fSlave[0][243] = 5; fChannel[0][243] = 4;
//  fSFP[0][244] = 1; fSlave[0][244] = 7; fChannel[0][244] = 13;
//  fSFP[0][245] = 1; fSlave[0][245] = 6; fChannel[0][245] = 5;
//  fSFP[0][246] = 1; fSlave[0][246] = 4; fChannel[0][246] = 13;
//  fSFP[0][247] = 1; fSlave[0][247] = 5; fChannel[0][247] = 5;
//  fSFP[0][248] = 1; fSlave[0][248] = 7; fChannel[0][248] = 14;
//  fSFP[0][249] = 1; fSlave[0][249] = 6; fChannel[0][249] = 6;
//  fSFP[0][250] = 1; fSlave[0][250] = 4; fChannel[0][250] = 14;
//  fSFP[0][251] = 1; fSlave[0][251] = 5; fChannel[0][251] = 6;
//  fSFP[0][252] = 1; fSlave[0][252] = 7; fChannel[0][252] = 15;
//  fSFP[0][253] = 1; fSlave[0][253] = 6; fChannel[0][253] = 7;
//  fSFP[0][254] = 1; fSlave[0][254] = 4; fChannel[0][254] = 15;
//  fSFP[0][255] = 1; fSlave[0][255] = 5; fChannel[0][255] = 7;
//
}

//-----------------------------------------------------------
Int_t TAwagsSisMapParam::PrintParameter(Text_t * n, Int_t){
  return 0;
// TODO JAM 2019

}
//-----------------------------------------------------------
Bool_t TAwagsSisMapParam::UpdateFrom(TGo4Parameter *pp){
  if(pp->InheritsFrom("TAwagsSisMapParam"))
  {
    TAwagsSisMapParam * from;
    from = (TAwagsSisMapParam *) pp;

    for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
       {
         fMaxWire[dev]=from->fMaxWire[dev];
         for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
         {
           fSFP [dev][wire]=from->fSFP [dev][wire];
           fSlave [dev][wire]=from->fSlave [dev][wire];
           fChannel[dev][wire]=from->fChannel[dev][wire];
         }

       }
    fSlowMotion=from->fSlowMotion;
    fStopAtEachSpill=from->fStopAtEachSpill;
    fMaxChamber=from->fMaxChamber;
  }
     else
     cout << "Wrong parameter object: " << pp->ClassName() << endl;
  return kTRUE;
}

//----------------------------END OF GO4 SOURCE FILE ---------------------
