//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#ifndef TDDASANALYSISPROCESSOR_H
#define TDDASANALYSISPROCESSOR_H

#include "TGo4EventProcessor.h"

#include "TH1.h"
#include "Parameters-ddas.h"
#include "Variables-ddas.h"
#include "Correlator.h"

#include "TDDASRawEvent.h"
class TGo4PolyCond;
class TDDASAnalysisEvent;
class TDDASAnalysisParameter;


class TDDASAnalysisProc: public TGo4EventProcessor
{
public:
  TDDASAnalysisProc();
  TDDASAnalysisProc(const char* name);
  virtual ~TDDASAnalysisProc();

  /** This method checks if event class is suited for the source */
  // virtual Bool_t CheckEventClass(TClass* cl);
  virtual Bool_t BuildEvent(TGo4EventElement* dest);

protected:

  void DdasToEvent();

  void FillHistograms();

  /** we provide one interactive cut for the Correlator here*/
  TGo4PolyCond  *fExampleGate;

  TH1* hFront_icent;
  TH1* hFront_dmax;
  TH1* hFront_imax;
  TH1* hFront_imaxtime;
  TH1* hFront_dmaxtime;
  TH1* hFront_imaxch;
  TH1* hFront_dmaxch;
  TH1* hFront_imult;
  TH1* hFront_dmult;
  TH1* hFront_isum;
  TH1* hFront_dsum;

  TH1* hFront_hiecal[GO4_DDAS_DSSD_STRIPS];
  TH1* hFront_loecal[GO4_DDAS_DSSD_STRIPS];
  TH1* hFront_hienergy[GO4_DDAS_DSSD_STRIPS];
  TH1* hFront_loenergy[GO4_DDAS_DSSD_STRIPS];

  TH1*hBack_icent;
  TH1*hBack_dmax;
  TH1*hBack_imax;
  TH1*hBack_imaxtime;
  TH1*hBack_dmaxtime;
  TH1*hBack_imaxch;
  TH1*hBack_dmaxch;
  TH1*hBack_imult;
  TH1*hBack_dmult;
  TH1*hBack_isum;
  TH1*hBack_dsum;

  TH1*hBack_hiecal[GO4_DDAS_DSSD_STRIPS];
  TH1*hBack_loecal[GO4_DDAS_DSSD_STRIPS];
  TH1*hBack_hienergy[GO4_DDAS_DSSD_STRIPS];
  TH1*hBack_loenergy[GO4_DDAS_DSSD_STRIPS];

private:

  TDDASRawEvent* fInput;
  TDDASAnalysisEvent* fOutput;

  TDDASAnalysisParameter *fAnaPar;
  Correlator fCorrelator;
  betadecay fBdecay;
  betadecayvariables fBdecayVars;

ClassDef(TDDASAnalysisProc,1)
};

#endif //TDDASUNPACKPROCESSOR_H
