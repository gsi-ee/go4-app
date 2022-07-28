

#ifndef TAwagsSisEVENT_H
#define TAwagsSisEVENT_H


#define MAX_SFP           4
#define MAX_SLAVE        16
#define N_CHA            16

#include "TGo4EventElement.h"
#include <vector>

/* JAM 27-jul-2022: the output event for awags contains data of complete spill only.
 * This means that mapping in second analysis step is always done spill-wise.
 * This is flagged by the IsValid state of this event object.*/

class TAwagsSisBasicEvent : public TGo4EventElement {
   public:
      TAwagsSisBasicEvent() : TGo4EventElement() {Clear();}
      TAwagsSisBasicEvent(const char* name) : TGo4EventElement(name) {Clear();}
      virtual ~TAwagsSisBasicEvent() {}

      /**
       * Method called by the event owner (analysis step) to clear the
       * event element.
       */
      virtual void Clear(Option_t *t="");
      /* stitched trace data for each spill*/
      std::vector<Double_t> fSpillTrace[MAX_SFP][MAX_SLAVE][N_CHA];

      /* trend of charge evaluation between acquired MBS febex events.
       * size of this vector should be below #mbs events per spill */
      std::vector<Double_t> fChargeTrend[MAX_SFP][MAX_SLAVE][N_CHA];

   ClassDef(TAwagsSisBasicEvent,1)
};
#endif //TAwagsSisEVENT_H



