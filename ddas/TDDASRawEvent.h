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

#ifndef TDDASRAWEVENT_H
#define TDDASRAWEVENT_H

#include "TGo4EventElement.h"
#include "DDASEvent.h"


/** JAM 2020 - try to figure out from existing code the histogram  array/limit sizes, please correct it*/
#define GO4_DDAS_CHANNELS 64
#define GO4_DDAS_SLOTS 16
#define GO4_DDAS_ADCS 21
#define GO4_DDAS_ERANGE 4096

#define GO4_DDAS_TIMERANGE 1000
#define GO4_DDAS_TRACELEN 8192

#define GO4_DDAS_MULTRANGE 100
#define GO4_DDAS_DSSD_STRIPS 16
#define GO4_DDAS_GAMMA_NPMTS 24


/**
 * Example for user defined raw event class.
 * This event structure corresponds to the user event source.
 */
class TDDASRawEvent : public TGo4EventElement {

   public:

      TDDASRawEvent();

      TDDASRawEvent(const char* name);

      virtual ~TDDASRawEvent();

      /**
        * Method called by the event owner (analysis step) to clear the
        * event element.
        */
      virtual void Clear(Option_t *t="");


      void PrintEvent();



      /** The event structure defined by ddasdumper*/
      DDASEvent fDDASEvent;

   ClassDef(TDDASRawEvent,1)
};

#endif //TDDASRAWEVENT_H
