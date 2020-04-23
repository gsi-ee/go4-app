// written by macro saveparam.C at Wed Aug 28 10:03:11 2019
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

   TGo4Log::Info("Set parameter Get4ppRawParam as saved at Wed Aug 28 10:03:11 2019");

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
//   param0->fNumSnapshots = 64;
//   param0->fTraceLength = 8;
//   param0->fDoCalibrate = kFALSE;
//   param0->fDoFFT = kTRUE;
//   param0->fFFTOptions = "R2C M";
//   param0->fFilterType = 0;
//   param0->fFilterCoeff[0] = 0;
//   param0->fFilterCoeff[1] = 0;
//   param0->fFilterCoeff[2] = 0;
//   param0->fFilterCoeff[3] = 0;
//   param0->fFilterCoeff[4] = 0;

}
