#ifndef TNectarANALYSIS_H
#define TNectarANALYSIS_H

#include "TGo4Analysis.h"


class TNectarAnalysis : public TGo4Analysis {
   public:
      TNectarAnalysis();
      TNectarAnalysis(int argc, char** argv);
      virtual ~TNectarAnalysis() ;
   private:

   ClassDef(TNectarAnalysis,1)
};
#endif //TANALYSIS_H
