// written by macro saveparam.C at Wed Aug 28 10:03:11 2019 JAM adjusted to new format 27-03-2023
void set_Ctr16RawParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_Ctr16RawParam can execute only in analysis" << std::endl;
   return;
#endif
   TCtr16RawParam* param0 = (TCtr16RawParam*) go4->GetParameter("Ctr16RawParam","TCtr16RawParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter Ctr16RawParam of class TCtr16RawParam");
      return;
   }

   TGo4Log::Info("Set parameter Ctr16RawParam from default macro");

   //param0->fBoardID[0][0] = 42;
   param0->fBoardID[1][0] = 55;
   param0->fNumSnapshots = 64;
   param0->fTraceLength = 64;
   param0->fDoCalibrate = kFALSE;

  // param0->fMemorySampleStatsLimit =2000;

   //param0->fVerbosity=2;
   //param0->fSlowMotion=kFALSE;

}
