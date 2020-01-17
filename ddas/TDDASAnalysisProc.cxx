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

#include "TDDASAnalysisProc.h"
#include "TDDASFilterProc.h"

#include <string>

#include "TClass.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TMath.h"
#include "TString.h"

#include "TGo4Log.h"
//#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"

#include "TDDASAnalysisEvent.h"
//#include "TDDASRawEvent.h"

#include "TDDASFilterProc.h"
#include "TDDASAnalysisParameter.h"

//***********************************************************
TDDASAnalysisProc::TDDASAnalysisProc() :
    TGo4EventProcessor(), fInput(0), fOutput(0)

{
}
//***********************************************************
TDDASAnalysisProc::TDDASAnalysisProc(const char* name) :
    TGo4EventProcessor(name), fInput(0), fOutput(0)
{
  TGo4Log::Info("TDDASAnalysisProc: Create %s", name);
  //// init user analysis objects:

  fAnaPar = dynamic_cast<TDDASAnalysisParameter*>(MakeParameter("DDASAnalysisParameter", "TDDASAnalysisParameter",
      "set_DDASAnalysisParameter.C"));

  fBdecay.Reset();
  fBdecayVars.Initialize();
  fBdecayVars.ReadDSSD();

  fBdecayVars.ReadOther();
  fBdecayVars.ReadSuN();
  fBdecayVars.tac.i2ncorrpos = (-1) * fBdecayVars.tac.i2ncorrpos;

  //the other parameters we get from Go4 parameter object:

  // set gate in correlator:
  std::string gate = fAnaPar->fGateName.GetString().Data();
  // here lets create a Go4 polygon condition to provide the TCutG of that name:
  // JAM2020: following example values taken from Fe51corr.C
  Double_t cutpnts[12][2] =  {
      { 10314.7, 9175.03} ,
      { 10250.9 , 9429.78},
      { 10241.4,9693.02},
      { 10321.1,9871.34},
      { 10400.7,9956.26},
      { 10502.8,9947.76},
      { 10569.7,9718.49},
      { 10576.1,9480.73},
      { 10509.1,9217.49},
      { 10413.5,9132.57},
      { 10333.8,9149.56},
      { 10314.7,9175.03}  };

  fExampleGate = MakePolyCond(gate.c_str(), 12, cutpnts);

  int gateselect = fCorrelator.SelectGate(gate);
  // NOTE: the version of Correlator class contained in this Go4 user package has the gate always disabled
  // Please change it if you want to use this.

  // TODO: put the pid histogram for this gating cut into Go4


  //set correlation time
  fBdecayVars.clock.max = fAnaPar->fClockMax;

  //set minimum implant time
  fBdecayVars.corr.minimplant = fAnaPar->fMinImplantTime;
  //
  // set random flag
  int randomflag = fAnaPar->fRandomFlag;
  fCorrelator.SetRandomFlag(randomflag, fBdecayVars);

  /////////// HISTOGRAMS
  TString obname;
  TString obtitle;
  // JAM2020: TODO please check histogram ranges as defined in TDDASRawEvent.h !!

  for (Int_t strip = 0; strip < GO4_DDAS_DSSD_STRIPS; ++strip)
  {
    obname.Form("Analyzed/DSSDFront/High/Ecal/Front_HiE_cal_%d", strip);
    obtitle.Form("Analyzed Front Hi E cal %d", strip);
    hFront_hiecal[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE,
        "calibrated energy");

    obname.Form("Analyzed/DSSDFront/Low/Ecal/Front_LoE_cal_%d", strip);
    obtitle.Form("Analyzed Front Low E cal %d", strip);
    hFront_loecal[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE,
        "calibrated energy");

    obname.Form("Analyzed/DSSDFront/High/E/Front_HiE_%d", strip);
    obtitle.Form("Analyzed Front Hi Energy %d", strip);
    hFront_hienergy[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "energy");

    obname.Form("Analyzed/DSSDFront/Low/E/Front_LoE_%d", strip);
    obtitle.Form("Analyzed Front Low Energy %d", strip);
    hFront_loenergy[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "energy");

    obname.Form("Analyzed/DSSDBack/High/Ecal/Back_HiE_cal_%d", strip);
    obtitle.Form("Analyzed Back Hi E cal %d", strip);
    hBack_hiecal[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE,
        "calibrated energy");

    obname.Form("Analyzed/DSSDBack/Low/Ecal/Back_LoE_cal_%d", strip);
    obtitle.Form("Analyzed Back Low E cal %d", strip);
    hBack_loecal[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE,
        "calibrated energy");

    obname.Form("Analyzed/DSSDBack/High/E/Back_HiE_%d", strip);
    obtitle.Form("Analyzed Back Hi Energy %d", strip);
    hBack_hienergy[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "energy");

    obname.Form("Analyzed/DSSDBack/Low/E/Back_LoE_%d", strip);
    obtitle.Form("Analyzed Back Low Energy %d", strip);
    hBack_loenergy[strip] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "energy");

  }    // for strips
       /////////////////////////////////////////////////

  obname.Form("Analyzed/DSSDFront/Front_Icent");
  obtitle.Form("Analyzed Front Icent");
  hFront_icent = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDFront/Front_Dmax");
  obtitle.Form("Analyzed Front Dmax");
  hFront_dmax = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDFront/Front_Imax");
  obtitle.Form("Analyzed Front Imax");
  hFront_imax = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDFront/Front_Imax_time");
  obtitle.Form("Analyzed Front Imax time");
  hFront_imaxtime = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_TIMERANGE, 0, GO4_DDAS_TIMERANGE);

  obname.Form("Analyzed/DSSDFront/Front_Dmax_time");
  obtitle.Form("Analyzed Front Dmax time");
  hFront_dmaxtime = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_TIMERANGE, 0, GO4_DDAS_TIMERANGE);

  obname.Form("Analyzed/DSSDFront/Front_Imax_ch");
  obtitle.Form("Analyzed Front Imax chan");
  hFront_imaxch = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_CHANNELS, 0, GO4_DDAS_CHANNELS);

  obname.Form("Analyzed/DSSDFront/Front_Dmax_ch");
  obtitle.Form("Analyzed Front Dmax chan");
  hFront_dmaxch = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_CHANNELS, 0, GO4_DDAS_CHANNELS);

  obname.Form("Analyzed/DSSDFront/Front_Imult");
  obtitle.Form("Analyzed Front Imult");
  hFront_imult = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_MULTRANGE, 0, GO4_DDAS_MULTRANGE);
  // TODO: check range of multiplicties JAM2020

  obname.Form("Analyzed/DSSDFront/Front_Dmult");
  obtitle.Form("Analyzed Front Dmult");
  hFront_dmult = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_MULTRANGE, 0, GO4_DDAS_MULTRANGE);
  // TODO: check range of multiplicties JAM2020

  obname.Form("Analyzed/DSSDFront/Front_Isum");
  obtitle.Form("Analyzed Front Isum");
  hFront_isum = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDFront/Front_Dsum");
  obtitle.Form("Analyzed Front Dsum");
  hFront_dsum = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Icent");
  obtitle.Form("Analyzed Back Icent");
  hBack_icent = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Dmax");
  obtitle.Form("Analyzed Back Dmax");
  hBack_dmax = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Imax");
  obtitle.Form("Analyzed Back Imax");
  hBack_imax = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Imax_time");
  obtitle.Form("Analyzed Back Imax time");
  hBack_imaxtime = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_TIMERANGE, 0, GO4_DDAS_TIMERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Dmax_time");
  obtitle.Form("Analyzed Back Dmax time");
  hBack_dmaxtime = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_TIMERANGE, 0, GO4_DDAS_TIMERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Imax_ch");
  obtitle.Form("Analyzed Back Imax chan");
  hBack_imaxch = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_CHANNELS, 0, GO4_DDAS_CHANNELS);

  obname.Form("Analyzed/DSSDBack/Back_Dmax_ch");
  obtitle.Form("Analyzed Back Dmax chan");
  hBack_dmaxch = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_CHANNELS, 0, GO4_DDAS_CHANNELS);

  obname.Form("Analyzed/DSSDBack/Back_Imult");
  obtitle.Form("Analyzed Back Imult");
  hBack_imult = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_MULTRANGE, 0, GO4_DDAS_MULTRANGE);
  // TODO: check range of multiplicties JAM2020

  obname.Form("Analyzed/DSSDBack/Back_Dmult");
  obtitle.Form("Analyzed Back Dmult");
  hBack_dmult = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_MULTRANGE, 0, GO4_DDAS_MULTRANGE);
  // TODO: check range of multiplicties JAM2020

  obname.Form("Analyzed/DSSDBack/Back_Isum");
  obtitle.Form("Analyzed Back Isum");
  hBack_isum = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

  obname.Form("Analyzed/DSSDBack/Back_Dsum");
  obtitle.Form("Analyzed Back Dsum");
  hBack_dsum = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE);

}
//***********************************************************
TDDASAnalysisProc::~TDDASAnalysisProc()
{

}
//***********************************************************

