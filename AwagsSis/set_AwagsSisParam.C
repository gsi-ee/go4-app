// written by macro saveparam.C at Tue Aug  2 08:20:54 2022
void set_AwagsSisParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_AwagsSisParam can execute only in analysis" << std::endl;
   return;
#endif
   TAwagsSisParam* param0 = (TAwagsSisParam*) go4->GetParameter("AwagsSisParam","TAwagsSisParam");

   if (!param0) {
      TGo4Log::Error("Could not find parameter AwagsSisParam of class TAwagsSisParam");
      return;
   }

   TGo4Log::Info("Set parameter AwagsSisParam as saved at Tue Aug  2 08:20:54 2022");

   param0->fNumEventsPerSpill = 180;
   param0->fMaxSpillEvent = 170;
   param0->fMapSpills = kTRUE;
   param0->fSpillTriggerSFP = 0;
   param0->fSpillTriggerSlave = 1;
   param0->fSpillTriggerChan = 7;

}
