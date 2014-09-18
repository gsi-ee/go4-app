// written by macro saveparam.C at Thu Mar 27 12:22:33 2014
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

   TGo4Log::Info("Set parameter QFWRawParam as saved at Thu Mar 27 12:22:33 2014");

// uncomment lines below to override comiled/stored defaults:

// mapping of qfw frontend boards to gosip optical connections [sfpid][devid]

   param0->fBoardID[0][0] = 10;
   param0->fBoardID[1][0] = 11;
   param0->fBoardID[2][0] = 12;
   param0->fBoardID[3][0] = 13;

   param0->fSimpleCompensation = kFALSE;
   param0->fSelectTriggerEvents = kFALSE;

   param0->fTriggerHighThreshold = 5000000;
   param0->fTriggerLowThreshold = 4000000;
   param0->fTriggerBoardID = 13;
   param0->fTriggerFirstChannel = 0;
   param0->fTriggerLastChannel = 1;

// param0->fUseFrontendOffsets=true;
   param0->fFrontendOffsetTrigger=14;

}