//Bool_t TDDASAnalysisProc::CheckEventClass(TClass* cl)
//{
//   return cl->InheritsFrom(TDDASAnalysisEvent::Class());
//}
//

//-----------------------------------------------------------

Bool_t TDDASAnalysisProc::BuildEvent(TGo4EventElement* dest)
{

  fInput = dynamic_cast<TDDASRawEvent*>(GetInputEvent());
  fOutput = dynamic_cast<TDDASAnalysisEvent*>(dest);

  if ((fInput == 0) || (fOutput == 0))
  {
    TGo4Log::Error("DDASAnalysisProc: events are not specified!");
    return kFALSE;
  }

  if (!fInput->IsValid())
    return kTRUE;    // just skip event if filtered out by previous step

  // fill poutevt here:
  DdasToEvent();
  FillHistograms();
  return kTRUE;
}

void TDDASAnalysisProc::DdasToEvent()
{
  std::vector<ddaschannel*>& channellist = fInput->fDDASEvent.GetData();
  std::vector<ddaschannel*>::iterator channellist_it;

  fBdecay.Reset();
  fBdecayVars.hit.Initialize();

  //int timewindow = 1000;
  double starttime = 0, endtime = 0;

//    double i2ntofout = -1;
//    double pin01eout = -1;
//    double i2posout = -1;
//    double     i2ncorrout = -1;
//    double pin01ecorr = -1;
//    double ide1 = -1;
//    double itof1 = -1;
//    double itof2 = -1;
//    double dde1 = -1;
//    double dtof1 = -1;
//    double dtof2 = -1;
//    double dtimecal = -1;
//    double gde1 = -1;
//    double gtof = -1;
//    double gtimecal = -1;
//    double fbtime = -1;

  double fdmax = -1;
  double bdmax = -1;
  double ftdc = -1;
//    int flag = -1;
  double impdec = -1;
//    double dtdc[17];

  bool overflow = false;

  int eventsize = channellist.size();

  fBdecay.ddasdiagnostics.cmult = eventsize;

  channellist_it = channellist.begin();

  for (int i = 1; i <= 18; i++)
  {
    fBdecay.ddasdiagnostics.adchit[i] = 0;
  }

  for (channellist_it = channellist.begin(); channellist_it < channellist.end(); channellist_it++)
  {
    int crateid = (*channellist_it)->crateid;
    int slotid = (*channellist_it)->slotid;
    int channum = (*channellist_it)->chanid;
    double current_time;

    /* The time of an event will be taken as the time of the first
     channel in the event. */

    if (channellist_it == channellist.begin())
    {
      /* Calculate time difference between events. */
      double tdiffevent = ((*channellist_it)->time) - starttime;
      tdiffevent = tdiffevent / 1000.; /* Microseconds time difference. */
      fBdecay.ddasdiagnostics.tdiffevent = tdiffevent;

      starttime = (*channellist_it)->time;
      fBdecay.clock.current = ((*channellist_it)->GetTime()) / 1000000.;
    }

    /* Unpack data from crate 0 -- check this if code fails e17009 is crate 1...*/
    if (crateid == 3)
    {
      unsigned int adcnumber = slotid - 1;

      fBdecay.ddasdiagnostics.adchit[adcnumber] += pow(2., (double) (*channellist_it)->chanid);
      fBdecay.adc[adcnumber].channel[channum] = ((*channellist_it)->energy);
      fBdecay.time[adcnumber].timefull[channum] = ((*channellist_it)->time);
      fBdecay.time[adcnumber].timecfd[channum] = (*channellist_it)->GetCFDTime();
      fBdecay.ddasdiagnostics.overflow[channum] = (*channellist_it)->GetOverflowCode();
      fBdecay.ddasdiagnostics.finishcode[channum] = (*channellist_it)->GetFinishCode();

      //check for trace and exract
      if ((*channellist_it)->tracelength != 0)
      {
        fBdecay.adc[adcnumber].chantrace[channum].trace = (*channellist_it)->GetTrace();
      }
    }

    /* Map the channel, calibrate and theshold check. */
    if ((slotid == 5 || slotid == 6) && ((*channellist_it)->GetOverflowCode()))
      overflow = true;

    // here we reuse the defined function of the first processor class:#include "TDDASFilterProc.h"
    TDDASFilterProc::MapChannels(crateid, slotid, channum, fBdecay, fBdecayVars);

    endtime = (*channellist_it)->time;

  } /* For entire channellist... */

  /* Calculate event length in 100 ns units. */
  fBdecay.ddasdiagnostics.eventlength = (endtime - starttime) * 100;

  /* Channels within an event have been unpacked into appropriate
   fBdecay parameters and hit flags have been set.  Maximum channels
   have already been established for the Si detectors in MapChannels. */

  /* Begin event level processing. */
  // pin01.ecorr will need to be changed because i2 is not used in e17009
  fBdecay.pin01.ecorr = fBdecay.pin01.ecal + 3000;    //+(fBdecay.tac.i2pos*-0.9)+3000;
  fBdecay.pin02.ecorr = fBdecay.pin02.ecal + 3000;    //+(fBdecay.tac.i2pos*-1.01)+3000;

  //modified for e17009
  fBdecay.pid.de1 = fBdecay.pin01.ecal;
  fBdecay.pid.de2 = fBdecay.pin02.ecal;

  // will need to update to XFP timing for e17009
  fBdecay.tac.i2pos_ns = ((fBdecay.tac.i2pos - 3000) * 0.00568);
  fBdecay.tac.pin01i2n_ns = (fBdecay.tac.pin01i2n * 0.00565);
  fBdecay.tac.pin01i2s_ns = (fBdecay.tac.pin01i2s * 0.00573);
  fBdecay.tac.pin02i2n_ns = (fBdecay.tac.pin02i2n * 0.00597);
  fBdecay.tac.calc_i2pos = TMath::Abs(fBdecay.tac.pin01i2s_ns - fBdecay.tac.pin01i2n_ns);
  fBdecay.tac.calc_i2pos_mixed = TMath::Abs(fBdecay.tac.pin01i2s_ns - fBdecay.tac.pin02i2n_ns);

  //cout << "correction: " << fBdecayVars.tac.i2ncorrpos << " other: " << fBdecayVars.tac.pin01corr << endl;
  //added condition to remove the "bad" i2pos TAC events (3000 is added to value when it's set)
  if (fBdecay.tac.i2pos > 3000)
  {
    fBdecay.tac.tof1_corr = (fBdecay.tac.pin01i2n * fBdecayVars.tac.pin01corr)
        - (fBdecayVars.tac.i2ncorrpos * fBdecay.tac.i2pos) + 1500;
    fBdecay.tac.tof2_corr = (fBdecay.tac.pin02i2n * fBdecayVars.tac.pin02corr)
        - (fBdecayVars.tac.i2ncorrpos * fBdecay.tac.i2pos) + 1500;
  }
  fBdecay.pid.pin01i2n = fBdecay.tac.tof1_corr;
  fBdecay.pid.pin01i2s = fBdecay.tac.pin01i2s;

  // MakeCuts();

  // if(Cr_band_raw_pins->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal) ||
  //    Mn_band_raw_pins_big->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal) ||
  //    Fe_band_raw_pins_big->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal) ||
  //    Co_band_raw_pins_big->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal) ||
  //    Ni_band_raw_pins_big->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal) ||
  //    Cu_band_raw_pins_big->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal) ||
  //    Zn_band_raw_pins_big->IsInside(fBdecay.tac.pin02i2n,fBdecay.pin01.ecal+fBdecay.pin02.ecal))
  //   {
  //cout << "element band" << endl;
  // USE THESE VARIABLES FOR PID -- includes element band gates and I2 pos gate to eliminate smaller distributions that multiply
  //   if(de1i2pos_big_right->IsInside(fBdecay.tac.i2pos,fBdecay.pin01.ecal))
  //  {
  //    //cout << "i2pos" << endl;
  //    fBdecay.pid.de1 = fBdecay.pin01.ecorr;
  //    fBdecay.pid.pin02i2n = fBdecay.tac.tof2_corr;
  //  }
  // }

  // calibrate and sum the sun spectra
  int sunmult = 0;
  for (int seg = 0; seg < 8; seg++)
  {
    for (int z = 0; z < 3; z++)
    {
      fBdecay.sun.rseg[seg] += fBdecay.sun.gmpmt[seg * 3 + z];
    }
    fBdecay.sun.cseg[seg] = fBdecayVars.SuN.square[seg] * fBdecay.sun.rseg[seg] * fBdecay.sun.rseg[seg]
        + fBdecayVars.SuN.slope[seg] * fBdecay.sun.rseg[seg] + fBdecayVars.SuN.intercept[seg];

    if (fBdecay.sun.cseg[seg] > fBdecayVars.SuN.thresh[seg])
    {
      fBdecayVars.hit.sun = 1;
      fBdecay.sun.totalmult = ++sunmult;
      fBdecay.sun.total += fBdecay.sun.cseg[seg];
      if (seg == 1 || seg == 2 || seg == 5 || seg == 6)
      {
        fBdecay.sun.midseg_total += fBdecay.sun.cseg[seg];
      }
    }
  }

  fBdecay.sun.total_cal = fBdecayVars.SuN.tot_square * fBdecay.sun.total * fBdecay.sun.total
      + fBdecayVars.SuN.tot_slope * fBdecay.sun.total + fBdecayVars.SuN.tot_intercept;
  fBdecay.sun.midseg_total_cal = fBdecayVars.SuN.tot_square * fBdecay.sun.midseg_total * fBdecay.sun.midseg_total
      + fBdecayVars.SuN.tot_slope * fBdecay.sun.midseg_total + fBdecayVars.SuN.tot_intercept;

  //fOutput->fData.Reset();

  //if the high gain side of the dssd hit overflow, shut down the correlations
  if (overflow)
  {
    fBdecay.front.dmaxch = -1;
    fBdecay.back.dmaxch = -1;
    fBdecayVars.hit.fronthi = 0;
    fBdecayVars.hit.backhi = 0;
  }

  /* Correlate this event. */
  impdec = fCorrelator.Correlate(fBdecay, fBdecayVars, 16, 16, 1);
  fOutput->fData.corr = fBdecay.corr;

  if (fBdecay.corr.dtimecal > 0 && impdec < 0)
  {
    // cout << "wtf1 " << fBdecay.corr.dtimecal << " " << impdec << endl;
  }
  //Int_t temp;

  if (fBdecay.corr.dtimecal > 0 && impdec < 0)
  {
    //  cout << "wtf2 " << fBdecay.corr.dtimecal << " " << impdec << endl;
  }

  /* Put selected data into root structure for write. */
  fOutput->fData.ddasdiagnostics = fBdecay.ddasdiagnostics;
  fOutput->fData.clock = fBdecay.clock;
  fOutput->fData.tac = fBdecay.tac;
  fOutput->fData.pin01 = fBdecay.pin01;
  fOutput->fData.pin02 = fBdecay.pin02;
  //fOutput->fData.pin03 = fBdecay.pin03;
  fOutput->fData.veto = fBdecay.veto;
  fOutput->fData.pid = fBdecay.pid;
  fOutput->fData.sun = fBdecay.sun;
  fOutput->fData.front = fBdecay.front;
  fOutput->fData.back = fBdecay.back;

  fOutput->fData.front.icent = fBdecay.front.icent;
  fOutput->fData.front.dmax = fBdecay.front.dmax;
  fOutput->fData.front.imax = fBdecay.front.imax;
  fOutput->fData.front.imaxtime = fBdecay.front.imaxtime;
  fOutput->fData.front.dmaxtime = fBdecay.back.dmaxtime;
  fOutput->fData.front.imaxch = fBdecay.front.imaxch;
  fOutput->fData.front.dmaxch = fBdecay.front.dmaxch;
  fOutput->fData.front.imult = fBdecay.front.imult;
  fOutput->fData.front.dmult = fBdecay.front.dmult;
  fOutput->fData.front.isum = fBdecay.front.isum;
  fOutput->fData.front.dsum = fBdecay.front.dsum;

  fOutput->fData.back.icent = fBdecay.back.icent;
  fOutput->fData.back.dmax = fBdecay.back.dmax;
  fOutput->fData.back.imax = fBdecay.back.imax;
  fOutput->fData.back.imaxtime = fBdecay.back.imaxtime;
  fOutput->fData.back.dmaxtime = fBdecay.back.dmaxtime;
  fOutput->fData.back.imaxch = fBdecay.back.imaxch;
  fOutput->fData.back.dmaxch = fBdecay.back.dmaxch;
  fOutput->fData.back.vimaxch = fBdecay.back.vimaxch;
  fOutput->fData.back.vdmaxch = fBdecay.back.vdmaxch;
  fOutput->fData.back.imult = fBdecay.back.imult;
  fOutput->fData.back.dmult = fBdecay.back.dmult;
  fOutput->fData.back.isum = fBdecay.back.isum;
  fOutput->fData.back.dsum = fBdecay.back.dsum;

  for (int z = 0; z < 16; z++)
  {
    fOutput->fData.front.hiecal[z] = fBdecay.front.hiecal[z];
    fOutput->fData.back.hiecal[z] = fBdecay.back.hiecal[z];
    fOutput->fData.front.loecal[z] = fBdecay.front.loecal[z];
    fOutput->fData.back.loecal[z] = fBdecay.back.loecal[z];
    fOutput->fData.front.hienergy[z] = fBdecay.front.hienergy[z];
    fOutput->fData.back.hienergy[z] = fBdecay.back.hienergy[z];
    fOutput->fData.front.loenergy[z] = fBdecay.front.loenergy[z];
    fOutput->fData.back.loenergy[z] = fBdecay.back.loenergy[z];
  }

  // ide1 = fBdecay.corr.ide1;
  // itof1 = fBdecay.corr.itof1;
  // itof2 = fBdecay.corr.itof2;
  // dde1 = fBdecay.corr.dde1;
  // dtof1 = fBdecay.corr.dtof1;
  // dtof2 = fBdecay.corr.dtof2;
  // dtimecal= fBdecay.corr.dtimecal;
  // gde1 = fBdecay.corr.gde1;
  // gtof = fBdecay.corr.gtof;
  // gtimecal= fBdecay.corr.gtimecal;
  // flag = fBdecay.corr.flag;

  if (fBdecay.back.dmaxch > 0 && fBdecay.back.dmaxch < 17)
  {
    ftdc = fBdecay.back.hieventtdc[fBdecay.back.dmaxch];
  }

  fdmax = fBdecay.front.dmax;
  bdmax = fBdecay.back.dmax;

  // JAM2020: probably additional conditions here when to write it out?
  fOutput->SetValid(kTRUE);

}

