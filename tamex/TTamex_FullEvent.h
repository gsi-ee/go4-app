// $Id: TTamex_FullEvent.h 2627 2019-10-01 08:02:45Z linev $
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

#ifndef TTAMEXXEVENT_H
#define TTAMEXXEVENT_H

#include "TGo4EventElement.h"

#define MAX_CHA_AN    64    


class TTamex_FullEvent : public TGo4EventElement {
   public:
      TTamex_FullEvent();
      TTamex_FullEvent(const char* name);
      virtual ~TTamex_FullEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");
      
      Double_t fTimeDiff[MAX_CHA_AN];
      // NOTE: any new class member variable has to be cleared to "unfilled value" in TTamex_FullEvent.cxx - Clear() !!!!!

     
   ClassDef(TTamex_FullEvent,1)
};
#endif //TEVENT_H



