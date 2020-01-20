// written by macro saveparam.C at Mon Jan 20 08:25:51 2020
void set_DDASAnalysisParameter()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_DDASAnalysisParameter can execute only in analysis" << std::endl;
   return;
#endif
   TDDASAnalysisParameter* param1 = (TDDASAnalysisParameter*) go4->GetParameter("DDASAnalysisParameter","TDDASAnalysisParameter");

   if (param1==0) {
      TGo4Log::Error("Could not find parameter DDASAnalysisParameter of class TDDASAnalysisParameter");
      return;
   }

   TGo4Log::Info("Set parameter DDASAnalysisParameter as saved at Mon Jan 20 08:25:51 2020");

   param1->fUseGate = kTRUE;
   param1->fGateName = "TheDefaultCut";
   param1->fClockMax = 150;
   param1->fMinImplantTime = 150;
   param1->fRandomFlag = 0;

}
