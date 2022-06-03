// $Id: TTamex_FullEvent.h 2627 2019-10-01 08:02:45Z linev $
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

#ifndef TTAMEXXEVENT_H
#define TTAMEXXEVENT_H

#include "TGo4EventElement.h"
#include <vector>

#define MAX_CHA_AN    64
// JAM 3-jun-2022: we only use the preselected "analysis" channels from event processor here. This is redefinition of TTamex_FullProc.h
// proper way would be to put all defines here, but we don't do this not to confuse legacy users...
#define MAX_HITS 1



class TTamex_FullEvent : public TGo4EventElement {
   public:
      TTamex_FullEvent();
      TTamex_FullEvent(const char* name);
      virtual ~TTamex_FullEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");
      
      /* add new timstamp to buffer*/
      void AddTimestamp(UChar_t channel, Double_t value)
      {
        if(channel>=MAX_CHA_AN) return;
        //fTimeStamp[channel].push_back(value);
        fTimeStamp[channel]=value;
      }
#
      /* get timestamp from position i in buffer (0 is first in time sequence)*/
      Double_t GetTimestamp(UInt_t channel, UInt_t i)
      {
        if(channel>=MAX_CHA_AN) return -1; // TODO: proper error handling maybe..
//        if(i>=fTimeStamp[channel].size()) return 0;
//        return fTimeStamp[channel].at(i);
        return fTimeStamp[channel];
      }

      /* Total number of timestamps in buffer for specific id. to be used in readout loops of second analysis step*/
//      UInt_t NumTimestamps(UChar_t channel)
//        {
//          if(channel>=MAX_CHA_AN) return 0;
//          return fTimeStamp[channel].size();
//        }

      // OLD example
      //Double_t fTimeDiff[MAX_CHA_AN];

       //std::vector<Double_t> fTimeStamp[MAX_CHA_AN]; //<full time information of this analysis channel
      // JAM: vectors problematic with treeviewer? we restrict to one hit per channel/event
      Double_t fTimeStamp[MAX_CHA_AN];


   ClassDef(TTamex_FullEvent,1)
};
#endif //TEVENT_H



