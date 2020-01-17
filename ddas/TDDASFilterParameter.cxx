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

#include "TDDASFilterParameter.h"

#include "Riostream.h"

#include "TGo4Log.h"

TDDASFilterParameter::TDDASFilterParameter() :
   TGo4Parameter("DDASFilterParameter")
{
  fDoFiltering=kTRUE;
  fDSSD_Front_Imax=0; // lower threshold of front dssd imax value for filtering
  fDSSD_Front_Dmax=0; // lower threshold of front dssd dmax value for filtering
  fDSSD_Back_Imax=0; // lower threshold of back dssd imax value for filtering
  fDSSD_Back_Dmax=0; // lower threshold of back dssd dmax value for filtering
}

TDDASFilterParameter::TDDASFilterParameter(const char* name) :
   TGo4Parameter(name)
{
  fDoFiltering=kTRUE;
  fDSSD_Front_Imax=0; // lower threshold of front dssd imax value for filtering
  fDSSD_Front_Dmax=0; // lower threshold of front dssd dmax value for filtering
  fDSSD_Back_Imax=0; // lower threshold of back dssd imax value for filtering
  fDSSD_Back_Dmax=0; // lower threshold of back dssd dmax value for filtering

}

TDDASFilterParameter::~TDDASFilterParameter()
{

}


Bool_t TDDASFilterParameter::UpdateFrom(TGo4Parameter *source)
{
   TDDASFilterParameter* from = dynamic_cast<TDDASFilterParameter*> (source);
   if (from==0) {
      TGo4Log::Error("Wrong parameter class: %s", source->ClassName());
      return kFALSE;
   }
   fDoFiltering= from->fDoFiltering;
   fDSSD_Front_Imax=from->fDSSD_Front_Imax;
   fDSSD_Front_Dmax=from->fDSSD_Front_Dmax;
   fDSSD_Back_Imax=from->fDSSD_Back_Imax;
   fDSSD_Back_Dmax=from->fDSSD_Back_Dmax;
   return kTRUE;
}
