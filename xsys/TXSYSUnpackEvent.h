
#ifndef TXSYSEVENT_H
#define TXSYSEVENT_H

#define XSYS_CHANNELS 16
#define XSYS_BINS 0x1000 // 12 bit spectrum

#include "TGo4EventElement.h"

class TXSYSUnpackEvent : public TGo4EventElement {
   public:
      TXSYSUnpackEvent() ;
      TXSYSUnpackEvent(const char* name) ;
      virtual ~TXSYSUnpackEvent() ;

      /**
       * Method called by the event owner (analysis step) to clear the
       * event element.
       */
      void Clear(Option_t *t="");

      /* The spectrum data (12bit)*/
      UShort_t fSpectrum[XSYS_CHANNELS];

      /* The spectrum ID number (4bit)*/
      UChar_t fSpecID[XSYS_CHANNELS];

      Int_t fLen;
      Short_t fChannel;
      Short_t fFlag;

   ClassDef(TXSYSUnpackEvent,1)
};
#endif //TXSYSEVENT_H



