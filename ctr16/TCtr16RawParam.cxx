// $Id: TCtr16RawParam.cxx 557 2010-01-27 15:11:43Z linev $
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

#include "TCtr16RawParam.h"
#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TCtr16RawParam::TCtr16RawParam(const char* name) :
    TGo4Parameter(name)
{
  InitBoardMapping();
}

void TCtr16RawParam::InitBoardMapping()
{
  // init to non valid here:
  for (int s = 0; s < Ctr16_MAXSFP; ++s)
  {
    for (int d = 0; d < Ctr16_MAXDEVS; ++d)
    {
      fBoardID[s][d] = -1;
    }
  }

  fBoardID[0][0] = 42;    // this might be a unique hardware id

  fNumSnapshots = 64;
  fTraceLength = 16; //Ctr16_TRACEBINS;
  fDoCalibrate=kFALSE;
  fSlowMotion=kFALSE;
  fStopAtEvent=0;
  fVerbosity=0;

#ifdef Ctr16_DO_MEMORYSAMPLES
 fMemorySampleBoard=42; // id of board for adc memory sample evaluation
 fMemorySampleStatsLimit=1000; // number of event threshold for writing memory cells ADC values
#endif


}

Bool_t TCtr16RawParam::SetConfigBoards()
{
  TCtr16RawEvent::fgConfigCtr16Boards.clear();
  for (int s = 0; s < Ctr16_MAXSFP; ++s)
  {
    for (int d = 0; d < Ctr16_MAXDEVS; ++d)
    {
      Int_t bid = fBoardID[s][d];
      if (bid < 0)
        continue;
      TCtr16RawEvent::fgConfigCtr16Boards.push_back(bid);
      TGo4Log::Info("TCtr16RawParam::SetConfigBoards registers board unique id %u configured for sfp:%d dev:%d \n", bid,
          s, d);
    }
  }
  return kTRUE;
}

Bool_t TCtr16RawParam::UpdateFrom(TGo4Parameter *pp)
{
  TCtr16RawParam* from = dynamic_cast<TCtr16RawParam*>(pp);
  if (from == 0)
  {
    std::cout << "Wrong parameter object: " << pp->ClassName() << std::endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  std::cout << "**** TCtr16RawParam::UpdateFrom ... " << std::endl;

  SetConfigBoards();
  std::cout<< std::endl; // flush Qt text stream by this
  return kTRUE;

}
