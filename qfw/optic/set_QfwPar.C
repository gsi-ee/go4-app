void set_QfwPar()
{
#ifndef __GO4ANAMACRO__
   cout << "Macro set_QfwPar can execute only in analysis" << endl;
   return;
#endif
   TOpticQFWParam* param1 = (TOpticQFWParam*) go4->GetParameter("QFWSetup", "TOpticQFWParam");

   if (param1==0) {
      TGo4Log::Error("Could not find parameter QFWSetup");
      return;
   }

   TGo4Log::Info("Set parameter QFWSetup values");

   param1->fSimpleCompensation = kTRUE;

   param1->fMeasureBackground = kFALSE;
   param1->fCorrectBackground = kFALSE;
}
