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
#ifndef AWAGSISPAR_H
#define AWAGSISPAR_H


#include "TGo4Parameter.h"

class TAwagsSisParam : public TGo4Parameter {
   public:
      TAwagsSisParam();
      TAwagsSisParam(const char* name);
      virtual ~TAwagsSisParam();

      Int_t fNumEventsPerSpill; // estimated number of MBS event for each spill - for display
      Int_t fMaxSpillEvent;    // maximum MBS event index for each spill. After this we are out of spill for sure.
      Bool_t fMapSpills; // if true copy spill traces to output event for mapping step.
      Int_t fSpillTriggerSFP; // sfp for spill trigger decision channel. set to -1 to use average of all channels
      UInt_t fSpillTriggerSlave; // slave for spill trigger decision channel
      UInt_t fSpillTriggerChan; // slave for spill trigger decision channel

   ClassDef(TAwagsSisParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
