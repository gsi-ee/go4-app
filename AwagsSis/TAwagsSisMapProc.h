
#ifndef TAwagsSisANLPROCESSOR_H
#define TAwagsSisANLPROCESSOR_H

#include "TGo4EventProcessor.h"

#include "TAwagsSisMapEvent.h"
#include "TAwagsSisMapParam.h"

class TAwagsSisMapEvent;

class TAwagsSisMapProc: public TGo4EventProcessor
{
public:
  TAwagsSisMapProc();
  TAwagsSisMapProc(const char * name);
  virtual ~TAwagsSisMapProc();

  virtual Bool_t BuildEvent(TGo4EventElement* dest);


protected:

  void ResetTraces();

  Int_t GetNumWires(Int_t dev)
  {
    Int_t numwires=fParam->fMaxWire[dev];
    if(numwires>CSA_MAXWIRES && numwires<0) numwires=CSA_MAXWIRES;
    return numwires;
  }

  Int_t GetNumChambers()
  {
    Int_t numchambers=fParam->fMaxChamber;
      if(numchambers>CSA_MAXCHAMBERS && numchambers<0) numchambers=CSA_MAXCHAMBERS;
      return numchambers;
  }

private:

  TH1* hWireTraces[CSA_MAXCHAMBERS][CSA_MAXWIRES];

  TH1* hWireSpillCharge[CSA_MAXCHAMBERS][CSA_MAXWIRES];

  /** overview of traces vs wires*/
  TH2* hMapTrace[CSA_MAXCHAMBERS];
  TH2* hMapTrace_Sum[CSA_MAXCHAMBERS];

  TH1* hWireProfile[CSA_MAXCHAMBERS];
  TH1* hWireProfile_Sum[CSA_MAXCHAMBERS];

  TH2* hMapSpillCharge[CSA_MAXCHAMBERS];
   TH2* hMapSpillCharge_Sum[CSA_MAXCHAMBERS];

   TH1* hWireChargeProfile[CSA_MAXCHAMBERS];
   TH1* hWireChargeProfile_Sum[CSA_MAXCHAMBERS];

   TH1* hSampleChargeProfile[CSA_MAXCHAMBERS];
   TH1* hSampleChargeProfile_Sum[CSA_MAXCHAMBERS];


  TAwagsSisMapParam *fParam;

ClassDef(TAwagsSisMapProc, 1)
};
#endif //TAwagsSisANLPROCESSOR_H
