//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#ifndef TDDASFilterParameter_H
#define TDDASFilterParameter_H

#define __ARRAYSIZE__ 5

#include "TGo4Parameter.h"

class TGo4Fitter;

class TDDASFilterParameter : public TGo4Parameter {
   public:
      TDDASFilterParameter();
      TDDASFilterParameter(const char* name);
      virtual ~TDDASFilterParameter();
      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Bool_t fDoFiltering; // if true, output event will be filtered with given dssd thresholds, if false pass through everything
      Int_t fDSSD_Front_Imax; // lower threshold of front dssd imax value for filtering
      Int_t fDSSD_Front_Dmax; // lower threshold of front dssd dmax value for filtering
      Int_t fDSSD_Back_Imax; // lower threshold of back dssd imax value for filtering
      Int_t fDSSD_Back_Dmax; // lower threshold of back dssd dmax value for filtering




   ClassDef(TDDASFilterParameter,1)
};

#endif //TDDASFilterParameter_H





