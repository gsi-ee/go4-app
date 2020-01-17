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

#include "TDDASRawEvent.h"


#include "TGo4Log.h"

TDDASRawEvent::TDDASRawEvent() :
   TGo4EventElement()
{
   // default ctor is for streamer only, avoid heap objects here!
}

TDDASRawEvent::TDDASRawEvent(const char* name) :
   TGo4EventElement(name)
{
}

TDDASRawEvent::~TDDASRawEvent()
{

}

void TDDASRawEvent::Clear(Option_t *t)
{
  fDDASEvent.Reset();
}

void TDDASRawEvent::PrintEvent()
{
   TGo4EventElement::PrintEvent();
   TGo4Log::Info( "TDDASRawEvent printout");
   TGo4Log::Info("  Number of channel data: %d",fDDASEvent.GetNEvents());
   TGo4Log::Info("  First Time: %e",fDDASEvent.GetFirstTime());
   TGo4Log::Info("  Last Time: %e",fDDASEvent.GetLastTime());
   TGo4Log::Info("  Time Width: %e", fDDASEvent.GetTimeWidth());
}
