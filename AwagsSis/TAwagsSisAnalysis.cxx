// $Id: TAwagsSisAnalysis.cxx 2627 2019-10-01 08:02:45Z linev $
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

#include "TAwagsSisAnalysis.h"

#include <stdlib.h>

#include "TH1.h"
#include "TFile.h"
#include "TSystem.h"

extern "C" {
   #include "s_filhe_swap.h"
   #include "s_bufhe_swap.h"
   #include "f_ut_utime.h"
}

#include "TGo4Log.h"
#include "TGo4Fitter.h"
#include "TGo4FitterEnvelope.h"
#include "TGo4AnalysisStep.h"
#include "TGo4StepFactory.h"
#include "TGo4Version.h"

#include "Go4EventServer.h"

//***********************************************************
TAwagsSisAnalysis::TAwagsSisAnalysis() :
   TGo4Analysis()
{
   TGo4Log::Error("Wrong constructor TAwagsSisAnalysis()!");
}

//***********************************************************
// this constructor is called by go4analysis executable
TAwagsSisAnalysis::TAwagsSisAnalysis(int argc, char** argv) :
   TGo4Analysis(argc, argv)

{
   if (!TGo4Version::CheckVersion(__GO4BUILDVERSION__)) {
      TGo4Log::Error("Go4 version mismatch");
      exit(-1);
   }

   TGo4Log::Info("TAwagsSisAnalysis: Create %s", GetName());

   TString kind, input, out1, out2;

// Create step 1 Unpack.
   TGo4StepFactory* factory1 = new TGo4StepFactory("UnpackFactory");
   factory1->DefEventProcessor("AwagsSisProc", "TAwagsSisProc");// object name, class name
   factory1->DefOutputEvent("BasicEvent", "TAwagsSisBasicEvent"); // object name, class name
   TGo4AnalysisStep* step1 = new TGo4AnalysisStep("Unpacker",factory1,0,0,0);
   step1->SetErrorStopEnabled(kTRUE);
   AddAnalysisStep(step1);
// These settings will be overwritten by setup.C
   step1->SetSourceEnabled(kTRUE);
   step1->SetStoreEnabled(kFALSE);
   step1->SetProcessEnabled(kTRUE);

// Create step 2 Analysis.
   TGo4StepFactory* factory2 = new TGo4StepFactory("MapFactory");
   factory2->DefInputEvent("BasicEvent", "TAwagsSisBasicEvent"); // object name, class name
   factory2->DefEventProcessor("MapProc", "TAwagsSisMapProc"); // object name, class name
   factory2->DefOutputEvent("MapEvent", "TAwagsSisMapEvent"); // object name, class name
   TGo4AnalysisStep* step2    = new TGo4AnalysisStep("Mapping",factory2,0,0,0);
   step2->SetErrorStopEnabled(kTRUE);
   AddAnalysisStep(step2);
// These settings will be overwritten by setup.C
   step2->SetSourceEnabled(kFALSE);
   step2->SetStoreEnabled(kFALSE);
   step2->SetProcessEnabled(kTRUE);




}
//***********************************************************
TAwagsSisAnalysis::~TAwagsSisAnalysis()
{
   TGo4Log::Info("TAwagsSisAnalysis: Delete");
}
//***********************************************************

