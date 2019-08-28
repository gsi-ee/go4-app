#define HitDet_FILTERCOEFFS 5
#define HitDet_MAXBOARDS 10

void set_HitDetRawParam()
{
#ifndef __GO4ANAMACRO__
   std::cout << "Macro set_HitDetRawParam can execute only in analysis" << std::endl;
   return;
#endif
   THitDetRawParam* param0 = (THitDetRawParam*) go4->GetParameter("HitDetRawParam","THitDetRawParam");

   if (param0==0) {
      TGo4Log::Error("Could not find parameter HitDetRawParam of class THitDetRawParam");
      return;
   }

   TGo4Log::Info("Set parameter HitDetRawParam as provided on 28-aug-2019 by JAM");

// uncomment lines below to override comiled/stored defaults:


   for (int i = 0; i < HitDet_MAXBOARDS; ++i)
     {
     param0->fBoardID[i] = -1;
     }

   param0->fBoardID[0] = 42;    // this might be a unique hardware id

   param0->fNumSnapshots = 64;
   param0->fTraceLength = 8;
   param0->fDoFFT=kTRUE;
   param0-> fDoCalibrate=kFALSE;
   param0->fFFTOptions="R2C M";
   param0->fFilterType=0;
   for (int i = 0; i < HitDet_FILTERCOEFFS; ++i)
       {
       param0->fFilterCoeff[i]=0;
       }
   


}
