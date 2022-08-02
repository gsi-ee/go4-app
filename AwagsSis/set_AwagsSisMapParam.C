// written by macro saveparam.C at Tue Aug  2 08:20:54 2022
// JAM 02-08-2022: put here Svens new mapping
void set_AwagsSisMapParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_AwagsSisMapParam can execute only in analysis" << std::endl;
   return;
#endif
   TAwagsSisMapParam* param1 = (TAwagsSisMapParam*) go4->GetParameter("AwagsSisMapParam","TAwagsSisMapParam");

   if (!param1) {
      TGo4Log::Error("Could not find parameter AwagsSisMapParam of class TAwagsSisMapParam");
      return;
   }

   TGo4Log::Info("Set parameter AwagsSisMapParam as saved at Tue Aug  2 08:20:54 2022");

   // do not overwrite interactive slowmotion switches:
   //param1->fSlowMotion = kFALSE;
   //param1->fStopAtEachSpill = kFALSE;
   // wire mappping below:
   param1->fMaxChamber = 1;
   param1->fMaxWire[0] = 31;

   // index: [chamber][wire/pin]
  for (Int_t w = 0; w < 31; ++w)
  {
    param1->fSFP[0][w] = 0;    // one detector at sfp 0
  }

  // index: [chamber][wire/pin]
  param1->fSlave[0][0] = 1;
  param1->fSlave[0][1] = 0;
  param1->fSlave[0][2] = 0;
  param1->fSlave[0][3] = 0;
  param1->fSlave[0][4] = 0;
  param1->fSlave[0][5] = 0;
  param1->fSlave[0][6] = 0;
  param1->fSlave[0][7] = 0;
  param1->fSlave[0][8] = 0;
  param1->fSlave[0][9] = 0;
  param1->fSlave[0][10] = 0;
  param1->fSlave[0][11] = 0;
  param1->fSlave[0][12] = 0;
  param1->fSlave[0][13] = 0;
  param1->fSlave[0][14] = 0;
  param1->fSlave[0][15] = 0;
  param1->fSlave[0][16] = 0;
  param1->fSlave[0][17] = 1;
  param1->fSlave[0][18] = 1;
  param1->fSlave[0][19] = 1;
  param1->fSlave[0][20] = 1;
  param1->fSlave[0][21] = 1;
  param1->fSlave[0][22] = 1;
  param1->fSlave[0][23] = 1;
  param1->fSlave[0][24] = 1;
  param1->fSlave[0][25] = 1;
  param1->fSlave[0][26] = 1;
  param1->fSlave[0][27] = 1;
  param1->fSlave[0][28] = 1;
  param1->fSlave[0][29] = 1;
  param1->fSlave[0][30] = 1;
  param1->fSlave[0][31] = 1;

  param1->fChannel[0][0] = 0;//1;
  param1->fChannel[0][1] = 15;//16;
  param1->fChannel[0][2] = 14;//15;
  param1->fChannel[0][3] = 13; //14;
  param1->fChannel[0][4] = 12;// 13;
  param1->fChannel[0][5] = 11;//12;
  param1->fChannel[0][6] = 10;//11;
  param1->fChannel[0][7] = 9;//10;
  param1->fChannel[0][8] = 8;//9;
  param1->fChannel[0][9] = 5;//6;
  param1->fChannel[0][10] = 6;//7;
  param1->fChannel[0][11] = 7;//8;
  param1->fChannel[0][12] = 4;//5;
  param1->fChannel[0][13] = 3;//4;
  param1->fChannel[0][14] = 2;//3;
  param1->fChannel[0][15] = 1;//2;
  param1->fChannel[0][16] = 0;//1;
  param1->fChannel[0][17] = 11;//12;
  param1->fChannel[0][18] = 15;//16;
  param1->fChannel[0][19] = 14;//15;
  param1->fChannel[0][20] = 13;//14;
  param1->fChannel[0][21] = 12;//13;
  param1->fChannel[0][22] = 1;//2;
  param1->fChannel[0][23] = 10;//11;
  param1->fChannel[0][24] = 3;//4;
  param1->fChannel[0][25] = 8;//9;
  param1->fChannel[0][26] = 7;//8;
  param1->fChannel[0][27] = 6;//7;
  param1->fChannel[0][28] = 5;//6;
  param1->fChannel[0][29] = 4;//5;
  param1->fChannel[0][30] = 9;//10;
  param1->fChannel[0][31] = 2;//3;
                                              
                                              
                                              


}
