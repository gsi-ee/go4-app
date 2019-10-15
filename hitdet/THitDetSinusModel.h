#ifndef THITDETSINUSMODEL_H
#define THITDETSINUSMODEL_H

#include "TGo4FitModel.h"

#include "TMath.h"

class THitDetSinusModel : public TGo4FitModel {
   public:
      THitDetSinusModel(const char* iName):
         TGo4FitModel(iName, "Sinus", kTRUE)
   {
     NewParameter("T","model template parameter",20.); // period length
     NewParameter("X0","model template parameter",0.); // sinus phase shift

     SetParRange("X0", 0.0, 20);
     SetParRange("Ampl", 0.0, 2048);

   }

   virtual ~THitDetSinusModel(){}


   protected:
      virtual Double_t UserFunction(Double_t* Coordinates, Double_t* Parameters)
   {
     Double_t T= Parameters[0];
     Double_t X0= Parameters[1];

     Double_t x = Coordinates[0];


     return TMath::Sin((2 * TMath::Pi() / T) * (x - X0));
   }



   ClassDef(THitDetSinusModel, 1);
};

#endif // TMODELTEMPLATE_H
