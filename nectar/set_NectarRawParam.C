// written by macro saveparam.C at Mon Sep 13 14:55:46 2021
// JAM feel free to modify this setup manually :)
void set_NectarRawParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_NectarRawParam can execute only in analysis" << std::endl;
   return;
#endif
   TNectarRawParam* param0 = (TNectarRawParam*) go4->GetParameter("NectarRawParam","TNectarRawParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter NectarRawParam of class TNectarRawParam");
      return;
   }

   TGo4Log::Info("Set parameter NectarRawParam as saved at Mon Sep 13 14:55:46 2021");

   param0->fVMMR_BoardID[0] =  1; // this is board ID send by frontends
   param0->fVMMR_BoardID[1] = -1; // -1 means no board is expected
   param0->fVMMR_BoardID[2] = -1;
   param0->fVMMR_BoardID[3] = -1;
   param0->fVMMR_BoardID[4] = -1;
   param0->fVMMR_BoardID[5] = -1;
   param0->fVMMR_BoardID[6] = -1;
   param0->fVMMR_BoardID[7] = -1;
   param0->fVMMR_BoardID[8] = -1;
   param0->fVMMR_BoardID[9] = -1;
   
   
   param0->fMDPP_BoardID[0] = 1; // this is board ID send by frontends
   param0->fMDPP_BoardID[1] = -1;
   param0->fMDPP_BoardID[2] = -1;
   param0->fMDPP_BoardID[3] = -1;
   param0->fMDPP_BoardID[4] = -1;
   param0->fMDPP_BoardID[5] = -1;
   param0->fMDPP_BoardID[6] = -1;
   param0->fMDPP_BoardID[7] = -1;
   param0->fMDPP_BoardID[8] = -1;
   param0->fMDPP_BoardID[9] = -1;
   
   param0->fMDPP_ReferenceChannel[0] = 15; // TDC reference channel of this board
   param0->fMDPP_ReferenceChannel[1] = -1;
   param0->fMDPP_ReferenceChannel[2] = -1;
   param0->fMDPP_ReferenceChannel[3] = -1;
   param0->fMDPP_ReferenceChannel[4] = -1;
   param0->fMDPP_ReferenceChannel[5] = -1;
   param0->fMDPP_ReferenceChannel[6] = -1;
   param0->fMDPP_ReferenceChannel[7] = -1;
   param0->fMDPP_ReferenceChannel[8] = -1;
   param0->fMDPP_ReferenceChannel[9] = -1;
   
   param0->fMDPP_is16Channels[0] = kTRUE; // specify if we have mdpp-16 (true) or mdpp-32 (false)
   param0->fMDPP_is16Channels[1] = kFALSE;
   param0->fMDPP_is16Channels[2] = kFALSE;
   param0->fMDPP_is16Channels[3] = kFALSE;
   param0->fMDPP_is16Channels[4] = kFALSE;
   param0->fMDPP_is16Channels[5] = kFALSE;
   param0->fMDPP_is16Channels[6] = kFALSE;
   param0->fMDPP_is16Channels[7] = kFALSE;
   param0->fMDPP_is16Channels[8] = kFALSE;
   param0->fMDPP_is16Channels[9] = kFALSE;
   
   param0->fSlowMotion = kFALSE; // stop go4 analysis after each event if true

}
