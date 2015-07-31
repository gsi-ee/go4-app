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
#ifndef SPAR_H
#define SPAR_H



#include "TGo4Parameter.h"

#define HitDet_FILTERCOEFFS 5

class TFeb3BasicParam : public TGo4Parameter {
 public:
  enum FilterTypes
     {
       FIL_NONE = 0,
       FIL_VONHANN = 1,
       FIL_HAMMING = 2,
       FIL_BLACKMAN = 3,
       FIL_NUTALL = 4,
       FIL_BLACKNUTALL = 5,
       FIL_BLACKHARRIS = 6,
       FIL_SRSFLATTOP = 7,
       FIL_USER = 8
     };













      TFeb3BasicParam(const char* name=0);
      virtual ~TFeb3BasicParam();
      virtual Int_t  PrintParameter(Text_t * n, Int_t);
      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Bool_t fDoFFT;  // enable FFT of trace samples
      Int_t  fSFP_sample;  // sfp id of trace sample
      Int_t  fSlave_sample;  // febex id of trace sample
      Int_t  fChannel_sample;  // channel number of trace sample
      Int_t  fSFP_ref;  // sfp id of reference trace
      Int_t  fSlave_ref;  // febex id of reference trace
      Int_t  fChannel_ref;  // channel number of reference trace


      TString fFFTOptions; // ROOT fft option string
      Int_t fFilterType; // pre-FFT filter type 0-none 1-VonHann 2-Hamming 3-Blackmann 4-Nutall 5-BlackmannNutall 6-Blackmann-Harris 7-FlatTop 8-User
      Double_t fFilterCoeff[HitDet_FILTERCOEFFS]; // coefficient for generalized cosine window filters



    ClassDef(TFeb3BasicParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
