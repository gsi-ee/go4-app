// $Id: TApfelModel.h 478 2009-10-29 12:26:09Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#ifndef TAPFELMODEL_H
#define TAPFELMODEL_H

// this define will switch between Go4 fitter amplitude estimation and custom amplitude parameter that is always positive!
//#define APFELMODEL_USE_AMPLITUDEESTIMATION 1


#include "TGo4FitModel.h"

#include "TMath.h"

class TApfelModel : public TGo4FitModel {
   public:
      TApfelModel(const char* iName):
#ifdef       APFELMODEL_USE_AMPLITUDEESTIMATION
         TGo4FitModel(iName, "Peak form", kTRUE) 
#else
      TGo4FitModel(iName, "Peak form", kFALSE)
#endif
   {

     NewParameter("N","model template parameter",3.);
     NewParameter("Tau","model template parameter",10.);
     NewParameter("Shift","model template parameter",-20.);
#ifndef APFELMODEL_USE_AMPLITUDEESTIMATION
     NewParameter("ApfelAmp","model template parameter",100.);
#endif

   }

   protected:
      virtual Double_t UserFunction(Double_t* Coordinates, Double_t* Parameters) 
   {
     Double_t N = Parameters[0];
     Double_t Tau = Parameters[1];
     Double_t Shift = Parameters[2];
#ifndef APFELMODEL_USE_AMPLITUDEESTIMATION
     Double_t Amp = Parameters[3];
#endif
     Double_t x = Coordinates[0];

     if ((x-Shift) < 0) return 0;
     //return -1*TMath::Exp(-N*(x-Shift)/Tau)*TMath::Power((x-Shift)/Tau, N);

#ifdef       APFELMODEL_USE_AMPLITUDEESTIMATION
     return 20.*TMath::Exp(-N*(x-Shift)/Tau)*TMath::Power((x-Shift)/Tau, N);
#else
     return 20.*TMath::Abs(Amp) *TMath::Exp(-N*(x-Shift)/Tau)*TMath::Power((x-Shift)/Tau, N);
#endif

}

   ClassDef(TApfelModel, 1);
};

#endif // TMODELTEMPLATE_H
