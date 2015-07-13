#ifndef THitDetANALYSIS_H
#define THitDetANALYSIS_H

#include "TGo4Analysis.h"


class THitDetAnalysis : public TGo4Analysis {
   public:
      THitDetAnalysis();
      THitDetAnalysis(int argc, char** argv);
      virtual ~THitDetAnalysis() ;
   private:

   ClassDef(THitDetAnalysis,1)
};
#endif //TANALYSIS_H
