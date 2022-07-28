

/**
 * Analysis for the GEM CSA tests in December 2019
 * Added mapping to original unpacker step based on TFeb3BasicProc
 * v.01 on 10-Dec-2019 by JAM (j.adamczewski@gsi.de)
 *
 *
 * */



#ifndef TAwagsSisANALYSIS_H
#define TAwagsSisANALYSIS_H

#include "TGo4Analysis.h"


class TAwagsSisAnalysis : public TGo4Analysis  {
   public:
      TAwagsSisAnalysis();
      TAwagsSisAnalysis(int argc, char** argv);
      virtual ~TAwagsSisAnalysis() ;
   private:

   ClassDef(TAwagsSisAnalysis,1)
};



#endif //TAwagsSisANALYSIS_H



