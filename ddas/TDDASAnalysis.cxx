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

#include "TDDASAnalysis.h"

#include <stdlib.h>

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"

#include "Go4EventServer.h"
#include "TGo4AnalysisStep.h"
#include "TGo4Version.h"

#include "TDDASAnalysisEvent.h"
#include "TDDASFilterParameter.h"
#include "TDDASRawEvent.h"
#include "TGo4StepFactory.h"

//***********************************************************
TDDASAnalysis::TDDASAnalysis() :
   TGo4Analysis()
{
   TGo4Log::Error("Wrong constructor TDDASAnalysis()!");
}
//***********************************************************
// this constructor is used
TDDASAnalysis::TDDASAnalysis(int argc, char** argv) :
   TGo4Analysis(argc, argv)

{
   if (!TGo4Version::CheckVersion(__GO4BUILDVERSION__)) {
      TGo4Log::Error("Go4 version mismatch");
      exit(-1);
   }

   TGo4Log::Info("Create TDDASAnalysis %s", GetName());

   // the step definitions can be changed in the GUI
   // first step definitions:
   // the name of the step can be used later to get event objects
   TGo4StepFactory* factory1 = new TGo4StepFactory("Filter-factory");

   factory1->DefUserEventSource("TDDASEventSource"); // class name of user event source
   factory1->DefInputEvent("RawEvent","TDDASRawEvent"); // object name, class name
   factory1->DefEventProcessor("FilterProc", "TDDASFilterProc");// object name, class name
   factory1->DefOutputEvent("RawEvent", "TDDASRawEvent"); // object name, class name

   TGo4UserSourceParameter* source1  = new TGo4UserSourceParameter("run-2196_filt.root");
   TGo4FileStoreParameter*  store1   = new TGo4FileStoreParameter(Form("%sOutput", argv[0]));
   store1->SetOverwriteMode(kTRUE);
   TGo4AnalysisStep*        step1    = new TGo4AnalysisStep("Filter",factory1,source1,store1,0);

   step1->SetSourceEnabled(kTRUE);
   step1->SetStoreEnabled(kFALSE);  // disable output
   step1->SetProcessEnabled(kTRUE);
   step1->SetErrorStopEnabled(kTRUE);
   AddAnalysisStep(step1);


   TGo4StepFactory* factory2 = new TGo4StepFactory("Analysis-factory");

//      factory1->DefUserEventSource("TDDASEventSource"); // class name of user event source
      factory2->DefInputEvent("RawEvent","TDDASRawEvent"); // object name, class name
      factory2->DefEventProcessor("AnalysisProc", "TDDASAnalysisProc");// object name, class name
      factory2->DefOutputEvent("AnalysisEvent", "TDDASAnalysisEvent"); // object name, class name

//      TGo4UserSourceParameter* source2  = new TGo4UserSourceParameter("run-2196_filt.root");
   //   TGo4FileStoreParameter*  store2   = new TGo4FileStoreParameter(Form("%sOutput", argv[0]));
   //   store2->SetOverwriteMode(kTRUE);
      TGo4AnalysisStep*        step2    = new TGo4AnalysisStep("Analysis",factory2,0,0,0);

      step2->SetSourceEnabled(kFALSE);
      step2->SetStoreEnabled(kFALSE);  // disable output
      step2->SetProcessEnabled(kTRUE);
      step2->SetErrorStopEnabled(kTRUE);
      AddAnalysisStep(step2);

   // uncomment following line to define custom passwords for analysis server
   // DefineServerPasswords("DDASadmin", "DDASctrl", "DDASview");


}

//***********************************************************
TDDASAnalysis::~TDDASAnalysis()
{
   TGo4Log::Info("TDDASAnalysis: Delete");
}
//***********************************************************

//-----------------------------------------------------------
Int_t TDDASAnalysis::UserPreLoop()
{
   TGo4Log::Info("TDDASAnalysis: PreLoop");
   // we update the pointers to the current event structures here:

   return 0;
}
//-----------------------------------------------------------
Int_t TDDASAnalysis::UserPostLoop()
{
   TGo4Log::Info("TDDASAnalysis: PostLoop");
   TGo4Log::Info("Total events: %d", fEvents);

   return 0;
}

//-----------------------------------------------------------
Int_t TDDASAnalysis::UserEventFunc()
{
   //// This function is called once for each event.

   return 0;
}
