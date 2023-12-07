

#ifndef TGemCSAMAPEVENT_H
#define TGemCSAMAPEVENT_H

#include "TGo4EventElement.h"


// enable this to map channels to wires of a chamber
//#define USE_CSA_MAPPING 1

// enable this to map certain channels for beam position pickup detectors
#define USE_AWAGS_BEAMMONITOR 1

#define CSA_MAXCHAMBERS 1
#define CSA_MAXWIRES 256
#define CSA_TRACE_SIZE 1024

// number of beam position pickup electrodes in use
#define AWAGS_NUM_ELECTRODES 4

// granularity of X/Y beamposition display
#define AWAGS_BEAMPOS_BINS 50
// granularity of X/Y beamposition display, projected
#define AWAGS_BEAMPOS_BINS_FINE 500
// display range (+/-)of beam position histogram
#define AWAGS_BEAMPOS_RANGE 1

class TGemCSAMapEvent : public TGo4EventElement {
   public:
      TGemCSAMapEvent() : TGo4EventElement() {}
      TGemCSAMapEvent(const char* name) : TGo4EventElement(name) {}
      virtual ~TGemCSAMapEvent() {}

      virtual void  Clear(Option_t *t="");

      // here we could pass the mapped traces further, to do JAM 2019
//      std::vector<Double_t> fWireTrace[CSA_MAXCHAMBERS][CSA_MAXWIRES];
//      std::vector<Double_t> fWireTraceBLR[CSA_MAXCHAMBERS][CSA_MAXWIRES];
//      std::vector<Double_t> fWireTraceFPGA[CSA_MAXCHAMBERS][CSA_MAXWIRES];

   ClassDef(TGemCSAMapEvent,1)
};
#endif //TGemCSAANLEVENT_H



