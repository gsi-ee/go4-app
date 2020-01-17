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

#ifndef TDDASAnalysisParameter_H
#define TDDASAnalysisParameter_H


#include "TGo4Parameter.h"
#include "TObjString.h"

class TDDASAnalysisParameter : public TGo4Parameter {
   public:
      TDDASAnalysisParameter();
      TDDASAnalysisParameter(const char* name);
      virtual ~TDDASAnalysisParameter();
      virtual Bool_t UpdateFrom(TGo4Parameter *);

      TObjString fGateName; // name of existing TCutG for gating in correlator
      Double_t fClockMax; // Correlation time
      Double_t fMinImplantTime; // Minimum implant time
      Int_t fRandomFlag; // random flag for correlator


   ClassDef(TDDASAnalysisParameter,1)
};

#endif //TDDASAnalysisParameter_H





