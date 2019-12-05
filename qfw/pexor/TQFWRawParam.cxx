// $Id: TQFWRawParam.cxx 557 2010-01-27 15:11:43Z linev $
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

#include "TQFWRawParam.h"
//#include "Riostream.h"
#include "TGo4Log.h"

using namespace std;

TQFWRawParam::TQFWRawParam(const char* name) :
   TGo4Parameter(name),
   fSimpleCompensation(kFALSE),fCheckEventSequence(kFALSE),fBadChannelCorrection(kFALSE)
{
  InitBoardMapping();
}


void TQFWRawParam::InitBoardMapping()
{
   // init to non valid here:
   for (int sfp = 0; sfp < PEXOR_MAXSFP; ++sfp)
         {
         for (int dev = 0; dev < PEXOR_MAXDEVS; ++dev)
               {
                  fBoardID[sfp][dev]=-1;
               }
         }

//    fBoardID[0][0]=10; // test case: one qfw board at sfp0
//    fBoardID[1][0]=11; // test case: one qfw board at sfp1
//    fBoardID[2][0]=12; // test case: one qfw board at sfp2



   fSelectTriggerEvents=kFALSE;
   fTriggerHighThreshold=5000000; // selects measurement event if integral of trigger channel counts is above
   fTriggerLowThreshold=4000000; // selects background events if integral of trigger channel counts is below
   fTriggerBoardID=12;
   fTriggerFirstChannel=0; // lower boundary of trigger channel region
   fTriggerLastChannel=1;

   fFrontendOffsetTrigger=14; // per default use start trigger
   fUseFrontendOffsets=kFALSE;
   fFrontendOffsetLoop=-1;
   fUseScalingFactor=kFALSE;
   fScalingFactor=1.0;


}

Bool_t TQFWRawParam::SetConfigBoards()
{
   TQFWRawEvent::fgConfigQFWBoards.clear();
   for (int sfp = 0; sfp < PEXOR_MAXSFP; ++sfp)
            {
            for (int dev = 0; dev < PEXOR_MAXDEVS; ++dev)
                  {
                     Int_t bid=fBoardID[sfp][dev];
                     if(bid<0) continue;
                     TQFWRawEvent::fgConfigQFWBoards.push_back(bid);
                     TGo4Log::Info("TQFWRawParam::SetConfigBoards registers board unique id %u configured at sfp %d dev %d\n",bid,sfp,dev);
                  }
            }
   return kTRUE;
}







Bool_t TQFWRawParam::UpdateFrom(TGo4Parameter *pp)
{
	TQFWRawParam* from = dynamic_cast<TQFWRawParam*> (pp);
   if (from==0) {
      cout << "Wrong parameter object: " << pp->ClassName() << endl;
      return kFALSE;
   }
   if(!TGo4Parameter::UpdateFrom(pp)) return kFALSE; // will automatically copy 2d arrays
   cout << "**** TQFWRawParam::UpdateFrom ... " << endl;
   // automatic reset if we newly start the offset calibration:


   fSelectTriggerEvents=from->fSelectTriggerEvents;
   fTriggerHighThreshold=from->fTriggerHighThreshold;
   fTriggerLowThreshold=from->fTriggerLowThreshold;
   fTriggerBoardID=from->fTriggerBoardID;
   fTriggerFirstChannel=from->fTriggerFirstChannel;
   fTriggerLastChannel=from->fTriggerLastChannel;
   fFrontendOffsetTrigger=from->fFrontendOffsetTrigger;
   fUseFrontendOffsets=from->fUseFrontendOffsets;
   fFrontendOffsetLoop=from->fFrontendOffsetLoop;
   fCheckEventSequence=from->fCheckEventSequence;
   fBadChannelCorrection=from->fBadChannelCorrection;
   fUseScalingFactor=from->fUseScalingFactor;
   fScalingFactor=from->fScalingFactor;

   SetConfigBoards();
   return kTRUE;

}
