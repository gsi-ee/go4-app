// written by macro saveparam.C at Mon May 23 13:54:20 2016
void set_Feb3BasicParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_Feb3BasicParam can execute only in analysis" << std::endl;
   return;
#endif
   TFeb3BasicParam* param0 = (TFeb3BasicParam*) go4->GetParameter("Feb3BasicParam","TFeb3BasicParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter Feb3BasicParam of class TFeb3BasicParam");
      return;
   }

   TGo4Log::Info("Set parameter Feb3BasicParam as saved at Mon May 23 13:54:20 2016");

   // disable grid mapping for performance reasons here:
   param0->fDoGridMapping = kTRUE;

   // febex id to sfp and slave index:
   // param0->fBoardID[sfp][slave] = febexid;
   param0->fBoardID[0][0] = 14;
   param0->fBoardID[0][1] = 18;


   param0->fNumGrids = 2;
   param0->fGridDeviceID[0] = 42;
   param0->fGridDeviceID[1] = 43;

 for (int wire = 0; wire < 32; ++wire)
       {

         // high gain grid (index 0):
          param0->fGridBoardID_X[1][wire] = -1; // not used
          param0->fGridBoardID_Y[0][wire] = -1; //not used
       }
   
   
   
   
   
   // mapping of wires on grid to febex id
   int grid = 0;
     for (int wire = 0; wire < 16; ++wire)
     {
          param0->fGridBoardID_X[grid][wire] = 14;
     }
     for (int wire = 16; wire < 32; ++wire)
     {
          param0->fGridBoardID_X[grid][wire] = 18;
     }
     
     

   grid = 1;
          for (int wire = 0; wire < 16; ++wire)
          {
             param0->fGridBoardID_Y[grid][wire] = 14;	     
          }
          for (int wire = 16; wire < 32; ++wire)
          {
             param0->fGridBoardID_Y[grid][wire] = 18;
          }


   // here mapping of wires on grid to febex channels:
   // febex_channel -1 disables mapping
   // fGridChannel_X[grid][wire] = febex_channel
  
  // 1st 16 channels on Febex 1  
  // APFEL Configuration on Febex: high ampl. = even (0,2,4..)  Low ampl. = odd (1,3,5..)
   
   
   param0->fGridChannel_X[0][0] = -1;
   param0->fGridChannel_X[0][1] = 12;
   param0->fGridChannel_X[0][2] = -1;
   param0->fGridChannel_X[0][3] = 14;
   param0->fGridChannel_X[0][4] = -1;
   param0->fGridChannel_X[0][5] = 8;
   param0->fGridChannel_X[0][6] = -1;
   param0->fGridChannel_X[0][7] = 10;
   param0->fGridChannel_X[0][8] = -1;
   param0->fGridChannel_X[0][9] = 6;
   param0->fGridChannel_X[0][10] = -1;
   param0->fGridChannel_X[0][11] = 4;
   param0->fGridChannel_X[0][12] = -1;
   param0->fGridChannel_X[0][13] = 2;
   param0->fGridChannel_X[0][14] = -1;
   param0->fGridChannel_X[0][15] = 0;

   
   // 2nd 16 channels on Febex 2
   
   param0->fGridChannel_X[0][16] = -1;
   param0->fGridChannel_X[0][17] = 12;
   param0->fGridChannel_X[0][18] = -1;
   param0->fGridChannel_X[0][19] = 14;
   param0->fGridChannel_X[0][20] = -1;
   param0->fGridChannel_X[0][21] = 8;
   param0->fGridChannel_X[0][22] = -1;
   param0->fGridChannel_X[0][23] = 10;
   param0->fGridChannel_X[0][24] = -1;
   param0->fGridChannel_X[0][25] = 6;
   param0->fGridChannel_X[0][26] = -1;
   param0->fGridChannel_X[0][27] = 4;
   param0->fGridChannel_X[0][28] = -1;
   param0->fGridChannel_X[0][29] = 2;
   param0->fGridChannel_X[0][30] = -1;
   param0->fGridChannel_X[0][31] = 0;

   
   
    // not used
   

   param0->fGridChannel_X[1][0] = -1;
   param0->fGridChannel_X[1][1] = -1;
   param0->fGridChannel_X[1][2] = -1;
   param0->fGridChannel_X[1][3] = -1;
   param0->fGridChannel_X[1][4] = -1;
   param0->fGridChannel_X[1][5] = -1;
   param0->fGridChannel_X[1][6] = -1;
   param0->fGridChannel_X[1][7] = -1;
   param0->fGridChannel_X[1][8] = -1;
   param0->fGridChannel_X[1][9] = -1;
   param0->fGridChannel_X[1][10] = -1;
   param0->fGridChannel_X[1][11] = -1;
   param0->fGridChannel_X[1][12] = -1;
   param0->fGridChannel_X[1][13] = -1;
   param0->fGridChannel_X[1][14] = -1;
   param0->fGridChannel_X[1][15] = -1;


    // Low ampl. for 1st 16 channels on Febex 1


   param0->fGridChannel_Y[1][0] = -1;
   param0->fGridChannel_Y[1][1] = 13;
   param0->fGridChannel_Y[1][2] = -1;
   param0->fGridChannel_Y[1][3] = 15;
   param0->fGridChannel_Y[1][4] = -1;
   param0->fGridChannel_Y[1][5] = 9;
   param0->fGridChannel_Y[1][6] = -1;
   param0->fGridChannel_Y[1][7] = 11;
   param0->fGridChannel_Y[1][8] = -1;
   param0->fGridChannel_Y[1][9] = 7;
   param0->fGridChannel_Y[1][10] = -1;
   param0->fGridChannel_Y[1][11] = 5;
   param0->fGridChannel_Y[1][12] = -1;
   param0->fGridChannel_Y[1][13] = 3;
   param0->fGridChannel_Y[1][14] = -1;
   param0->fGridChannel_Y[1][15] = 1;

  
  
   // not used

   param0->fGridChannel_Y[1][16] = -1;
   param0->fGridChannel_Y[1][17] = 13;
   param0->fGridChannel_Y[1][18] = -1;
   param0->fGridChannel_Y[1][19] = 15;
   param0->fGridChannel_Y[1][20] = -1;
   param0->fGridChannel_Y[1][21] = 9;
   param0->fGridChannel_Y[1][22] = -1;
   param0->fGridChannel_Y[1][23] = 11;
   param0->fGridChannel_Y[1][24] = -1;
   param0->fGridChannel_Y[1][25] = 7;
   param0->fGridChannel_Y[1][26] = -1;
   param0->fGridChannel_Y[1][27] = 5;
   param0->fGridChannel_Y[1][28] = -1;
   param0->fGridChannel_Y[1][29] = 3;
   param0->fGridChannel_Y[1][30] = -1;
   param0->fGridChannel_Y[1][31] = 1;





}
