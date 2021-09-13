// $Id: TNectarRawParam.cxx 557 2010-01-27 15:11:43Z linev $
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

#include "TNectarRawParam.h"
//#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TNectarRawParam::TNectarRawParam(const char* name) :
    TGo4Parameter(name)
{
  InitBoardMapping();
}

void TNectarRawParam::InitBoardMapping()
{
  // init to non valid here:
  for (int i = 0; i < VMMR_MAXBOARDS; ++i)
  {
    fVMMR_BoardID[i] = -1;
  }
  for (int i = 0; i < MDPP_MAXBOARDS; ++i)
    {
      fMDPP_BoardID[i] = -1;
      fMDPP_ReferenceChannel[i] = -1;
      fMDPP_is16Channels[i] = kFALSE;
    }

  fVMMR_BoardID[0] = 0;    // this might be a unique hardware id
  fMDPP_BoardID[0] = 0;    // this might be a unique hardware id
  fMDPP_ReferenceChannel[0]=0;
  fMDPP_is16Channels[0]=kTRUE; // test setup with one mdpp16
  fSlowMotion=kFALSE;

}

Bool_t TNectarRawParam::SetConfigBoards()
{
  TNectarRawEvent::fgConfigVmmrBoards.clear();
  for (int i = 0; i < VMMR_MAXBOARDS; ++i)
  {
    Int_t bid = fVMMR_BoardID[i];
    if (bid < 0)
      continue;
    TNectarRawEvent::fgConfigVmmrBoards.push_back(bid);
    TGo4Log::Info("TNectarRawParam::SetConfigBoards registers VMMR board unique id %u configured at index %d \n", bid, i);
  }
  TNectarRawEvent::fgConfigMdppBoards.clear();
   for (int i = 0; i < MDPP_MAXBOARDS; ++i)
   {
     Int_t bid = fMDPP_BoardID[i];
     if (bid < 0)
       continue;
     TNectarRawEvent::fgConfigMdppBoards.push_back(bid);
     TGo4Log::Info("TNectarRawParam::SetConfigBoards registers MDPP board unique id %u configured at index %d \n", bid, i);
   }

  return kTRUE;
}

Int_t TNectarRawParam::GetMDPPArrayindex(Int_t uniqueid)
{

  for (Int_t i = 0; i < MDPP_MAXBOARDS; ++i)
     {
       if (fMDPP_BoardID[i] == uniqueid) return i;

     }
  return -1;
}



Bool_t TNectarRawParam::UpdateFrom(TGo4Parameter *pp)
{
  TNectarRawParam* from = dynamic_cast<TNectarRawParam*>(pp);
  if (from == 0)
  {
    cout << "Wrong parameter object: " << pp->ClassName() << endl;
    return kFALSE;
  }
  if (!TGo4Parameter::UpdateFrom(pp))
    return kFALSE;    // will automatically copy 2d arrays
  cout << "**** TNectarRawParam::UpdateFrom ... " << std::endl;


  fSlowMotion=from->fSlowMotion;


  SetConfigBoards();
  std::cout<< std::endl; // flush Qt text stream by this
  return kTRUE;

}
