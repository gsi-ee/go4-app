

#ifndef TAwagsSisMAPEVENT_H
#define TAwagsSisMAPEVENT_H

#include "TGo4EventElement.h"

#define CSA_MAXCHAMBERS 2
#define CSA_MAXWIRES 32
#define CSA_TRACE_SIZE 1024

class TAwagsSisMapEvent : public TGo4EventElement {
   public:
      TAwagsSisMapEvent() : TGo4EventElement() {}
      TAwagsSisMapEvent(const char* name) : TGo4EventElement(name) {}
      virtual ~TAwagsSisMapEvent() {}

      virtual void  Clear(Option_t *t="");

      // here we could pass the mapped traces further, to do JAM 2019
//      std::vector<Double_t> fWireTrace[CSA_MAXCHAMBERS][CSA_MAXWIRES];

   ClassDef(TAwagsSisMapEvent,1)
};
#endif //TAwagsSisANLEVENT_H



