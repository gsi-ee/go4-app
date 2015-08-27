// $Id: THitDetRawParam.cxx 557 2010-01-27 15:11:43Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "THitDetRawParam.h"
//#include "Riostream.h"
#include "TGo4Log.h"

THitDetRawParam::THitDetRawParam(const char* name) :
    TGo4Parameter(name)
{
  InitBoardMapping();
}

void THitDetRawParam::InitBoardMapping()
{
  // init to non valid here:
  for (int i = 0; i < HitDet_MAXBOARDS; ++i)
  {
    fBoardID[i] = -1;
  }

  fBoardID[0] = 42;    // this might be a unique hardware id

  fNumSnapshots = 64;
  fTraceLength = 8; //HitDet_TRACEBINS;
  fDoFFT=kTRUE;
  fDoCalibrate=kFALSE;
  fFFTOptions="R2C M";
  fFilterType=0;
  for (int i = 0; i < HitDet_FILTERCOEFFS; ++i)
    {
      fFilterCoeff[i]=0;
    }
}

Bool_t THitDetRawParam::SetConfigBoards()
{
  THitDetRawEvent::fgConfigHitDetBoards.clear();
  for (int i = 0; i < HitDet_MAXBOARDS; ++i)
  {
    Int_t bid = fBoardID[i];
    if (bid < 0)
      continue;
    THitDetRawEvent::fgConfigHitDetBoards.push_back(bid);
    TGo4Log::Info("THitDetRawParam::SetConfigBoards registers board unique id %u configured at index %d \n", bid, i);
  }
  return kTRUE;
}

Bool_t THitDetRawParam::UpdateFrom(TGo4Parameter *pp)
{
  THitDetRawParam* from = dynamic_cast<THitDetRawParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  cout << "**** THitDetRawParam::UpdateFrom ... " << std::endl;

  fNumSnapshots = from->fNumSnapshots;
  fTraceLength = from->fTraceLength;
  fDoFFT=from->fDoFFT;
  fDoCalibrate=from->fDoCalibrate;
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
     case THitDetRawParam::FIL_NONE:
       printf("No Filter (rectangular box)\n");
       fFilterCoeff[0]=1;
       break;

     case THitDetRawParam::FIL_VONHANN:    // von Hann (hanning) window
       printf("Von Hann window\n");
       fFilterCoeff[0]=0.5;
       fFilterCoeff[1] = -0.5;
       break;
     case THitDetRawParam::FIL_HAMMING:    // Hamming window
       printf("Hamming window\n");
       fFilterCoeff[0]=0.46;
       fFilterCoeff[1] = -0.53;
       break;

     case THitDetRawParam::FIL_BLACKMAN:    // plain blackman window:
       printf("Blackman window (exact Blackmann)\n");
       fFilterCoeff[0] = 7938. / 18608.;    // "exact blackmann" coefficients
       fFilterCoeff[1] = -9240. / 18608.;
       fFilterCoeff[2] = 1430. / 18608.;
       break;

     case THitDetRawParam::FIL_NUTALL:    // plain nutall window:
       printf("Nutall window\n");
       fFilterCoeff[0] = 0.355768;
       fFilterCoeff[1] = -0.487396;
       fFilterCoeff[2] = 0.144232;
       fFilterCoeff[3] = -0.012604;
       break;

     case THitDetRawParam::FIL_BLACKNUTALL: // blackmann-nutall window:
       printf("Blackmann-Nutall window\n");
       fFilterCoeff[0] = 0.3635819;
       fFilterCoeff[1] = -0.4891775;
       fFilterCoeff[2] = 0.1365995;
       fFilterCoeff[3] = -0.0106411;
       break;

     case THitDetRawParam::FIL_BLACKHARRIS:
       printf("Blackmann-Harris window\n");
       fFilterCoeff[0] = 0.35875;
       fFilterCoeff[1] = -0.48829;
       fFilterCoeff[2] = 0.14128;
       fFilterCoeff[3] = -0.01168;
       break;

     case THitDetRawParam::FIL_SRSFLATTOP:    // Stanford Research Systems flat top filter:
       printf("Flat-Top window (by Stanford Research Systems)\n");
       fFilterCoeff[0] = 1;
       fFilterCoeff[1] = -1.93;
       fFilterCoeff[2] = 1.29;
       fFilterCoeff[3] = -0.388;
       fFilterCoeff[4] = 0.028;
       break;
     case THitDetRawParam::FIL_USER: // user defined coefficients, copy from parameter editor:
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

  SetConfigBoards();
  std::cout<< std::endl; // flush Qt text stream by this
  return kTRUE;

}
