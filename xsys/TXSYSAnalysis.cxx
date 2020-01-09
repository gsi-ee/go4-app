
#include "TXSYSAnalysis.h"

#include <stdlib.h>
#include "Riostream.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"

#include "Go4EventServer.h"
#include "TGo4AnalysisStep.h"
#include "TGo4Version.h"

#include "TXSYSUnpackEvent.h"
#include "TGo4StepFactory.h"
#include "TGo4MbsEvent.h"

//***********************************************************
TXSYSAnalysis::TXSYSAnalysis() :
   TGo4Analysis(),
   fRawEvent(0),
   fUnpackEvent(0),
   fSize(0),
   fEvents(0)
{
  cout << "Wrong constructor TXSYSAnalysis()!" << endl;
}
//***********************************************************
// this constructor is used
TXSYSAnalysis::TXSYSAnalysis(int argc, char** argv) :
   TGo4Analysis(argc, argv),
   fRawEvent(0),
   fUnpackEvent(0),
   fSize(0),
   fEvents(0)
{
   if (!TGo4Version::CheckVersion(__GO4BUILDVERSION__)) {
      cout << "****  Go4 version mismatch" << endl;
      exit(-1);
   }


   cout << "**** TXSYSAnalysis: Create" << endl;

   // the step definitions can be changed in the GUI
   // first step definitions:
   // the name of the step can be used later to get event objects
   TGo4StepFactory* factory1 = new TGo4StepFactory("Unpack-factory");

   factory1->DefUserEventSource("TXSYSEventSource"); // class name of user event source
   factory1->DefInputEvent("RawEvent","TGo4MbsEvent"); // object name, class name
   factory1->DefEventProcessor("UnpackProc", "TXSYSUnpackProc");// object name, class name
   factory1->DefOutputEvent("UnpackEvent", "TXSYSUnpackEvent"); // object name, class name

   TGo4UserSourceParameter* source1  = new TGo4UserSourceParameter("test");
   TGo4FileStoreParameter*  store1   = new TGo4FileStoreParameter(Form("%sOutput", argv[0]));
   store1->SetOverwriteMode(kTRUE);
   TGo4AnalysisStep*        step1    = new TGo4AnalysisStep("Unpack",factory1,source1,store1,0);

   step1->SetSourceEnabled(kTRUE);
   step1->SetStoreEnabled(kFALSE);  // disable output
   step1->SetProcessEnabled(kTRUE);
   step1->SetErrorStopEnabled(kTRUE);
   AddAnalysisStep(step1);

}

//***********************************************************
TXSYSAnalysis::~TXSYSAnalysis()
{
   cout << "**** TXSYSAnalysis: Delete" << endl;
}
//***********************************************************

//-----------------------------------------------------------
Int_t TXSYSAnalysis::UserPreLoop()
{
   cout << "**** TXSYSAnalysis: PreLoop" << endl;
   // we update the pointers to the current event structures here:
   fRawEvent = dynamic_cast<TGo4MbsEvent*>    (GetInputEvent("Unpack"));   // of step "Unpack"
   fUnpackEvent = dynamic_cast<TXSYSUnpackEvent*> (GetOutputEvent("Unpack"));
   fEvents=0;

   // create histogram for UserEventFunc
   // At this point, the histogram has been restored from autosave file if any.
   fSize=(TH1D*)GetHistogram("Eventsize");
   if(fSize==0) {
      // no autosave read, create new and register
      fSize = new TH1D ("Eventsize", "Read columns",160,1,160);
      AddHistogram(fSize);
   }
   //ClearObjects("Histograms"); // reset all histograms to 0 before run
   // name specifies folder to clear
   return 0;
}
//-----------------------------------------------------------
Int_t TXSYSAnalysis::UserPostLoop()
{
   cout << "**** TXSYSAnalysis: PostLoop" << endl;
   cout << " Total events: " << fEvents << endl;
   fUnpackEvent = 0; // reset to avoid invalid pointer if analysis is changed in between
   fRawEvent = 0;
   fEvents=0;
   return 0;
}

//-----------------------------------------------------------
Int_t TXSYSAnalysis::UserEventFunc()
{
   //// This function is called once for each event.
   Int_t value=0;
   if(fRawEvent) value = fRawEvent->GetDlen();
   fSize->Fill(value); // fill histogram
   fEvents++;
   return 0;
}
