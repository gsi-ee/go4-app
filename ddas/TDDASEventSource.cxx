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




#include "TDDASEventSource.h"
#include "TGo4FileSource.h"

#include <stdlib.h>

#include "TClass.h"

#include "TGo4Log.h"
#include "TGo4EventErrorException.h"
#include "TGo4EventEndException.h"
#include "TGo4EventTimeoutException.h"
#include "TGo4UserSourceParameter.h"
#include "TDDASRawEvent.h"

#include "TKey.h"
#include "TFile.h"
#include "TTree.h"
#include "TList.h"
#include "TSystem.h"
#include "TObjString.h"


TDDASEventSource::TDDASEventSource(const char* name) :
   TGo4EventSource(name),
   fxFile(0),
   fxTree(0),
   fiMaxEvents(0),
   fiCurrentEvent(0),
   fiGlobalEvent(0),
   fbActivated(kFALSE),
   fxFilesNames(0)


{
   Init();
}

TDDASEventSource::TDDASEventSource(TGo4UserSourceParameter* par) :
   TGo4EventSource(" "),
   fxFile(0),
      fxTree(0),
      fiMaxEvents(0),
      fiCurrentEvent(0),
      fiGlobalEvent(0),
      fbActivated(kFALSE),
      fxFilesNames(0)


   {
   if(par) {
      SetName(par->GetName());
      Init();
   } else {
      TGo4Log::Error("TDDASEventSource constructor with zero parameter!");
   }
}

TDDASEventSource::TDDASEventSource() :
   TGo4EventSource("default DDAS source"),
   fxFile(0),
   fxTree(0),
   fiMaxEvents(0),
   fiCurrentEvent(0),
   fiGlobalEvent(0),
   fbActivated(kFALSE),
   fxFilesNames(0)
{
}


void TDDASEventSource::Init()
{
  fxFilesNames = TGo4FileSource::ProducesFilesList(GetName());

     if (!OpenNextFile())
        ThrowError(66,0, Form("!!! ERROR: Cannot open source %s!!!", GetName()));
}

TDDASEventSource::~TDDASEventSource()
{
  CloseCurrentFile();

     if (fxFilesNames) {
        delete fxFilesNames;
        fxFilesNames = 0;
     }
}

Bool_t TDDASEventSource::CheckEventClass(TClass* cl)
{
   return cl->InheritsFrom(TDDASRawEvent::Class());
}

Bool_t TDDASEventSource::BuildEvent(TGo4EventElement* dest)
{
   TDDASRawEvent* evnt = dynamic_cast<TDDASRawEvent*> (dest);
   if(evnt==0) ThrowError(0,22,"!!! TDDASEventSource ERROR BuildEvent: no TDDASRawEvent destination event!!!");
   if (fxTree==0) ThrowError(0,33,"!!! TDDASEventSource ERROR BuildEvent: no Tree !!!");

     if(fiCurrentEvent >= fiMaxEvents) {
        if (!OpenNextFile())
           ThrowEOF(0,44,"End at event %ld !!!", fiGlobalEvent);
     }

     if(!fbActivated) {
        // Event dest should be full event as filled into the tree
        // the name of the event element may indicate the subpart
        //(tree branchname) that should be read partially only
       fDDASEventPointer=&(evnt->fDDASEvent);

       fxTree->SetBranchAddress("ddasevent",&fDDASEventPointer);


        fbActivated = kTRUE;
        TGo4Log::Info("TDDASEventSourceT: Initialized tree from file %s", fxCurrentFileName.Data());
     }  //  if(!fbActivated)
     // end init section ////////////

     fiGlobalEvent++;

     return fxTree->GetEntry(fiCurrentEvent++) > 0;
}



Bool_t TDDASEventSource::OpenNextFile()
{
   CloseCurrentFile();

   if ((fxFilesNames==0) || (fxFilesNames->GetSize()==0)) return kFALSE;

   TObject* obj = fxFilesNames->First();
   fxCurrentFileName = obj->GetName();
   fxFilesNames->Remove(fxFilesNames->FirstLink());
   delete obj;

   fxFile = TFile::Open(fxCurrentFileName.Data(), "READ"); // in such way rfio etc is also supported!
   if(fxFile==0) ThrowError(66,0,Form("!!! ERROR: FILE %s not found !!!", fxCurrentFileName.Data()));
   if (!fxFile->IsOpen()) ThrowError(66,0,Form("!!! ERROR: FILE %s cannot open !!!", fxCurrentFileName.Data()));

   TKey* kee = 0;
   TIter iter(fxFile->GetListOfKeys());
   while ( (kee=dynamic_cast<TKey*>(iter())) !=0 ) {
      fxTree = dynamic_cast<TTree*>(kee->ReadObj());
      if (fxTree) break; // we take first Tree in file, disregarding its name...
   }
   if (fxTree==0) {
      ThrowError(77,0,"!!! ERROR: Tree not found !!!");
   } else {
      SetCreateStatus(0);
      fiMaxEvents = fxTree->GetEntries();
   }

   return kTRUE;
}


Bool_t TDDASEventSource::CloseCurrentFile()
{
   if (fxFile) {
      delete fxFile;
      TGo4Log::Info("TDDASEventSource: Close file %s", fxCurrentFileName.Data());
   }

   fxFile = 0;
   fxTree = 0;
   fiMaxEvents = 0;
   fiCurrentEvent = 0;
   fbActivated = kFALSE;
   fxCurrentFileName = "";

   return kTRUE;
}


