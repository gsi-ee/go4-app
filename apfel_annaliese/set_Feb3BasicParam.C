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

   // de-select all grid and channels...
   for (int wire = 0; wire < 16; ++wire)
       {

         // high gain grid (index 0):
          param0->fGridBoardID_X[0][wire] = -1; // not used
          param0->fGridBoardID_Y[0][wire] = -1; //not used

          param0->fGridBoardID_X[1][wire] = -1; // not used
          param0->fGridBoardID_Y[1][wire] = -1; //not used
       }
   
   // febex id to sfp and slave index:
   // param0->fBoardID[sfp][slave] = febexid;
   param0->fBoardID[0][0] = 14;
   param0->fBoardID[0][1] = 18;


   param0->fNumGrids = 2;
   param0->fGridDeviceID[0] = 42;    // high gain arbitrary id number
   param0->fGridDeviceID[1] = 43;    // low gain arbitrary id number


    // mapping of wires and grids to frontend boards:
    for (int wire = 0; wire < 8; ++wire)
       {

         // high gain grid (index 0):
         param0->fGridBoardID_X[0][wire] = 14; // x direction at febex0 -> board id 10
         param0->fGridBoardID_X[0][wire+8] = 18; // y direction at febex1 -> board id 11
         // high gain grid (index 1):
         param0->fGridBoardID_X[1][wire] = 14; // x direction at febex0 -> board id 10
         param0->fGridBoardID_X[1][wire+8] = 18;// y direction at febex1 -> board id 11
       }

    int grid=0; // first high gain

    param0->fGridChannel_X[grid][0] = 12;  //0
    param0->fGridChannel_X[grid][1] = 14; //0
    param0->fGridChannel_X[grid][2] = 8;//0
    param0->fGridChannel_X[grid][3] = 10; //0
    param0->fGridChannel_X[grid][4] = 6; //0
    param0->fGridChannel_X[grid][5] = 4;//0
    param0->fGridChannel_X[grid][6] = 2;  //0
    param0->fGridChannel_X[grid][7] = 0; //0
    param0->fGridChannel_X[grid][8] = 12;//0
    param0->fGridChannel_X[grid][9] = 14; //0
    param0->fGridChannel_X[grid][10] = 8; //0
    param0->fGridChannel_X[grid][11] = 10;//0
    param0->fGridChannel_X[grid][12] = 6;  //0
    param0->fGridChannel_X[grid][13] = 4; //0
    param0->fGridChannel_X[grid][14] = 2;//0
    param0->fGridChannel_X[grid][15] = 0; //0

    grid=1; // secondly low gain:
    param0->fGridChannel_X[grid][0] = 13;  //0
    param0->fGridChannel_X[grid][1] = 15; //0
    param0->fGridChannel_X[grid][2] = 9;//0
    param0->fGridChannel_X[grid][3] = 11; //0
    param0->fGridChannel_X[grid][4] = 7; //0
    param0->fGridChannel_X[grid][5] = 5;//0
    param0->fGridChannel_X[grid][6] = 3;  //0
    param0->fGridChannel_X[grid][7] = 1; //0
    param0->fGridChannel_X[grid][8] = 13;//0
    param0->fGridChannel_X[grid][9] = 15; //0
    param0->fGridChannel_X[grid][10] = 9; //0
    param0->fGridChannel_X[grid][11] = 11;//0
    param0->fGridChannel_X[grid][12] = 7;  //0
    param0->fGridChannel_X[grid][13] = 5; //0
    param0->fGridChannel_X[grid][14] = 3;//0
    param0->fGridChannel_X[grid][15] = 1; //0

}
