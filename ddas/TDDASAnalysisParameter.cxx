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

#include "TDDASAnalysisParameter.h"

#include "Riostream.h"

#include "TGo4Log.h"

TDDASAnalysisParameter::TDDASAnalysisParameter() :
   TGo4Parameter("DDASAnalysisParameter")
{
  fUseGate=kTRUE;
  fGateName="TheDefaultCut";
  // JAM 1-2020: these defaults were taken from old example loopAnalysis_fireside:
  fClockMax=150;
  fMinImplantTime=150;
  fRandomFlag=0;
}

TDDASAnalysisParameter::TDDASAnalysisParameter(const char* name) :
   TGo4Parameter(name)
{
  fUseGate=kTRUE;
  fGateName="TheDefaultCut";
    // JAM 1-2020: these defaults were taken from old example loopAnalysis_fireside:
  fClockMax=150;
  fMinImplantTime=150;
  fRandomFlag=0;

}

TDDASAnalysisParameter::~TDDASAnalysisParameter()
{

}


Bool_t TDDASAnalysisParameter::UpdateFrom(TGo4Parameter *source)
{
   TDDASAnalysisParameter* from = dynamic_cast<TDDASAnalysisParameter*> (source);
   if (from==0) {
      TGo4Log::Error("Wrong parameter class: %s", source->ClassName());
      return kFALSE;
   }
   fUseGate=from->fUseGate;
   fGateName=from->fGateName;
   fClockMax=from->fClockMax;
   fMinImplantTime=from->fMinImplantTime;
   fRandomFlag=from->fRandomFlag;
   return kTRUE;
}
