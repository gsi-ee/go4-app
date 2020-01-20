// written by macro saveparam.C at Mon Jan 20 08:25:51 2020
void set_DDASFilterParameter()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_DDASFilterParameter can execute only in analysis" << std::endl;
   return;
#endif
   TDDASFilterParameter* param0 = (TDDASFilterParameter*) go4->GetParameter("DDASFilterParameter","TDDASFilterParameter");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter DDASFilterParameter of class TDDASFilterParameter");
      return;
   }

   TGo4Log::Info("Set parameter DDASFilterParameter as saved at Mon Jan 20 08:25:51 2020");

   param0->fDoFiltering = kTRUE;
   param0->fDSSD_Front_Imax = 0;
   param0->fDSSD_Front_Dmax = 0;
   param0->fDSSD_Back_Imax = 0;
   param0->fDSSD_Back_Dmax = 0;

}
