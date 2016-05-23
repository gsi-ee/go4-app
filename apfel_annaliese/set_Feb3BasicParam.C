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
   param0->fBoardID[0][0] = 10;
   param0->fBoardID[0][1] = 11;


   param0->fNumGrids = 2;
   param0->fGridDeviceID[0] = 42;
   param0->fGridDeviceID[1] = 43;


   // mapping of wires on grid to febex id
   int grid = 0;
     for (int wire = 0; wire < 32; ++wire)
     {
        param0->fGridBoardID_X[grid][wire] = 10;
        param0->fGridBoardID_Y[grid][wire] = 11;
     }

   grid = 1;
          for (int wire = 0; wire < 32; ++wire)
          {
             param0->fGridBoardID_X[grid][wire] = 10;
             param0->fGridBoardID_Y[grid][wire] = 11;
          }


   // here mapping of wires on grid to febex channels:
   // febex_channel -1 disables mapping
   // fGridChannel_X[grid][wire] = febex_channel
   param0->fGridChannel_X[0][0] = -1;
   param0->fGridChannel_X[0][1] = -1;
   param0->fGridChannel_X[0][2] = -1;
   param0->fGridChannel_X[0][3] = -1;
   param0->fGridChannel_X[0][4] = -1;
   param0->fGridChannel_X[0][5] = -1;
   param0->fGridChannel_X[0][6] = -1;
   param0->fGridChannel_X[0][7] = -1;
   param0->fGridChannel_X[0][8] = -1;
   param0->fGridChannel_X[0][9] = 0;
   param0->fGridChannel_X[0][10] = 2;
   param0->fGridChannel_X[0][11] = -1;
   param0->fGridChannel_X[0][12] = 10;
   param0->fGridChannel_X[0][13] = 4;
   param0->fGridChannel_X[0][14] = 6;
   param0->fGridChannel_X[0][15] = 14;


   param0->fGridChannel_X[1][0] = -1;
   param0->fGridChannel_X[1][1] = -1;
   param0->fGridChannel_X[1][2] = -1;
   param0->fGridChannel_X[1][3] = -1;
   param0->fGridChannel_X[1][4] = -1;
   param0->fGridChannel_X[1][5] = -1;
   param0->fGridChannel_X[1][6] = -1;
   param0->fGridChannel_X[1][7] = -1;
   param0->fGridChannel_X[1][8] = -1;
   param0->fGridChannel_X[1][9] = 1;
   param0->fGridChannel_X[1][10] = 3;
   param0->fGridChannel_X[1][11] = -1;
   param0->fGridChannel_X[1][12] = 11;
   param0->fGridChannel_X[1][13] = 5;
   param0->fGridChannel_X[1][14] = 7;
   param0->fGridChannel_X[1][15] = 15;




   param0->fGridChannel_Y[0][0] = -1;
   param0->fGridChannel_Y[0][1] = 4;
   param0->fGridChannel_Y[0][2] = -1;
   param0->fGridChannel_Y[0][3] = 14;
   param0->fGridChannel_Y[0][4] = 12;
   param0->fGridChannel_Y[0][5] = 10;
   param0->fGridChannel_Y[0][6] = 8;
   param0->fGridChannel_Y[0][7] = 0;
   param0->fGridChannel_Y[0][8] = 2;
   param0->fGridChannel_Y[0][9] = -1;
   param0->fGridChannel_Y[0][10] = -1;
   param0->fGridChannel_Y[0][11] = -1;
   param0->fGridChannel_Y[0][12] = -1;
   param0->fGridChannel_Y[0][13] = -1;
   param0->fGridChannel_Y[0][14] = -1;
   param0->fGridChannel_Y[0][15] = -1;


   param0->fGridChannel_Y[1][0] = -1;
   param0->fGridChannel_Y[1][1] = 5;
   param0->fGridChannel_Y[1][2] = -1;
   param0->fGridChannel_Y[1][3] = 15;
   param0->fGridChannel_Y[1][4] = 13;
   param0->fGridChannel_Y[1][5] = 11;
   param0->fGridChannel_Y[1][6] = 9;
   param0->fGridChannel_Y[1][7] = 1;
   param0->fGridChannel_Y[1][8] = 3;
   param0->fGridChannel_Y[1][9] = -1;
   param0->fGridChannel_Y[1][10] = -1;
   param0->fGridChannel_Y[1][11] = -1;
   param0->fGridChannel_Y[1][12] = -1;
   param0->fGridChannel_Y[1][13] = -1;
   param0->fGridChannel_Y[1][14] = -1;
   param0->fGridChannel_Y[1][15] = -1;





}
