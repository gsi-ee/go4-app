

#ifndef TGemCSAEVENT_H
#define TGemCSAEVENT_H


#define MAX_SFP          4
#define MAX_SLAVE        4
#define N_CHA            16

#include "TGo4EventElement.h"
#include <vector>

class TGemCSABasicEvent : public TGo4EventElement {
   public:
      TGemCSABasicEvent() : TGo4EventElement() {}
      TGemCSABasicEvent(const char* name) : TGo4EventElement(name) {}
      virtual ~TGemCSABasicEvent() {}

      /**
       * Method called by the event owner (analysis step) to clear the
       * event element.
       */
      virtual void Clear(Option_t *t="");

      std::vector<Double_t> fTrace[MAX_SFP][MAX_SLAVE][N_CHA];
      std::vector<Double_t> fTraceBLR[MAX_SFP][MAX_SLAVE][N_CHA];
      std::vector<Double_t> fTraceFPGA[MAX_SFP][MAX_SLAVE][N_CHA];

      /* JAM 6-12-23: new members to evaluate beam position display*/

      Double_t fSignal[MAX_SFP][MAX_SLAVE][N_CHA]; // average height of signal region for each channel trace , defined by condition
      Double_t fBackground[MAX_SFP][MAX_SLAVE][N_CHA]; //average height of background region for each channel trace , defined by condition

   ClassDef(TGemCSABasicEvent,1)
};
#endif //TGemCSAEVENT_H



