

#ifndef TGemCSAMAPEVENT_H
#define TGemCSAMAPEVENT_H

#include "TGo4EventElement.h"

#define CSA_MAXCHAMBERS 1
#define CSA_MAXWIRES 256
#define CSA_TRACE_SIZE 1024

class TGemCSAMapEvent : public TGo4EventElement {
   public:
      TGemCSAMapEvent() : TGo4EventElement() {}
      TGemCSAMapEvent(const char* name) : TGo4EventElement(name) {}
      virtual ~TGemCSAMapEvent() {}

      virtual void  Clear(Option_t *t="");

      // here we could pass the mapped traces further, to do JAM 2019
//      std::vector<Double_t> fWireTrace[CSA_MAXCHAMBERS][CSA_MAXWIRES];
//      std::vector<Double_t> fWireTraceBLR[CSA_MAXCHAMBERS][CSA_MAXWIRES];
//      std::vector<Double_t> fWireTraceFPGA[CSA_MAXCHAMBERS][CSA_MAXWIRES];

   ClassDef(TGemCSAMapEvent,1)
};
#endif //TGemCSAANLEVENT_H



