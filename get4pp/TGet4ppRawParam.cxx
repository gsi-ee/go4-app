// $Id: TGet4ppRawParam.cxx 557 2010-01-27 15:11:43Z linev $
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

#include "TGet4ppRawParam.h"
//#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TGet4ppRawParam::TGet4ppRawParam(const char* name) :
    TGo4Parameter(name)
{
  InitBoardMapping();
}

void TGet4ppRawParam::InitBoardMapping()
{
  // init to non valid here:
  for (int i = 0; i < Get4pp_MAXBOARDS; ++i)
  {
    fBoardID[i] = -1;
  }

  fBoardID[0] = 42;    // this might be a unique hardware id

//  fNumSnapshots = 64;
//  fTraceLength = 8; //Get4pp_TRACEBINS;
//  fDoFFT=kTRUE;
//  fDoCalibrate=kFALSE;
//  fFFTOptions="R2C M";
//  fFilterType=0;
//  for (int i = 0; i < Get4pp_FILTERCOEFFS; ++i)
//    {
//      fFilterCoeff[i]=0;
//    }
//
//  fDoSinusFit=kFALSE;
//  fSinusAmp = 800.0;
//  fSinusPeriod = 10.0;
//  fSinusBaseline=0.0;
//  fSinusPhase=0.0;
//  fSinusTolerance = 0.1;
  fSlowMotion=kFALSE;

}

Bool_t TGet4ppRawParam::SetConfigBoards()
{
  TGet4ppRawEvent::fgConfigGet4ppBoards.clear();
  for (int i = 0; i < Get4pp_MAXBOARDS; ++i)
  {
    Int_t bid = fBoardID[i];
    if (bid < 0)
      continue;
    TGet4ppRawEvent::fgConfigGet4ppBoards.push_back(bid);
    TGo4Log::Info("TGet4ppRawParam::SetConfigBoards registers board unique id %u configured at index %d \n", bid, i);
  }
  return kTRUE;
}

Bool_t TGet4ppRawParam::UpdateFrom(TGo4Parameter *pp)
{
  TGet4ppRawParam* from = dynamic_cast<TGet4ppRawParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  cout << "**** TGet4ppRawParam::UpdateFrom ... " << std::endl;

//  fNumSnapshots = from->fNumSnapshots;
//  fTraceLength = from->fTraceLength;
//  fDoFFT=from->fDoFFT;
//  fDoCalibrate=from->fDoCalibrate;
//  fFFTOptions=from->fFFTOptions;
//  fFilterType=from->fFilterType;
//
//
//  fDoSinusFit=from->fDoSinusFit;
//  fSinusAmp = from->fSinusAmp;
//  fSinusPeriod = from->fSinusPeriod;
//  fSinusBaseline=from->fSinusBaseline;
//  fSinusPhase=from->fSinusPhase;
//  fSinusTolerance=from->fSinusTolerance;
  fSlowMotion=from->fSlowMotion;
  // here set coefficients appropriate for filter types:
  // please compare https://en.wikipedia.org/wiki/Window_function
//  for (int i = 0; i < Get4pp_FILTERCOEFFS; ++i)
//       {
//           fFilterCoeff[i]=0;
//       }
//
//  printf("**** Switching pre-FFT filter to: ");
//  switch (fFilterType)
//   {
//
//     default:    // no filter
//     case TGet4ppRawParam::FIL_NONE:
//       printf("No Filter (rectangular box)\n");
//       fFilterCoeff[0]=1;
//       break;
//
//     case TGet4ppRawParam::FIL_VONHANN:    // von Hann (hanning) window
//       printf("Von Hann window\n");
//       fFilterCoeff[0]=0.5;
//       fFilterCoeff[1] = -0.5;
//       break;
//     case TGet4ppRawParam::FIL_HAMMING:    // Hamming window
//       printf("Hamming window\n");
//       fFilterCoeff[0]=0.46;
//       fFilterCoeff[1] = -0.53;
//       break;
//
//     case TGet4ppRawParam::FIL_BLACKMAN:    // plain blackman window:
//       printf("Blackman window (exact Blackmann)\n");
//       fFilterCoeff[0] = 7938. / 18608.;    // "exact blackmann" coefficients
//       fFilterCoeff[1] = -9240. / 18608.;
//       fFilterCoeff[2] = 1430. / 18608.;
//       break;
//
//     case TGet4ppRawParam::FIL_NUTALL:    // plain nutall window:
//       printf("Nutall window\n");
//       fFilterCoeff[0] = 0.355768;
//       fFilterCoeff[1] = -0.487396;
//       fFilterCoeff[2] = 0.144232;
//       fFilterCoeff[3] = -0.012604;
//       break;
//
//     case TGet4ppRawParam::FIL_BLACKNUTALL: // blackmann-nutall window:
//       printf("Blackmann-Nutall window\n");
//       fFilterCoeff[0] = 0.3635819;
//       fFilterCoeff[1] = -0.4891775;
//       fFilterCoeff[2] = 0.1365995;
//       fFilterCoeff[3] = -0.0106411;
//       break;
//
//     case TGet4ppRawParam::FIL_BLACKHARRIS:
//       printf("Blackmann-Harris window\n");
//       fFilterCoeff[0] = 0.35875;
//       fFilterCoeff[1] = -0.48829;
//       fFilterCoeff[2] = 0.14128;
//       fFilterCoeff[3] = -0.01168;
//       break;
//
//     case TGet4ppRawParam::FIL_SRSFLATTOP:    // Stanford Research Systems flat top filter:
//       printf("Flat-Top window (by Stanford Research Systems)\n");
//       fFilterCoeff[0] = 1;
//       fFilterCoeff[1] = -1.93;
//       fFilterCoeff[2] = 1.29;
//       fFilterCoeff[3] = -0.388;
//       fFilterCoeff[4] = 0.028;
//       break;
//     case TGet4ppRawParam::FIL_USER: // user defined coefficients, copy from parameter editor:
//       printf("User defined cosine window\n");
//       for (int i = 0; i < Get4pp_FILTERCOEFFS; ++i)
//              {
//                  fFilterCoeff[i]=from->fFilterCoeff[i];
//              }
//       break;
//   };    // switch
//  printf("\tCurrent coefficients:\n");
//  for (int i = 0; i < Get4pp_FILTERCOEFFS; ++i)
//        {
//            printf("\t\ta[%d]=%f\n",i,fFilterCoeff[i]);
//        }

  SetConfigBoards();
  std::cout<< std::endl; // flush Qt text stream by this
  return kTRUE;

}
