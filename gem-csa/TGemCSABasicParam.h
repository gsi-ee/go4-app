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

class TGemCSABasicParam : public TGo4Parameter {
   public:
      TGemCSABasicParam();
      TGemCSABasicParam(const char* name);
      virtual ~TGemCSABasicParam();


       Bool_t fDoBaselineFits; // if true, polynomial fit is done for traces within the region
       Bool_t fMapTraces; // if true, full traces are mapped to the output event
      // Bool_t fSlowMotion; // if true than only process one MBS event and stop. TODO: move to second step!

   ClassDef(TGemCSABasicParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
