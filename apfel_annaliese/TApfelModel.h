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

#include "TGo4FitModel.h"

#include "TMath.h"

class TApfelModel : public TGo4FitModel {
   public:
      TApfelModel(const char* iName):
         TGo4FitModel(iName, "Peak form", kTRUE) 
   {
     NewParameter("N","model template parameter",3.);
     NewParameter("Tau","model template parameter",10.);
     NewParameter("Shift","model template parameter",-20.);
   }


   protected:
      virtual Double_t UserFunction(Double_t* Coordinates, Double_t* Parameters) 
   {
     Double_t N = Parameters[0];
     Double_t Tau = Parameters[1];
     Double_t Shift = Parameters[2];
     Double_t x = Coordinates[0];

     if ((x-Shift) < 0) return 0;
     //return -1*TMath::Exp(-N*(x-Shift)/Tau)*TMath::Power((x-Shift)/Tau, N);
   	return 20.*TMath::Exp(-N*(x-Shift)/Tau)*TMath::Power((x-Shift)/Tau, N);
}

   ClassDef(TApfelModel, 1);
};

#endif // TMODELTEMPLATE_H
