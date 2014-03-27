// written by macro saveparam.C at Fri Feb 21 16:08:17 2014
void set_QFWRawParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_QFWRawParam can execute only in analysis" << std::endl;
   return;
#endif
   TQFWRawParam* param0 = (TQFWRawParam*) go4->GetParameter("QFWRawParam","TQFWRawParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter QFWRawParam of class TQFWRawParam");
      return;
   }

   TGo4Log::Info("Set parameter QFWRawParam as saved at Fri Feb 21 16:08:18 2014");

//   param0->fBoardID[0][0] = 11;
//   param0->fBoardID[0][1] = -1;
//   param0->fBoardID[0][2] = -1;
//   param0->fBoardID[1][0] = 10;
//   param0->fBoardID[1][1] = -1;
//   param0->fBoardID[1][2] = -1;
//   param0->fBoardID[2][0] = -1;
//   param0->fBoardID[2][1] = -1;
//   param0->fBoardID[2][2] = -1;
//   param0->fBoardID[3][0] = -1;
//   param0->fBoardID[3][1] = -1;
//   param0->fBoardID[3][2] = -1;
   param0->fSimpleCompensation = kFALSE;
   param0->fSelectTriggerEvents = kFALSE;
   param0->fTriggerHighThreshold = 5000000;
   param0->fTriggerLowThreshold = 4000000;
   param0->fTriggerBoardID = 12;
   param0->fTriggerFirstChannel = 0;
   param0->fTriggerLastChannel = 1;

}