void TDDASAnalysisProc::FillHistograms()
{
  // here do some first histograming of data that is in output event structure
  if (!fOutput->IsValid())
    return;

  hFront_icent->Fill(fOutput->fData.front.icent);
  hFront_dmax->Fill(fOutput->fData.front.dmax);
  hFront_imax->Fill(fOutput->fData.front.imax);
  hFront_imaxtime->Fill(fOutput->fData.front.imaxtime);
  hFront_dmaxtime->Fill(fOutput->fData.front.dmaxtime);
  hFront_imaxch->Fill(fOutput->fData.front.imaxch);
  hFront_dmaxch->Fill(fOutput->fData.front.dmaxch);
  hFront_imult->Fill(fOutput->fData.front.imult);
  hFront_dmult->Fill(fOutput->fData.front.dmult);
  hFront_isum->Fill(fOutput->fData.front.isum);
  hFront_dsum->Fill(fOutput->fData.front.dsum);

  hBack_icent->Fill(fOutput->fData.back.icent);
  hBack_dmax->Fill(fOutput->fData.back.dmax);
  hBack_imax->Fill(fOutput->fData.back.imax);
  hBack_imaxtime->Fill(fOutput->fData.back.imaxtime);
  hBack_dmaxtime->Fill(fOutput->fData.back.dmaxtime);
  hBack_imaxch->Fill(fOutput->fData.back.imaxch);
  hBack_dmaxch->Fill(fOutput->fData.back.dmaxch);
  hBack_imult->Fill(fOutput->fData.back.imult);
  hBack_dmult->Fill(fOutput->fData.back.dmult);
  hBack_isum->Fill(fOutput->fData.back.isum);
  hBack_dsum->Fill(fOutput->fData.back.dsum);

  for (int z = 0; z < GO4_DDAS_DSSD_STRIPS; z++)
  {
    hFront_hiecal[z]->Fill(fOutput->fData.front.hiecal[z]);
    hBack_hiecal[z]->Fill(fOutput->fData.back.hiecal[z]);
    hFront_loecal[z]->Fill(fOutput->fData.front.loecal[z]);
    hBack_loecal[z]->Fill(fOutput->fData.back.loecal[z]);
    hFront_hienergy[z]->Fill(fOutput->fData.front.hienergy[z]);
    hBack_hienergy[z]->Fill(fOutput->fData.back.hienergy[z]);
    hFront_loenergy[z]->Fill(fOutput->fData.front.loenergy[z]);
    hBack_loenergy[z]->Fill(fOutput->fData.back.loenergy[z]);
  }

}

