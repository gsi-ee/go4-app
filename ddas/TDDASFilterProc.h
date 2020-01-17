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

#ifndef TDDASUNPACKPROCESSOR_H
#define TDDASUNPACKPROCESSOR_H

#include "TGo4EventProcessor.h"

#include "TH1.h"
#include "TRandom.h"
#include "TRandom3.h"

#include "Parameters-ddas.h"
#include "Variables-ddas.h"

//class TDDASRawEvent;
#include "TDDASRawEvent.h"

class TDDASFilterParameter;
class ddaschannel;


class TDDASFilterProc : public TGo4EventProcessor {
   public:
      TDDASFilterProc() ;
      TDDASFilterProc(const char* name);
      virtual ~TDDASFilterProc() ;

      /** This method checks if event class is suited for the source */
      //virtual Bool_t CheckEventClass(TClass* cl);

      virtual Bool_t BuildEvent(TGo4EventElement* dest);
      /** make MapChannels static, so other event processor can use same function.*/
      static void MapChannels(int crateid, int slotid, int channum, betadecay &bdecay, betadecayvariables &bdecayv);
      static TRandom3 fRandom3;

   protected:
      void DdasToEvent();



      /** Raw histograms JAM 2020*/
      TH1* hTime[GO4_DDAS_ADCS][GO4_DDAS_CHANNELS];
      TH1* hEnergy[GO4_DDAS_ADCS][GO4_DDAS_CHANNELS];
      TH1* hTrace[GO4_DDAS_ADCS][GO4_DDAS_CHANNELS];

      TH2* hEnergyMap;


      /** some mapped histograms JAM2020*/
      TH1* hDSSD_Front_Higain_Energy[GO4_DDAS_DSSD_STRIPS];
      TH1* hDSSD_Front_Logain_Energy[GO4_DDAS_DSSD_STRIPS];
      TH1* hDSSD_Back_Higain_Energy[GO4_DDAS_DSSD_STRIPS];
      TH1* hDSSD_Back_Logain_Energy[GO4_DDAS_DSSD_STRIPS];
      TH1* hGamma_SuN_Intens[GO4_DDAS_GAMMA_NPMTS];
      TH1* hGamma_SuN_Energy[GO4_DDAS_GAMMA_NPMTS];
      TH1* hGamma_SuN_Time [GO4_DDAS_GAMMA_NPMTS];

      // TODO- further histograms for // Pin01, Pin02, I2pos, TAC,  XFP, Pin01 - I2S, Pin02 - I2N
      //Pin01 - XFP, Pin02 - XFP, Veto



   private:
      TDDASFilterParameter *fPar;
      TDDASRawEvent* fInput;
      TDDASRawEvent* fOutput;

      betadecay fBdecay;
      betadecayvariables fBdecayVars;



   ClassDef(TDDASFilterProc,1)
};

#endif //TDDASUNPACKPROCESSOR_H
