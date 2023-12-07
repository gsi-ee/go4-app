

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
   fParam = (TGemCSAMapParam*) MakeParameter("GemCSAMapParam", "TGemCSAMapParam", "set_GemCSAMapParam.C");
   TString obname;
   TString obtitle;
   //TString foldername;

#ifdef   USE_CSA_MAPPING

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
    hMapTrace[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/BLR/Chamber%d/OverviewTraceBLR_%d", dev, dev);
    obtitle.Form("Chamber %d Trace base line restored wire map", dev);
    hMapTraceBLR[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/Filtered/Chamber%d/OverviewTraceFiltered_%d", dev, dev);
    obtitle.Form("Chamber %d Trace (FPGA filtered)wire map", dev);
    hMapTraceFPGA[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/Raw/Chamber%d/SumOverviewTrace_%d", dev, dev);
    obtitle.Form("Chamber %d Trace wire map accumulated", dev);
    hMapTrace_Sum[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/BLR/Chamber%d/SumOverviewTraceBLR_%d", dev, dev);
    obtitle.Form("Chamber %d Trace base line restored wire map accumulated", dev);
    hMapTraceBLR_Sum[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/Filtered/Chamber%d/SumOverviewTraceFiltered_%d", dev, dev);
    obtitle.Form("Chamber %d Trace (FPGA filtered)wire map accumulated", dev);
    hMapTraceFPGA_Sum[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");

    obname.Form("Mapped/Raw/Chamber%d/WireProfile_%d", dev, dev);
    obtitle.Form("Chamber %d Wire Profile", dev);
    hWireProfile[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");

    obname.Form("Mapped/BLR/Chamber%d/WireProfileBLR_%d", dev, dev);
    obtitle.Form("Chamber %d Wire Profile BLR", dev);
    hWireProfileBLR[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");

    obname.Form("Mapped/Filtered/Chamber%d/WireProfileFPGA_%d", dev, dev);
    obtitle.Form("Chamber %d Wire Profile FPGA", dev);
    hWireProfileFPGA[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");

    obname.Form("Mapped/Raw/Chamber%d/SumWireProfile_%d", dev, dev);
     obtitle.Form("Chamber %d Wire Profile accumulated", dev);
     hWireProfile_Sum[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");

     obname.Form("Mapped/BLR/Chamber%d/SumWireProfileBLR_%d", dev, dev);
     obtitle.Form("Chamber %d Wire Profile BLR accumulated", dev);
     hWireProfileBLR_Sum[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");

     obname.Form("Mapped/Filtered/Chamber%d/SumWireProfileFPGA_%d", dev, dev);
     obtitle.Form("Chamber %d Wire Profile FPGA accumulated", dev);
     hWireProfileFPGA_Sum[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");



  }    // devs

#endif

#ifdef USE_AWAGS_BEAMMONITOR


  obname.Form("Beam/BeamPositionMapTrace");
  obtitle.Form("Beam centroid from last trace");
  hBeamPosition=MakeTH2('I', obname.Data(), obtitle.Data(), AWAGS_BEAMPOS_BINS, -AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_BINS,  -AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_RANGE,"X (rel.)", "Y (rel.)");

  obname.Form("Beam/BeamPositionMapAcc");
  obtitle.Form("Beam centroids accumulated");
  hBeamPositionAcc=MakeTH2('I', obname.Data(), obtitle.Data(), AWAGS_BEAMPOS_BINS, -AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_BINS,  -AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_RANGE,"X (rel.)", "Y (rel.)");

  obname.Form("Beam/BeamPositionX");
  obtitle.Form("Beam positions X accumulated");
  hBeamXAcc=MakeTH1('I', obname.Data(), obtitle.Data(), AWAGS_BEAMPOS_BINS_FINE, -AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_RANGE, "X (rel.)");

  obname.Form("Beam/BeamPositionY");
  obtitle.Form("Beam positions X accumulated");
  hBeamYAcc=MakeTH1('I', obname.Data(), obtitle.Data(), AWAGS_BEAMPOS_BINS_FINE, -AWAGS_BEAMPOS_RANGE, AWAGS_BEAMPOS_RANGE, "Y (rel.)");

  obname.Form("Beam/BeamIntensity");
  obtitle.Form("Beam Intensity accumulated");
  hBeamIntensityAcc=MakeTH1('I', obname.Data(), obtitle.Data(), 1000, 0, 8000, "Electrode sum beam intensity (a.u.)");
#endif


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
#ifdef   USE_CSA_MAPPING
// fill mapped traces using the mapping here:
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

            hWireProfile[dev]->Fill(wire, theTrace[bin]);
            hWireProfile_Sum[dev]->Fill(wire, theTrace[bin]);
            hMapTrace[dev]->Fill(bin, wire, theTrace[bin]);
            hMapTrace_Sum[dev]->Fill(bin, wire, theTrace[bin]);
            //out_evt->fWireTrace[dev][wire].push_back(theTrace[bin]);

          }

          std::vector<Double_t> & theTraceBLR=inp_evt->fTraceBLR[sfp][slave][chan];
          for(size_t bin=0; bin<theTraceBLR.size(); ++bin)
          {
            hWireTracesBLR[dev][wire]->SetBinContent(1+bin,theTraceBLR[bin]);
            hWireProfileBLR[dev]->Fill(wire, theTraceBLR[bin]);
            hWireProfileBLR_Sum[dev]->Fill(wire, theTraceBLR[bin]);
            hMapTraceBLR[dev]->Fill(bin, wire, theTraceBLR[bin]);
            hMapTraceBLR_Sum[dev]->Fill(bin, wire, theTraceBLR[bin]);
            //out_evt->fWireTraceBLR[dev][wire].push_back(theTrace[bin]);
          }

          std::vector<Double_t> & theTraceFPGA=inp_evt->fTraceFPGA[sfp][slave][chan];
          for(size_t bin=0; bin<theTraceFPGA.size(); ++bin)
          {
            hWireTracesFPGA[dev][wire]->SetBinContent(1+bin,theTraceFPGA[bin]);
            hWireProfileFPGA[dev]->Fill(wire, theTraceFPGA[bin]);
            hWireProfileFPGA_Sum[dev]->Fill(wire, theTraceFPGA[bin]);
            hMapTraceFPGA[dev]->Fill(bin, wire, theTraceFPGA[bin]);
            hMapTraceFPGA_Sum[dev]->Fill(bin, wire, theTraceFPGA[bin]);
            //out_evt->fWireTraceFPGA[dev][wire].push_back(theTrace[bin]);
          }


        }

      }
#endif


#ifdef USE_AWAGS_BEAMMONITOR
/////////////////////////////////////////////////////////////
   // JAM 7-Dec-2023 here evaluate beam position from pickup electrodes:

   Int_t xlowSFP=fParam->fBeamMonitorSFP[0];
   Int_t xlowSlave=fParam->fBeamMonitorSlave[0];
   Int_t xlowChan=fParam->fBeamMonitorChannel[0];
   Int_t xhiSFP=fParam->fBeamMonitorSFP[1];
   Int_t xhiSlave=fParam->fBeamMonitorSlave[1];
   Int_t xhiChan=fParam->fBeamMonitorChannel[1];
   Int_t ylowSFP=fParam->fBeamMonitorSFP[2];
   Int_t ylowSlave=fParam->fBeamMonitorSlave[2];
   Int_t ylowChan=fParam->fBeamMonitorChannel[2];
   Int_t yhiSFP=fParam->fBeamMonitorSFP[3];
   Int_t yhiSlave=fParam->fBeamMonitorSlave[3];
   Int_t yhiChan=fParam->fBeamMonitorChannel[3];

   if(xlowSFP <0 || xlowSFP>MAX_SFP || xlowSlave <0 || xlowSlave>MAX_SLAVE || xlowChan < 0 ||  xlowChan> N_CHA)
   {
     GO4_STOP_ANALYSIS_MESSAGE("Configuration error of beam position electrode x low indices! (%d,%d,%d) - stopped. Check Map parameter! ", xlowSFP, xlowSlave, xlowChan);
   }
   if(xhiSFP <0 || xhiSFP>MAX_SFP || xhiSlave <0 || xhiSlave>MAX_SLAVE || xhiChan < 0 ||  xhiChan> N_CHA)
    {
        GO4_STOP_ANALYSIS_MESSAGE("Configuration error of beam position electrode x hi indices! (%d,%d,%d) - stopped. Check Map parameter! ", xhiSFP, xhiSlave, xhiChan);
    }
   if(ylowSFP <0 || ylowSFP>MAX_SFP || ylowSlave <0 || ylowSlave>MAX_SLAVE || ylowChan < 0 ||  ylowChan> N_CHA)
     {
       GO4_STOP_ANALYSIS_MESSAGE("Configuration error of beam position electrode y low indices! (%d,%d,%d) - stopped. Check Map parameter! ", ylowSFP, ylowSlave, ylowChan);
     }
   if(yhiSFP <0 || yhiSFP>MAX_SFP || yhiSlave <0 || yhiSlave>MAX_SLAVE || yhiChan < 0 ||  yhiChan> N_CHA)
      {
     GO4_STOP_ANALYSIS_MESSAGE("Configuration error of beam position electrode y hi indices! (%d,%d,%d) - stopped. Check Map parameter! ", yhiSFP, yhiSlave, yhiChan);
      }

   Double_t SigXlow=inp_evt->fSignal[xlowSFP][xlowSlave][xlowChan];
   Double_t SigXhi=inp_evt->fSignal[xhiSFP][xhiSlave][xhiChan];
   Double_t SigYlow=inp_evt->fSignal[ylowSFP][ylowSlave][ylowChan];
   Double_t SigYhi=inp_evt->fSignal[yhiSFP][yhiSlave][yhiChan];
   Double_t BackXlow=inp_evt->fBackground[xlowSFP][xlowSlave][xlowChan];
   Double_t BackXhi=inp_evt->fBackground[xhiSFP][xhiSlave][xhiChan];
   Double_t BackYlow=inp_evt->fBackground[ylowSFP][ylowSlave][ylowChan];
   Double_t BackYhi=inp_evt->fBackground[yhiSFP][yhiSlave][yhiChan];


   Double_t SigMinusBackXlow=TMath::Abs(SigXlow - BackXlow);
   Double_t SigMinusBackXhi=TMath::Abs(SigXhi - BackXhi);
   Double_t SigMinusBackYlow=TMath::Abs(SigYlow - BackYlow);
   Double_t SigMinusBackYhi=TMath::Abs(SigYhi - BackYhi);

   // use sum of opposite signals for intensity normalisation:
   Double_t Xintensity=SigMinusBackXlow+SigMinusBackXhi;
   Double_t Yintensity=SigMinusBackYlow+SigMinusBackYhi;

   Double_t xlownorm=SigMinusBackXlow/Xintensity; // between 0 and 1
   //Double_t xhiwnorm=SigMinusBackXhi/Xintensity; // redundant
   Double_t ylownorm=SigMinusBackYlow/Yintensity;
   //Double_t yhiwnorm=SigMinusBackYhi/Yintensity;; // redundant

   Double_t xpos= 2*AWAGS_BEAMPOS_RANGE*xlownorm -AWAGS_BEAMPOS_RANGE ;// 0 is in centre of plot, border scaled to AWAGS_BEAMPOS_RANGE
   Double_t ypos= 2*AWAGS_BEAMPOS_RANGE*ylownorm -AWAGS_BEAMPOS_RANGE ;// 0 is in centre of plot, border scaled to AWAGS_BEAMPOS_RANGE

   hBeamPosition->Fill(xpos,ypos);
   hBeamPositionAcc->Fill(xpos,ypos);
   hBeamXAcc->Fill(xpos);
   hBeamYAcc->Fill(ypos);
   hBeamIntensityAcc->Fill(Xintensity + Yintensity);

#endif
   out_evt->SetValid(isValid);

   if (fParam->fSlowMotion)
        {
            Int_t evnum = -1;
               TGo4MbsEvent* mbsevent = dynamic_cast<TGo4MbsEvent*>(GetInputEvent("Basic"));
               if (mbsevent)
                 evnum = mbsevent->GetCount();
            GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
        }

   return isValid;
}


void TGemCSAMapProc::ResetTraces()
{

#ifdef   USE_CSA_MAPPING
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

         hWireProfile[dev]->Reset("");
         hWireProfileBLR[dev]->Reset("");
         hWireProfileFPGA[dev]->Reset("");
       }

#endif

#ifdef USE_AWAGS_BEAMMONITOR
  hBeamPosition->Reset("");
#endif

}
