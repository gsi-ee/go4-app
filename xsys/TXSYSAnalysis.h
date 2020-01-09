
#ifndef TXSYSANALYSIS_H
#define TXSYSANALYSIS_H

#include "TGo4Analysis.h"

class TH1D;
class TGo4MbsEvent;
class TXSYSUnpackEvent;
class TXSYSParameter;

class TXSYSAnalysis : public TGo4Analysis {
   public:
      TXSYSAnalysis();
      TXSYSAnalysis(int argc, char** argv);
      virtual ~TXSYSAnalysis() ;
      virtual Int_t UserPreLoop();
      virtual Int_t UserEventFunc();
      virtual Int_t UserPostLoop();
   private:
      TGo4MbsEvent    *fRawEvent;
      TXSYSUnpackEvent *fUnpackEvent;
      TH1D            *fSize;
      Int_t            fEvents;
      Int_t            fLastEvent;

   ClassDef(TXSYSAnalysis,1)
};
#endif //TXSYSANALYSIS_H



