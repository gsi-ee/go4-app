

#include "TAwagsSisMapProc.h"

#include <math.h>
#include <vector>

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"

#include "TGo4WinCond.h"
#include "TGo4Analysis.h"

#include "TAwagsSisMapEvent.h"
#include "TAwagsSisBasicEvent.h"
#include "TAwagsSisMapParam.h"
#include "TAwagsSisParam.h"
//-----------------------------------------------------------
TAwagsSisMapProc::TAwagsSisMapProc() :
   TGo4EventProcessor()
{
}

//-----------------------------------------------------------
TAwagsSisMapProc::TAwagsSisMapProc(const char* name) :
   TGo4EventProcessor(name),
   fParam(0)
{
   TGo4Log::Info("TAwagsSisMapProc: Create");
   //// init user analysis objects:
   SetMakeWithAutosave(kTRUE);
   // here already exisitng parameter will be returned
   // one not need to specify macro here - it is already executed in first step
   fParam = (TAwagsSisMapParam*) MakeParameter("AwagsSisMapParam", "TAwagsSisMapParam", "set_AwagsSisMapParam.C");
   TString obname;
   TString obtitle;
   //TString foldername;
   Int_t tracelength=CSA_TRACE_SIZE; // TODO: dynamically get trace size from first step?
   Int_t spillsamples=200;
   TAwagsSisParam* firstparam=dynamic_cast<TAwagsSisParam*>(GetParameter("AwagsSisParam"));
   if(firstparam)
   {
     spillsamples=firstparam->fNumEventsPerSpill;
     printf("QQQQ took number of events per spill %d from parameter %s", spillsamples, firstparam->GetName());
   }


  for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
  {
    for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
    {

      obname.Form("Mapped/Chamber%d/Traces/SpillTrace_%d_%d", dev, dev, wire);
      obtitle.Form("Chamber %d Wire %d Raw Traces during spill", dev, wire);
      hWireTraces[dev][wire] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)",
          "counts");

      obname.Form("Mapped/Chamber%d/Charge/ChargeTrend_%d_%d", dev, dev, wire);
      obtitle.Form("Chamber %d Wire %d Charge trend during spill", dev, wire);


      hWireSpillCharge[dev][wire] = MakeTH1('D', obname.Data(), obtitle.Data(), spillsamples, 0, spillsamples, "MBS Event sample index",
          "#Delta Q");



    }    // wires

    Int_t trbinsize= tracelength/10;
    obname.Form("Mapped/Chamber%d/OverviewSpillTrace_%d", dev, dev);
    obtitle.Form("Chamber %d Trace wire map", dev);
    hMapTrace[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");


    obname.Form("Mapped/Chamber%d/SumOverviewSpillTrace_%d", dev, dev);
    obtitle.Form("Chamber %d Trace wire map accumulated", dev);
    hMapTrace_Sum[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), trbinsize, 0, tracelength, CSA_MAXWIRES,  0, CSA_MAXWIRES,"time (bins)", "wire number", "counts");


    obname.Form("Mapped/Chamber%d/WireProfile_%d", dev, dev);
    obtitle.Form("Chamber %d Wire Profile", dev);
    hWireProfile[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");


    obname.Form("Mapped/Chamber%d/SumWireProfile_%d", dev, dev);
     obtitle.Form("Chamber %d Wire Profile accumulated", dev);
     hWireProfile_Sum[dev] = MakeTH1('I', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "counts");

     obname.Form("Mapped/Chamber%d/OverviewSpillCharge_%d", dev, dev);
     obtitle.Form("Chamber %d Charge wire map", dev);
     // check for dynamic recreation here:
      TH1* testhis = GetHistogram(obname.Data());
      if (testhis && (testhis->GetNbinsX() != spillsamples))
      {
        printf("QQQ MBS event range for Spill changed in parameter to %d, recreate spill charge histograms for chamber %d\n",
            spillsamples,dev);
        SetMakeWithAutosave(kFALSE);
      }
     hMapSpillCharge[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), spillsamples, 0, spillsamples, CSA_MAXWIRES,  0, CSA_MAXWIRES,"MBS Event sample", "wire number", "#Delta Q");

     obname.Form("Mapped/Chamber%d/Sum OverviewSpillCharge_%d", dev, dev);
     obtitle.Form("Chamber %d Charge wire map accumulated", dev);
     hMapSpillCharge_Sum[dev]=MakeTH2('I', obname.Data(), obtitle.Data(), spillsamples, 0, spillsamples, CSA_MAXWIRES,  0, CSA_MAXWIRES,"MBS Event sample", "wire number", "#Delta Q");
     SetMakeWithAutosave(kTRUE);

     obname.Form("Mapped/Chamber%d/WireChargeProfile_%d", dev, dev);
     obtitle.Form("Chamber %d Wire ChargeProfile", dev);
     hWireChargeProfile[dev] = MakeTH1('D', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "#Delta Q");

     obname.Form("Mapped/Chamber%d/SumWireChargeProfile_%d", dev, dev);
     obtitle.Form("Chamber %d Wire ChargeProfile accumulated", dev);
     hWireChargeProfile_Sum[dev] = MakeTH1('D', obname.Data(), obtitle.Data(),  CSA_MAXWIRES,  0, CSA_MAXWIRES, "wire number", "#Delta Q");




  }    // devs


}
//-----------------------------------------------------------
TAwagsSisMapProc::~TAwagsSisMapProc()
{
   TGo4Log::Info("TAwagsSisMapProc: Delete");

}
//-----------------------------------------------------------
Bool_t TAwagsSisMapProc::BuildEvent(TGo4EventElement* dest)
{
  TAwagsSisBasicEvent* inp_evt = (TAwagsSisBasicEvent*) GetInputEvent();
  TAwagsSisMapEvent* out_evt = (TAwagsSisMapEvent*) dest;
  out_evt->SetValid(kFALSE);
  if (inp_evt->IsValid())
  {
    ResetTraces();

    for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
    {
      for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
      {
        Int_t sfp = fParam->fSFP[dev][wire];
        Int_t slave = fParam->fSlave[dev][wire];
        Int_t chan = fParam->fChannel[dev][wire];
        if (sfp < 0 || slave < 0 || chan < 0)
          continue;
        if (sfp > MAX_SFP || slave > MAX_SLAVE || chan > N_CHA)
          continue;

        std::vector<Double_t> & theTrace = inp_evt->fSpillTrace[sfp][slave][chan];
        for (size_t bin = 0; bin < theTrace.size(); bin++)
        {
          if ((Int_t) bin >= hWireTraces[dev][wire]->GetNbinsX())
            break;    // avoid crash by accidental overflow
//          printf("TAwagsSisMapProc::BuildEvent for sfp %d slave %d chan %d access spilltrace bin %d , size is %ld\n",
//              sfp, slave, chan, bin, theTrace.size());
          if ((Int_t) (1 + bin) < hWireTraces[dev][wire]->GetNbinsX())
            {
              hWireTraces[dev][wire]->SetBinContent(1 + bin, theTrace[bin]);
            }
          hWireProfile[dev]->Fill(wire, theTrace[bin]);
          hWireProfile_Sum[dev]->Fill(wire, theTrace[bin]);
          hMapTrace[dev]->Fill(bin, wire, theTrace[bin]);
          hMapTrace_Sum[dev]->Fill(bin, wire, theTrace[bin]);
          //out_evt->fWireTrace[dev][wire].push_back(theTrace[bin]);

        }

        std::vector<Double_t> & theCharge = inp_evt->fChargeTrend[sfp][slave][chan];
        for (size_t bin = 0; bin < theCharge.size(); bin++)
        {
          if ((Int_t) bin >= hWireSpillCharge[dev][wire]->GetNbinsX())
                      break;    // avoid crash by accidental overflow
//          Double_t value=theCharge[bin];
//          printf("TAwagsSisMapProc::BuildEvent for sfp %d slave %d chan %d access charge bin %d , size is %ld, val=%e\n",
//                      sfp, slave, chan, bin, theCharge.size(), value);
//          continue;

          if ((Int_t) (1 + bin) < hWireSpillCharge[dev][wire]->GetNbinsX())
          {
            hWireSpillCharge[dev][wire]->SetBinContent(1 + bin, theCharge[bin]);
          }
          hWireChargeProfile[dev]->Fill(wire, theCharge[bin]);
          hWireChargeProfile_Sum[dev]->Fill(wire, theCharge[bin]);
          hMapSpillCharge[dev]->Fill(bin, wire, theCharge[bin]);
          hMapSpillCharge_Sum[dev]->Fill(bin, wire, theCharge[bin]);
        }
      }
    }

    out_evt->SetValid(kTRUE);
  }    // if(inp_evt->IsValid())

  if (fParam->fSlowMotion)
  {
    Int_t evnum = -1;
    TGo4MbsEvent* mbsevent = dynamic_cast<TGo4MbsEvent*>(GetInputEvent("Unpacker"));
    if (mbsevent)
      evnum = mbsevent->GetCount();
    GO4_STOP_ANALYSIS_MESSAGE(
        "Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
  }

  return kTRUE;
}


void TAwagsSisMapProc::ResetTraces()
{
  for (Int_t dev = 0; dev < CSA_MAXCHAMBERS; ++dev)
       {
         for (Int_t wire = 0; wire < CSA_MAXWIRES; ++wire)
         {
           hWireTraces[dev][wire]->Reset("");
           hWireSpillCharge[dev][wire]->Reset("");
         }
         hMapTrace[dev]->Reset("");
         hMapSpillCharge[dev]->Reset("");

         hWireProfile[dev]->Reset("");
         hWireChargeProfile[dev]->Reset("");
       }
}
