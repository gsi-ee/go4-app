
#ifndef TGemCSAANLPROCESSOR_H
#define TGemCSAANLPROCESSOR_H

#include "TGo4EventProcessor.h"

#include "TGemCSAMapEvent.h"

class TGemCSAMapParam;
class TGemCSAMapEvent;

class TGemCSAMapProc: public TGo4EventProcessor
{
public:
  TGemCSAMapProc();
  TGemCSAMapProc(const char * name);
  virtual ~TGemCSAMapProc();

  virtual Bool_t BuildEvent(TGo4EventElement* dest);

  void ResetTraces();

  TH1* hWireTraces[CSA_MAXCHAMBERS][CSA_MAXWIRES];
  TH1* hWireTracesBLR[CSA_MAXCHAMBERS][CSA_MAXWIRES];
  TH1* hWireTracesFPGA[CSA_MAXCHAMBERS][CSA_MAXWIRES];

  /** overview of traces vs wires*/
  TH2* hMapTrace[CSA_MAXCHAMBERS];
  TH2* hMapTraceBLR[CSA_MAXCHAMBERS];
  TH2* hMapTraceFPGA[CSA_MAXCHAMBERS];

  TGemCSAMapParam *fParam;

ClassDef(TGemCSAMapProc, 1)
};
#endif //TGemCSAANLPROCESSOR_H
