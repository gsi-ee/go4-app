// $Id: THitDetRawParam.cxx 557 2010-01-27 15:11:43Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "THitDetRawParam.h"
//#include "Riostream.h"
#include "TGo4Log.h"

THitDetRawParam::THitDetRawParam(const char* name) :
    TGo4Parameter(name)
{
  InitBoardMapping();
}

void THitDetRawParam::InitBoardMapping()
{
  // init to non valid here:
  for (int i = 0; i < HitDet_MAXBOARDS; ++i)
  {
    fBoardID[i] = -1;
  }

  fBoardID[0] = 42;    // this might be a unique hardware id

  fNumSnapshots = 64;
  fTraceLength = HitDet_TRACEBINS;
  fDoFFT=kTRUE;
  fFFTOptions="R2C M";

}

Bool_t THitDetRawParam::SetConfigBoards()
{
  THitDetRawEvent::fgConfigHitDetBoards.clear();
  for (int i = 0; i < HitDet_MAXBOARDS; ++i)
  {
    Int_t bid = fBoardID[i];
    if (bid < 0)
      continue;
    THitDetRawEvent::fgConfigHitDetBoards.push_back(bid);
    TGo4Log::Info("THitDetRawParam::SetConfigBoards registers board unique id %u configured at index %d \n", bid, i);
  }
  return kTRUE;
}

Bool_t THitDetRawParam::UpdateFrom(TGo4Parameter *pp)
{
  THitDetRawParam* from = dynamic_cast<THitDetRawParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  cout << "**** THitDetRawParam::UpdateFrom ... " << endl;

  fNumSnapshots = from->fNumSnapshots;
  fTraceLength = from->fTraceLength;
  fDoFFT=from->fDoFFT;
  fFFTOptions=from->fFFTOptions;
  SetConfigBoards();
  return kTRUE;

}
