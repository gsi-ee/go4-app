// written by macro saveparam.C at Fri Sep 10 17:56:31 2021
void set_GemCSABasicParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_GemCSABasicParam can execute only in analysis" << std::endl;
   return;
#endif
   TGemCSABasicParam* param0 = (TGemCSABasicParam*) go4->GetParameter("GemCSABasicParam","TGemCSABasicParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter GemCSABasicParam of class TGemCSABasicParam");
      return;
   }

   TGo4Log::Info("Set parameter GemCSABasicParam as saved at Fri Sep 10 17:56:31 2021");

   param0->fDoBaselineFits = kFALSE;
   param0->fMapTraces = kTRUE;

}
