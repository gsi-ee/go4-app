#ifndef TANALYSIS_H
#define TANALYSIS_H

#include "TGo4Analysis.h"

class TGo4MbsEvent;
class TVupromQFWControl;

class TVupromQFWAnalysis : public TGo4Analysis {
   public:
      TVupromQFWAnalysis();
      TVupromQFWAnalysis(int argc, char** argv);
      virtual ~TVupromQFWAnalysis() ;
      virtual Int_t UserPreLoop();
      virtual Int_t UserEventFunc();
      virtual Int_t UserPostLoop();
   private:
      TGo4MbsEvent*  fMbsEvent;
      TVupromQFWControl*   fCtl;
      Int_t          fEvents;
      Int_t          fLastEvent;

   ClassDef(TVupromQFWAnalysis,1)
};
#endif //TANALYSIS_H
