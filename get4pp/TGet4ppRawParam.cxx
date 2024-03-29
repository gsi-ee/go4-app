// $Id: TGet4ppRawParam.cxx 557 2010-01-27 15:11:43Z linev $
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

#include "TGet4ppRawParam.h"
#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TGet4ppRawParam::TGet4ppRawParam(const char* name) :
    TGo4Parameter(name)
{
  InitBoardMapping();
}

void TGet4ppRawParam::InitBoardMapping()
{
  // init to non valid here:
  for (int i = 0; i < Get4pp_MAXBOARDS; ++i)
  {
    fBoardID[i] = -1;
  }

  fBoardID[0] = 42;    // this might be a unique hardware id

  fVerbosity=0;
  fSlowMotion=kFALSE;
#ifdef Get4pp_DOFINETIMSAMPLES
  fFineTimeSampleBoard=42; // the only one we have...
  fFineTimeStatsLimit=10; // software test only
#endif

  fUseSoftwareCalibration=kTRUE; // if true, evaluate delta times with software calibration of fine time bins
  fResetCalibration=kFALSE; // set true to begin new software calibration procedure
  fCalibrationCounts=Get4pp_DEFAULT_CALIBCOUNT; // minimum required statistics for each channel (leading/trailing) to complete calibration

}

Bool_t TGet4ppRawParam::SetConfigBoards()
{
  TGet4ppRawEvent::fgConfigGet4ppBoards.clear();
  for (int i = 0; i < Get4pp_MAXBOARDS; ++i)
  {
    Int_t bid = fBoardID[i];
    if (bid < 0)
      continue;
    TGet4ppRawEvent::fgConfigGet4ppBoards.push_back(bid);
    TGo4Log::Info("TGet4ppRawParam::SetConfigBoards registers board unique id %u configured at index %d \n", bid, i);
  }
  return kTRUE;
}

Bool_t TGet4ppRawParam::UpdateFrom(TGo4Parameter *pp)
{
  TGet4ppRawParam* from = dynamic_cast<TGet4ppRawParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  cout << "**** TGet4ppRawParam::UpdateFrom ... " << std::endl;

  SetConfigBoards();
  std::cout<< std::endl; // flush Qt text stream by this
  return kTRUE;

}
