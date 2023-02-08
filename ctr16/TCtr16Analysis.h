#ifndef TCtr16ANALYSIS_H
#define TCtr16ANALYSIS_H

#include "TGo4Analysis.h"


class TCtr16Analysis : public TGo4Analysis {
   public:
      TCtr16Analysis();
      TCtr16Analysis(int argc, char** argv);
      virtual ~TCtr16Analysis() ;
   private:

   ClassDef(TCtr16Analysis,1)
};
#endif //TANALYSIS_H
