
#ifndef MAPPAR_H
#define MAPPAR_H



#include "TGo4Parameter.h"
#include "TGemCSAMapEvent.h"

class TGemCSAMapParam : public TGo4Parameter {
   public:
      TGemCSAMapParam();
      TGemCSAMapParam(const char* name);
      virtual ~TGemCSAMapParam();

      void InitCSAMapping();

      virtual Int_t  PrintParameter(Text_t * n, Int_t);
      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Int_t fSFP [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // sfp that produces data for wire on device
      Int_t fSlave [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // slave that produces data for wire on device
      Int_t fChannel [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // channel that produces data for wire on device


   ClassDef(TGemCSAMapParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
