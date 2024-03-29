
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

  TH2* hMapTrace_Sum[CSA_MAXCHAMBERS];
  TH2* hMapTraceBLR_Sum[CSA_MAXCHAMBERS];
  TH2* hMapTraceFPGA_Sum[CSA_MAXCHAMBERS];

  TH1* hWireProfile[CSA_MAXCHAMBERS];
  TH1* hWireProfileBLR[CSA_MAXCHAMBERS];
  TH1* hWireProfileFPGA[CSA_MAXCHAMBERS];

  TH1* hWireProfile_Sum[CSA_MAXCHAMBERS];
  TH1* hWireProfileBLR_Sum[CSA_MAXCHAMBERS];
  TH1* hWireProfileFPGA_Sum[CSA_MAXCHAMBERS];


  TH2* hBeamPosition; // beam position from charge electrodes, current trace
  TH2* hBeamPositionAcc; // beam position from charge electrodes, accumulated
  TH1* hBeamXAcc; // accumulated X positions of beam
  TH1* hBeamYAcc; // accumulated Y positions of beam
  TH1* hBeamIntensityAcc; // accumulated beam intensity = sum of all electrodes |signals-background|

  TGemCSAMapParam *fParam;

ClassDef(TGemCSAMapProc, 1)
};
#endif //TGemCSAANLPROCESSOR_H
