
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


      Bool_t fSlowMotion; // if true than only process one MBS event and stop.
#ifdef   USE_CSA_MAPPING
      Int_t fSFP [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // sfp that produces data for wire on device
      Int_t fSlave [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // slave that produces data for wire on device
      Int_t fChannel [CSA_MAXCHAMBERS][CSA_MAXWIRES]; // channel that produces data for wire on device
#endif

#ifdef   USE_AWAGS_BEAMMONITOR
      Int_t fBeamMonitorSFP[AWAGS_NUM_ELECTRODES]; // sfp connected to beam charge electrodes, index: [Xlo, Xhigh, Ylo, Yhi]
      Int_t fBeamMonitorSlave[AWAGS_NUM_ELECTRODES]; // slave connected to beam charge electrodes, index: [Xlo, Xhigh, Ylo, Yhi];
      Int_t fBeamMonitorChannel[AWAGS_NUM_ELECTRODES]; // channel connected to beam charge electrodes, index: [Xlo, Xhigh, Ylo, Yhi];
#endif

   ClassDef(TGemCSAMapParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
