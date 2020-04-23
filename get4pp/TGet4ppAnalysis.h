#ifndef TGet4ppANALYSIS_H
#define TGet4ppANALYSIS_H

#include "TGo4Analysis.h"


class TGet4ppAnalysis : public TGo4Analysis {
   public:
      TGet4ppAnalysis();
      TGet4ppAnalysis(int argc, char** argv);
      virtual ~TGet4ppAnalysis() ;
   private:

   ClassDef(TGet4ppAnalysis,1)
};
#endif //TANALYSIS_H
