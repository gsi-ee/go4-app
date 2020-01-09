
#ifndef TXSYSUNPACKPROCESSOR_H
#define TXSYSUNPACKPROCESSOR_H

#include "TGo4EventProcessor.h"
#include "TXSYSUnpackEvent.h"
#include "TXSYSParam.h"

#include "TH1.h"
#include "TGo4WinCond.h"

#define NUM_CONDS 5

class TXSYSUnpackProc : public TGo4EventProcessor {
   public:
      TXSYSUnpackProc() ;
      TXSYSUnpackProc(const char* name);
      virtual ~TXSYSUnpackProc() ;

      /** This method checks if event class is suited for the source */
      virtual Bool_t CheckEventClass(TClass* cl);

      virtual Bool_t BuildEvent(TGo4EventElement* dest);

   private:
      TH1* hRaw[XSYS_CHANNELS];

      TGo4WinCond* cCondition[NUM_CONDS];
      TXSYSParam* fPar;

   ClassDef(TXSYSUnpackProc,1)
};

#endif //TXSYSUNPACKPROCESSOR_H
