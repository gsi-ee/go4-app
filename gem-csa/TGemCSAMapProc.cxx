

#include "TGemCSAMapProc.h"

#include <math.h>
#include <vector>

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"

#include "TGo4WinCond.h"
#include "TGo4Analysis.h"

#include "TGemCSAMapEvent.h"
#include "TGemCSABasicEvent.h"
#include "TGemCSAMapParam.h"

//-----------------------------------------------------------
TGemCSAMapProc::TGemCSAMapProc() :
   TGo4EventProcessor()
{
}

//-----------------------------------------------------------
TGemCSAMapProc::TGemCSAMapProc(const char* name) :
   TGo4EventProcessor(name),
   fParam(0)
{
   TGo4Log::Info("TGemCSAMapProc: Create");
   //// init user analysis objects:

   // here already exisitng parameter will be returned
   // one not need to specify macro here - it is already executed in first step
   fParam = (TGemCSAMapParam*) MakeParameter("GemCSAMapParam", "TGemCSAMapParam");
   TString obname;
   TString obtitle;
   //TString foldername;
   Int_t tracelength=CSA_TRACE_SIZE; // TODO: dynamically get trace size from first step?
  for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
  {
    for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
    {

      obname.Form("Mapped/Raw/Chamber%d/Trace_%d_%d", dev, dev, wire);
      obtitle.Form("Chamber %d Wire %d Raw Trace", dev, wire);
      hWireTraces[dev][wire] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)",
          "counts");

      obname.Form("Mapped/BLR/Chamber%d/TraceBLR_%d_%d", dev, dev, wire);
      obtitle.Form("Chamber %d Wire %d base line restored Trace", dev, wire);
      hWireTracesBLR[dev][wire] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)",
          "counts");

      obname.Form("Mapped/Filtered/Chamber%d/TraceFPGA_%d_%d", dev, dev, wire);
      obtitle.Form("Chamber %d Wire %d FPGA trapez filtered Trace", dev, wire);
      hWireTracesFPGA[dev][wire] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)",
               "counts");


    }    // wires

    Int_t trbinsize= tracelength/10;
    obname.Form("Mapped/Raw/Chamber%d/OverviewTrace_%d", dev, dev);
    obtitle.Form("Chamber %d Trace wire map", dev);
    hMapTrace[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES -1,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/BLR/Chamber%d/OverviewTraceBLR_%d", dev, dev);
    obtitle.Form("Chamber %d Trace base line restored wire map", dev);
    hMapTraceBLR[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES -1,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/Filtered/Chamber%d/OverviewTraceFiltered_%d", dev, dev);
    obtitle.Form("Chamber %d Trace (FPGA filtered)wire map", dev);
    hMapTraceFPGA[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES -1,"time (bins)", "wire number", "counts");





  }    // devs


}
//-----------------------------------------------------------
TGemCSAMapProc::~TGemCSAMapProc()
{
   TGo4Log::Info("TGemCSAMapProc: Delete");

}
//-----------------------------------------------------------
Bool_t TGemCSAMapProc::BuildEvent(TGo4EventElement* dest)
{
	Bool_t isValid=kFALSE; // validity of output event

   TGemCSABasicEvent* inp_evt  = (TGemCSABasicEvent*) GetInputEvent();
   TGemCSAMapEvent* out_evt = (TGemCSAMapEvent*) dest;

   ResetTraces();

   if((inp_evt==0) || !inp_evt->IsValid()){ // input invalid
	  out_evt->SetValid(isValid); // invalid
	  return isValid; // must be same is for SetValid
   }
   isValid=kTRUE;

// TODO: fill mapped traces using the mapping here


   for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
      {
        for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
        {
          Int_t sfp=fParam->fSFP [dev][wire];
          Int_t slave=fParam->fSlave [dev][wire];
          Int_t chan=fParam->fChannel [dev][wire];
          if(sfp<0 || slave<0 || chan<0) continue;
          if(sfp>MAX_SFP || slave>MAX_SLAVE || chan>N_CHA) continue;

          std::vector<Double_t> & theTrace=inp_evt->fTrace[sfp][slave][chan];
          for(size_t bin=0; bin<theTrace.size(); ++bin)
          {
            hWireTraces[dev][wire]->SetBinContent(1+bin, theTrace[bin]);
            hMapTrace[dev]->Fill(bin, wire, theTrace[bin]);
            //out_evt->fWireTrace[dev][wire].push_back(theTrace[bin]);

          }

          std::vector<Double_t> & theTraceBLR=inp_evt->fTraceBLR[sfp][slave][chan];
          for(size_t bin=0; bin<theTraceBLR.size(); ++bin)
          {
            hWireTracesBLR[dev][wire]->SetBinContent(1+bin,theTraceBLR[bin]);
            hMapTraceBLR[dev]->Fill(bin, wire, theTraceBLR[bin]);
            //out_evt->fWireTraceBLR[dev][wire].push_back(theTrace[bin]);
          }

          std::vector<Double_t> & theTraceFPGA=inp_evt->fTraceFPGA[sfp][slave][chan];
          for(size_t bin=0; bin<theTraceFPGA.size(); ++bin)
          {
            hWireTracesFPGA[dev][wire]->SetBinContent(1+bin,theTraceFPGA[bin]);
            hMapTraceFPGA[dev]->Fill(bin, wire, theTraceFPGA[bin]);
            //out_evt->fWireTraceFPGA[dev][wire].push_back(theTrace[bin]);
          }


        }

      }



   out_evt->SetValid(isValid);
   return isValid;
}


void TGemCSAMapProc::ResetTraces()
{
  for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
       {
         for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
         {
           hWireTraces[dev][wire]->Reset("");
           hWireTracesBLR[dev][wire]->Reset("");
           hWireTracesFPGA[dev][wire]->Reset("");


         }
         hMapTrace[dev]->Reset("");
         hMapTraceBLR[dev]->Reset("");
         hMapTraceFPGA[dev]->Reset("");
       }
}
