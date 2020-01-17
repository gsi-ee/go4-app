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

#ifndef TDDASANALYSIS_H
#define TDDASANALYSIS_H

#include "TGo4Analysis.h"

class TH1D;
class TDDASRawEvent;
class TDDASAnalysisEvent;
class TDDASFilterParameter;

class TDDASAnalysis : public TGo4Analysis {
   public:
      TDDASAnalysis();
      TDDASAnalysis(int argc, char** argv);
      virtual ~TDDASAnalysis() ;
      virtual Int_t UserPreLoop();
      virtual Int_t UserEventFunc();
      virtual Int_t UserPostLoop();
   private:
      TDDASRawEvent    *fRawEvent;
      TDDASAnalysisEvent *fAnalysisEvent;
      TH1D            *fSize;
      TDDASFilterParameter   *fPar;
      Int_t            fEvents;
      Int_t            fLastEvent;

   ClassDef(TDDASAnalysis,1)
};
#endif //TDDASANALYSIS_H



