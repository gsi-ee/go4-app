#ifndef TQFWANALYSIS_H
#define TQFWANALYSIS_H

#include "TGo4Analysis.h"


class TQFWAnalysis : public TGo4Analysis {
   public:
      TQFWAnalysis();
      TQFWAnalysis(int argc, char** argv);
      virtual ~TQFWAnalysis() ;
   private:

   ClassDef(TQFWAnalysis,1)
};
#endif //TANALYSIS_H
