#include "TNectarAnalysis.h"

#include <stdlib.h>
#include "Riostream.h"

#include "Go4EventServer.h"
#include "TGo4StepFactory.h"
#include "TGo4AnalysisStep.h"
#include "TGo4Version.h"

//***********************************************************
TNectarAnalysis::TNectarAnalysis()
{
}
//***********************************************************

// this constructor is called by go4analysis executable
TNectarAnalysis::TNectarAnalysis(int argc, char** argv) :
   TGo4Analysis(argc, argv)
{
   cout << "**** Create TNectarAnalysis name: " << argv[0] << endl;

   if (!TGo4Version::CheckVersion(__GO4BUILDVERSION__)) {
      cout << "****  Go4 version mismatch" << endl;
      exit(-1);
   }



   TGo4StepFactory* factory = new TGo4StepFactory("Factory");
   factory->DefEventProcessor("NectarRawProc","TNectarRawProc");// object name, class name
   factory->DefOutputEvent("NectarRawEvent","TNectarRawEvent"); // object name, class name

   Text_t lmdfile[512]; // source file
   sprintf(lmdfile,"%s/data/test.lmd",getenv("GO4SYS"));
   // TGo4EventSourceParameter* sourcepar = new TGo4MbsTransportParameter("r3b");
   TGo4EventSourceParameter* sourcepar = new TGo4MbsFileParameter(lmdfile);

   TGo4FileStoreParameter* storepar = new TGo4FileStoreParameter(Form("%sOutput", argv[0]));
   storepar->SetOverwriteMode(kTRUE);

   TGo4AnalysisStep* step = new TGo4AnalysisStep("Raw", factory, sourcepar, storepar);

   step->SetSourceEnabled(kTRUE);
   step->SetStoreEnabled(kFALSE);
   step->SetProcessEnabled(kTRUE);
   step->SetErrorStopEnabled(kTRUE);
   AddAnalysisStep(step);

    // Now the first analysis step is set up.
   // Other steps could be created here
//   TGo4StepFactory* factory2 = new TGo4StepFactory("Factory2");
//   factory2->DefEventProcessor("NectarProfileProc","TNectarProfileProc");// object name, class name
//   factory2->DefOutputEvent("NectarProfileEvent","TNectarProfileEvent"); // object name, class name
//   factory2->DefInputEvent("NectarRawEvent","TNectarRawEvent"); // object name, class name
//
//
//     TGo4AnalysisStep* step2 = new TGo4AnalysisStep("Profile", factory2, 0, 0);
//
//     step2->SetSourceEnabled(kFALSE);
//     step2->SetStoreEnabled(kFALSE);
//     step2->SetProcessEnabled(kTRUE);
//     step2->SetErrorStopEnabled(kTRUE);
//
//     AddAnalysisStep(step2);

     //SetAutoSave(kFALSE);
   // uncomment following line to define custom passwords for analysis server
   // DefineServerPasswords("VupromNectaradmin", "VupromNectarctrl", "VupromNectarview");

}

//***********************************************************
TNectarAnalysis::~TNectarAnalysis()
{
   cout << "**** TNectarAnalysis: Delete instance" << endl;
}

