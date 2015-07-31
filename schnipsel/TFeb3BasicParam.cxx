//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#include "TFeb3BasicParam.h"

#include "Riostream.h"



//***********************************************************
TFeb3BasicParam::TFeb3BasicParam(const char* name) : TGo4Parameter(name),
    fDoFFT(kFALSE), fSFP_sample(0),fSlave_sample(1), fChannel_sample(0), fSFP_ref(0), fSlave_ref(1), fChannel_ref(1)
{

  fDoFFT=kTRUE;
  fDoFFT=kTRUE;
      fFFTOptions="R2C M";
      fFilterType=0;
      for (int i = 0; i < HitDet_FILTERCOEFFS; ++i)
        {
          fFilterCoeff[i]=0;
        }


}
//***********************************************************
TFeb3BasicParam::~TFeb3BasicParam()
{
}
//***********************************************************

//-----------------------------------------------------------
Int_t TFeb3BasicParam::PrintParameter(Text_t * n, Int_t){
  cout << "Parameter " << GetName()<<":" <<endl;
  cout << " do FFT="<<fDoFFT<<endl;
  cout << " SFP     sample="<<fSFP_sample<<endl;
  cout << " Slave   sample="<<fSlave_sample<< endl;
  cout << " Channel sample="<<fChannel_sample<<endl;
  cout << " SFP     ref="<<fSFP_ref<<endl;
  cout << " Slave   ref="<<fSlave_ref<< endl;
  cout << " Channel ref="<<fChannel_ref<<endl;
  return 0;
}
//-----------------------------------------------------------
Bool_t TFeb3BasicParam::UpdateFrom(TGo4Parameter *pp)
{
  TFeb3BasicParam* from = dynamic_cast<TFeb3BasicParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  cout << "**** TFeb3BasicParam::UpdateFrom ... " << std::endl;

  fSFP_sample = from->fSFP_sample;
  fSlave_sample = from->fSlave_sample;
  fChannel_sample=from->fChannel_sample;
  fSFP_ref = from->fSFP_ref;
  fSlave_ref= from->fSlave_ref;
  fChannel_ref=from->fChannel_ref;

  fDoFFT=from->fDoFFT;
  fFFTOptions=from->fFFTOptions;
  fFilterType=from->fFilterType;

  // here set coefficients appropriate for filter types:
  // please compare https://en.wikipedia.org/wiki/Window_function
  for (int i = 0; i < HitDet_FILTERCOEFFS; ++i)
       {
           fFilterCoeff[i]=0;
       }

  printf("**** Switching pre-FFT filter to: ");
  switch (fFilterType)
   {

     default:    // no filter
     case TFeb3BasicParam::FIL_NONE:
       printf("No Filter (rectangular box)\n");
       fFilterCoeff[0]=1;
       break;

     case TFeb3BasicParam::FIL_VONHANN:    // von Hann (hanning) window
       printf("Von Hann window\n");
       fFilterCoeff[0]=0.5;
       fFilterCoeff[1] = -0.5;
       break;
     case TFeb3BasicParam::FIL_HAMMING:    // Hamming window
       printf("Hamming window\n");
       fFilterCoeff[0]=0.46;
       fFilterCoeff[1] = -0.53;
       break;

     case TFeb3BasicParam::FIL_BLACKMAN:    // plain blackman window:
       printf("Blackman window (exact Blackmann)\n");
       fFilterCoeff[0] = 7938. / 18608.;    // "exact blackmann" coefficients
       fFilterCoeff[1] = -9240. / 18608.;
       fFilterCoeff[2] = 1430. / 18608.;
       break;

     case TFeb3BasicParam::FIL_NUTALL:    // plain nutall window:
       printf("Nutall window\n");
       fFilterCoeff[0] = 0.355768;
       fFilterCoeff[1] = -0.487396;
       fFilterCoeff[2] = 0.144232;
       fFilterCoeff[3] = -0.012604;
       break;

     case TFeb3BasicParam::FIL_BLACKNUTALL: // blackmann-nutall window:
       printf("Blackmann-Nutall window\n");
       fFilterCoeff[0] = 0.3635819;
       fFilterCoeff[1] = -0.4891775;
       fFilterCoeff[2] = 0.1365995;
       fFilterCoeff[3] = -0.0106411;
       break;

     case TFeb3BasicParam::FIL_BLACKHARRIS:
       printf("Blackmann-Harris window\n");
       fFilterCoeff[0] = 0.35875;
       fFilterCoeff[1] = -0.48829;
       fFilterCoeff[2] = 0.14128;
       fFilterCoeff[3] = -0.01168;
       break;

     case TFeb3BasicParam::FIL_SRSFLATTOP:    // Stanford Research Systems flat top filter:
       printf("Flat-Top window (by Stanford Research Systems)\n");
       fFilterCoeff[0] = 1;
       fFilterCoeff[1] = -1.93;
       fFilterCoeff[2] = 1.29;
       fFilterCoeff[3] = -0.388;
       fFilterCoeff[4] = 0.028;
       break;
     case TFeb3BasicParam::FIL_USER: // user defined coefficients, copy from parameter editor:
       printf("User defined cosine window\n");
       for (int i = 0; i < HitDet_FILTERCOEFFS; ++i)
              {
                  fFilterCoeff[i]=from->fFilterCoeff[i];
              }
       break;
   };    // switch
  printf("\tCurrent coefficients:\n");
  for (int i = 0; i < HitDet_FILTERCOEFFS; ++i)
        {
            printf("\t\ta[%d]=%f\n",i,fFilterCoeff[i]);
        }

  std::cout<< std::endl; // flush Qt text stream by this
  return kTRUE;

}
//----------------------------END OF GO4 SOURCE FILE ---------------------
