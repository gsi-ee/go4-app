

/**
 * Analysis for the GEM CSA tests in December 2019
 * Added mapping to original unpacker step based on TFeb3BasicProc
 * v.01 on 10-Dec-2019 by JAM (j.adamczewski@gsi.de)
 *
 *
 * */



#ifndef TGemCSAANALYSIS_H
#define TGemCSAANALYSIS_H

#include "TGo4Analysis.h"


class TGemCSAAnalysis : public TGo4Analysis  {
   public:
      TGemCSAAnalysis();
      TGemCSAAnalysis(int argc, char** argv);
      virtual ~TGemCSAAnalysis() ;
   private:

   ClassDef(TGemCSAAnalysis,1)
};



#endif //TGemCSAANALYSIS_H



