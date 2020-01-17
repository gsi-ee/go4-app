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

#ifndef TDDASEVENT_H
#define TDDASEVENT_H

#include "TGo4EventElement.h"
#include "Parameters-ddas.h"

class TDDASAnalysisEvent : public TGo4EventElement {
   public:
      TDDASAnalysisEvent() ;
      TDDASAnalysisEvent(const char* name) ;
      virtual ~TDDASAnalysisEvent() ;

      /**
       * Method called by the event owner (analysis step) to clear the
       * event element.
       */
      void Clear(Option_t *t="");

      /** Keep output data structure as defined in original ddas analysis:*/
     rootdataout fData;

   ClassDef(TDDASAnalysisEvent,1)
};
#endif //TDDASEVENT_H



