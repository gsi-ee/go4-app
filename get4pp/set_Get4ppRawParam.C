// written by macro saveparam.C at Mon May  4 10:15:39 2020
void set_Get4ppRawParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_Get4ppRawParam can execute only in analysis" << std::endl;
   return;
#endif
   TGet4ppRawParam* param0 = (TGet4ppRawParam*) go4->GetParameter("Get4ppRawParam","TGet4ppRawParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter Get4ppRawParam of class TGet4ppRawParam");
      return;
   }

   TGo4Log::Info("Set parameter Get4ppRawParam as saved at Mon May  4 10:15:39 2020");

   param0->fBoardID[0] = 42;
   param0->fBoardID[1] = -1;
   param0->fBoardID[2] = -1;
   param0->fBoardID[3] = -1;
   param0->fBoardID[4] = -1;
   param0->fBoardID[5] = -1;
   param0->fBoardID[6] = -1;
   param0->fBoardID[7] = -1;
   param0->fBoardID[8] = -1;
   param0->fBoardID[9] = -1;
   param0->fVerbosity = 1;
   param0->fFineTimeStatsLimit =8000;

   param0->fSlowMotion = kFALSE;

}
