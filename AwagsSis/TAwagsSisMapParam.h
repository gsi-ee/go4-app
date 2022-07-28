
#ifndef MAPPAR_H
#define MAPPAR_H



#include "TGo4Parameter.h"
#include "TAwagsSisMapEvent.h"

class TAwagsSisMapParam : public TGo4Parameter {
   public:
      TAwagsSisMapParam();
      TAwagsSisMapParam(const char* name);
      virtual ~TAwagsSisMapParam();

      void InitAwagsMapping();

      virtual Int_t  PrintParameter(Text_t * n, Int_t);
      virtual Bool_t UpdateFrom(TGo4Parameter *);
      Bool_t fSlowMotion; // if true than only process one MBS event and stop.
      Int_t fSFP [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // sfp that produces data for wire on device
      Int_t fSlave [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // slave that produces data for wire on device
      Int_t fChannel [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // channel that produces data for wire on device


   ClassDef(TAwagsSisMapParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
