// N.Kurz, EE, GSI, 15-Jan-2010

//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------


/**
 * Analysis for the Febex readout of Awags at SIS tests in 2022
 * v.01 on 25-Jul-2022 by JAM - modify original Feb3Basic unpacker (j.adamczewski@gsi.de)
 *
 * */




#include "TAwagsSisProc.h"
#include "TAwagsSisParam.h"
#include "TAwagsSisBasicEvent.h"
#include "stdint.h"

#include "Riostream.h"
using namespace std;

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "snprintf.h"

#include "TGo4MbsEvent.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CondArray.h"
#include "TGo4Picture.h"

#include "TGo4UserException.h"

#ifdef USE_MBS_PARAM
 static UInt_t    l_tr    [MAX_TRACE_SIZE];
 static Double_t  f_tr_blr[MAX_TRACE_SIZE];
 static UInt_t    l_sfp_slaves[MAX_SFP] = {MAX_SLAVE, MAX_SLAVE, MAX_SLAVE, MAX_SLAVE};
 static UInt_t    l_slaves=0;
 static UInt_t    l_trace=0;
 static UInt_t    l_e_filt=0;
#else
 static UInt_t    l_tr    [TRACE_SIZE];
 static Double_t  f_tr_blr[TRACE_SIZE];
 static UInt_t    l_sfp_slaves  [MAX_SFP] = NR_SLAVES;
 static UInt_t    l_sfp_adc_type[MAX_SFP] = ADC_TYPE;
#endif //

static UInt_t    l_adc_type;

static UInt_t    l_evt_ct=0;
static UInt_t    l_evt_ct_phys=0;
static UInt_t    l_good_energy_ct=0;
static UInt_t    l_1_hit_ct=0;
static UInt_t    l_more_1_hit_ct=0;
static UInt_t    l_more_1_hit_first_energy_ok_ct=0;
static UInt_t    l_eeeeee_ct=0;
static UInt_t    l_eeeee1_ct=0;
static UInt_t    l_eeeee2_ct=0;
static UInt_t    l_eeeee3_ct=0;
static UInt_t    l_1_hit_and_eeeeee_ct=0;
static UInt_t    l_more_1_hit_and_eeeeee_ct=0;

static UInt_t    l_e_filt_out_of_trig_wind_ct=0;

static UInt_t    l_first=0;

//***********************************************************
TAwagsSisProc::TAwagsSisProc() : TGo4EventProcessor("Proc"),
    fInput(0),fPar(0), fNewSpill(kFALSE), fInSpill(kFALSE), fiEventInSpill(0), h_spill_scaler(0), h_signal_to_background_ave(0),fxSpillSelector(0),fxBackgroundRegion(0),fxSignalRegion(0)
{
  cout << "**** TAwagsSisProc: Create instance " << endl;
}
//***********************************************************
TAwagsSisProc::~TAwagsSisProc()
{
  cout << "**** TAwagsSisProc: Delete instance " << endl;
}
//***********************************************************
// this one is used in standard factory
TAwagsSisProc::TAwagsSisProc(const char* name) :
    TGo4EventProcessor(name), fInput(0), fOutput(0), fNewSpill(kFALSE), fInSpill(kFALSE), fiEventInSpill(0),
        fxBackgroundRegion(0), fxSignalRegion(0)
{
  cout << "**** TAwagsSisProc: Create instance " << name << endl;
  l_first = 0;
  fPar = dynamic_cast<TAwagsSisParam*>(MakeParameter("AwagsSisParam", "TAwagsSisParam", "set_AwagsSisParam.C"));
  TString obname;
  TString obtitle;
  obname.Form("Spills/EventScaler");
  obtitle.Form("Spill counter");
  h_spill_scaler = MakeTH1('I', obname.Data(), obtitle.Data(), 2, 0, 2);

  if (IsObjMade())
  {
    h_spill_scaler->GetXaxis()->SetBinLabel(1, "MBS Events");
    h_spill_scaler->GetXaxis()->SetBinLabel(2, "Spills");
  }

  obname.Form("Spills/SpillSize");
  obtitle.Form("Spill size distribution");
  TH1* testhis = GetHistogram(obname.Data());
  if (testhis && (testhis->GetNbinsX() != fPar->fNumEventsPerSpill))
    {
        printf("SSSS MBS event range for Spill changed in parameter to %d, recreate spillsize histogram.\n", fPar->fNumEventsPerSpill);
        SetMakeWithAutosave(kFALSE);
    }


  h_spill_size = MakeTH1('I', obname.Data(), obtitle.Data(), fPar->fNumEventsPerSpill, 0, fPar->fNumEventsPerSpill, "MBS Events");
  SetMakeWithAutosave(kTRUE);
  if (fPar->fSpillTriggerSFP < 0)
  {
    obname.Form("Baselines/StoB/Signal_background_ratio_average");
    obtitle.Form("Average signal to background ratio all active channels");
  }
  else
  {
    obname.Form("Baselines/StoB/Signal_background_ratio_trigger channel");
    obtitle.Form("Signal to background ratio of trigger channel (%d %d %d)", fPar->fSpillTriggerSFP,
        fPar->fSpillTriggerSlave, fPar->fSpillTriggerChan);
  }

  h_signal_to_background_ave = MakeTH1('F', obname.Data(), obtitle.Data(), 1000, 0, 5);
#ifdef  USE_SIGNALTOBACK_RATIO
  fxSpillSelector = MakeWinCond("SpillSelect", 1.01, 5000, obname.Data());
#endif
  if (fPar->fSpillTriggerSFP < 0)
  {
    obname.Form("Baselines/S-B/Signal_background_diff_average");
    obtitle.Form("Average signal minus background  all active channels");
  }
  else
  {
    obname.Form("Baselines/S-B/Signal_background_diff_trigger channel");
    obtitle.Form("Signal minus background of trigger channel (%d %d %d)", fPar->fSpillTriggerSFP,
        fPar->fSpillTriggerSlave, fPar->fSpillTriggerChan);
  }

  h_signal_minus_background_ave = MakeTH1('F', obname.Data(), obtitle.Data(), 4000, -2000, 2000);
#ifndef      USE_SIGNALTOBACK_RATIO
  fxSpillSelector=MakeWinCond("SpillSelect", 100, 20000, obname.Data());
#endif

//  TH1          *h_signal_trace_stitched;  //!< stitched traces over signal reagion of all channels, for each spill
//       TH1          *h_signal_trace_stitched_sum;
  obname.Form("Spills/Traces/AllSignalTrace");
  obtitle.Form("Stitched trace of signal region during last spill");
  h_signal_trace_stitched = MakeTH1('I', obname.Data(), obtitle.Data(), CSA_SIGNAL_SIZE * fPar->fNumEventsPerSpill, 0,
             CSA_SIGNAL_SIZE * fPar->fNumEventsPerSpill);

 obname.Form("Spills/TracesSum/AllSignalTraceSum");
 obtitle.Form("Accumulated stitched traces of signal region");
 h_signal_trace_stitched_sum = MakeTH1('I', obname.Data(), obtitle.Data(),CSA_SIGNAL_SIZE * fPar->fNumEventsPerSpill, 0,
     CSA_SIGNAL_SIZE * fPar->fNumEventsPerSpill);



  //printf ("Histograms created \n");  fflush (stdout);
}
//-----------------------------------------------------------
// event function
Bool_t TAwagsSisProc::BuildEvent(TGo4EventElement* target)
{  // called by framework. We dont fill any output event here at all


  UInt_t         l_i, l_j, l_k, l_l;
  uint32_t      *pl_se_dat;
  uint32_t      *pl_tmp;

  UInt_t         l_dat_len;  
  UInt_t         l_dat_len_byte;  

  UInt_t         l_dat;
  UInt_t         l_trig_type;
  UInt_t         l_trig_type_triva;
  UInt_t         l_sfp_id;
  UInt_t         l_feb_id;
  UInt_t         l_cha_id;
  UInt_t         l_n_hit;
 // UInt_t         l_hit_id;
  UInt_t         l_hit_cha_id;
  Long64_t       ll_time;
  Long64_t       ll_trg_time;
 // Long64_t       ll_hit_time;
  UInt_t         l_ch_hitpat   [MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_ch_hitpat_tr[MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_first_trace [MAX_SFP][MAX_SLAVE];

  UInt_t         l_cha_head;  
  UInt_t         l_cha_size;
  UInt_t         l_trace_head;
  UInt_t         l_trace_size;
  UInt_t         l_trace_trail;

  UInt_t         l_spec_head;
  UInt_t         l_spec_trail;
  UInt_t         l_n_hit_in_cha;
  UInt_t         l_only_one_hit_in_cha;
  UInt_t         l_more_than_1_hit_in_cha;  
  UInt_t         l_e_filt_out_of_trig_wind;
  UInt_t         l_hit_time_sign;
   Int_t         l_hit_time;
  UInt_t         l_hit_cha_id2;
  UInt_t         l_fpga_energy_sign;
   Int_t         l_fpga_energy;

  //UInt_t         l_trapez_e_found [MAX_SFP][MAX_SLAVE][N_CHA];
  //UInt_t         l_fpga_e_found   [MAX_SFP][MAX_SLAVE][N_CHA];
  //UInt_t         l_trapez_e       [MAX_SFP][MAX_SLAVE][N_CHA];
  //UInt_t         l_fpga_e         [MAX_SFP][MAX_SLAVE][N_CHA];

  UInt_t         l_dat_fir;
  UInt_t         l_dat_sec;

  UInt_t         l_bls_start = BASE_LINE_SUBT_START;
  UInt_t         l_bls_stop  = BASE_LINE_SUBT_START + BASE_LINE_SUBT_SIZE; // 
  Double_t       f_bls_val=0.;

  Int_t       l_fpga_filt_on_off;
  //Int_t       l_fpga_filt_mode;
  Int_t       l_dat_trace;
  Int_t       l_dat_filt;
  Int_t       l_filt_sign;

  Double_t stob=0;


  TGo4MbsSubEvent* psubevt;
  fInput = dynamic_cast<TGo4MbsEvent*> (GetInputEvent());
  if(fInput == 0)
  {
    GO4_STOP_ANALYSIS_MESSAGE(
        "NEVER COME HERE: output event is not configured, wrong class!");
  }
  fOutput= dynamic_cast<TAwagsSisBasicEvent*>  (target);
  if(fOutput==0)  GO4_STOP_ANALYSIS_MESSAGE(
      "NEVER COME HERE: output event is not configured, wrong class!");
  fOutput->SetValid(kFALSE);
  l_trig_type_triva = fInput->GetTrigger();
  if (l_trig_type_triva == 1)
  {
     l_evt_ct_phys++;
  }

  //if(fInput->GetTrigger() > 11)
  //{
  //cout << "**** TAwagsSisProc: Skip trigger event"<<endl;
  //return kFALSE;
  //}
  // first we fill the arrays fCrate1,2 with data from MBS source
  // we have up to two subevents, crate 1 and 2
  // Note that one has to loop over all subevents and select them by
  // crate number:   psubevt->GetSubcrate(),
  // procid:         psubevt->GetProcid(),
  // and/or control: psubevt->GetControl()
  // here we use only crate number

  l_evt_ct++;

  fInput->ResetIterator();
  while((psubevt = fInput->NextSubEvent()) != 0) // loop over subevents
  {

  //psubevt = fInput->NextSubEvent(); // only one subevent
  //psubevt->GetControl();
  //printf ("sub-event procid: %d\n",  psubevt->GetControl()); fflush (stdout);
  //if (psubevt->GetControl() != 69) continue;
  
  
  
  //printf ("         psubevt: 0x%x \n", (UInt_t)psubevt); fflush (stdout);
  //printf ("%d -------------next event-----------\n", l_evt_ct); fflush (stdout);
  //sleep (1);

  pl_se_dat = (uint32_t *)psubevt->GetDataField();
  l_dat_len = psubevt->GetDlen();
  l_dat_len_byte = (l_dat_len - 2) * 2; 
  //printf ("sub-event data size:         0x%x, %d \n", l_dat_len, l_dat_len);
  //printf ("sub-event data size (bytes): 0x%x, %d \n", l_dat_len_byte, l_dat_len_byte);
  //fflush (stdout);

  pl_tmp = pl_se_dat;

  if (pl_se_dat == (UInt_t*)0)
  {
    printf ("ERROR>> ");
    printf ("pl_se_dat: 0x%lx, ", (long) pl_se_dat);
    printf ("l_dat_len: 0x%x, ", (UInt_t)l_dat_len);
    printf ("l_trig_type_triva: 0x%x \n", (UInt_t)l_trig_type_triva); fflush (stdout);
    goto bad_event;  
  }

  if ( (*pl_tmp) == 0xbad00bad)
  {
    printf ("ERROR>> found bad event (0xbad00bad) \n");
    goto bad_event;
  }

  #ifdef WR_TIME_STAMP
  // 4 first 32 bits must be TITRIS time stamp
  l_dat = *pl_tmp++;
  if (l_dat != SUB_SYSTEM_ID)
  {
    printf ("ERROR>> 1. data word is not sub-system id: %d \n");
    printf ("should be: 0x%x, but is: 0x%x\n", SUB_SYSTEM_ID, l_dat);
  }

  if (l_dat != 0x100)
  {
    goto bad_event;
  }
 
  l_dat = (*pl_tmp++) >> 16;
  if (l_dat != TS__ID_L16)
  {
    printf ("ERROR>> 2. data word does not contain low 16bit identifier: %d \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_L16, l_dat);
  }
  l_dat = (*pl_tmp++) >> 16;
  if (l_dat != TS__ID_M16)
  {
    printf ("ERROR>> 3. data word does not contain low 16bit identifier: %d \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_M16, l_dat);
  }
  l_dat = (*pl_tmp++) >> 16; 
	if (l_dat != TS__ID_H16)
  {
    printf ("ERROR>> 4. data word does not contain low 16bit identifier: %d \n");
    printf ("should be: 0x%x, but is: 0x%x\n", TS__ID_H16, l_dat);
  } 
  #endif // WR_TIME_STAMP

  // extract analysis parameters from MBS data 
  // ATTENTION:  these data is only present if WRITE_ANALYSIS_PARAM 
  //             is enabled in corresponding f_user.c 
  // WRITE_ANALYSIS_PARAM (in mbs) and USE_MBS_PARAM (in go4) must be used always together 

  #ifdef USE_MBS_PARAM
  l_slaves = *pl_tmp++;
  l_trace  = *pl_tmp++;
  l_e_filt = *pl_tmp++;
  pl_tmp  += 4;
  #endif

  if (l_first == 0)
  {
    l_first = 1;
    #ifdef USE_MBS_PARAM
    printf ("debug: 0x%x, 0x%x, 0x%x \n", l_slaves, l_trace, l_e_filt);
    fflush (stdout);
    #endif
    InitDisplay (0);
  }

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          h_trace       [l_i][l_j][l_k]->Reset ("");
          h_trace_blr   [l_i][l_j][l_k]->Reset ("");
          h_trapez_fpga [l_i][l_j][l_k]->Reset ("");
          l_ch_hitpat   [l_i][l_j][l_k] = 0;  
          l_ch_hitpat_tr[l_i][l_j][l_k] = 0;
          //l_fpga_e_found[l_i][l_j][l_k] = 0;
          //l_trapez_e    [l_i][l_j][l_k] = 0;
          //l_fpga_e      [l_i][l_j][l_k] = 0;
        }
        h_hitpat     [l_i][l_j]->Fill (-2, 1);  
        h_hitpat_tr  [l_i][l_j]->Fill (-2, 1);  
        l_first_trace[l_i][l_j] = 0;
      }
    }
  }

 
  
  while ( (pl_tmp - pl_se_dat) < (l_dat_len_byte/4) )
  {
    //sleep (1);
    //printf (" begin while loop \n");  fflush (stdout); 
    l_dat = *pl_tmp++;   // must be padding word or channel header
    //printf ("l_dat 0x%x \n", l_dat);
    if ( (l_dat & 0xfff00000) == 0xadd00000 ) // begin of padding 4 byte words
    {
      //printf ("padding found \n");
      l_dat = (l_dat & 0xff00) >> 8;
      pl_tmp += l_dat - 1;  // increment by pointer by nr. of padding  4byte words 
    }
    else if ( (l_dat & 0xff) == 0x34) //channel header
    {
      l_cha_head = l_dat;
      //printf ("l_cha_head: 0x%x \n", l_cha_head);

      l_trig_type = (l_cha_head & 0xf00)      >>  8;
      l_sfp_id    = (l_cha_head & 0xf000)     >> 12;
      l_feb_id    = (l_cha_head & 0xff0000)   >> 16;
      l_cha_id    = (l_cha_head & 0xff000000) >> 24;

      if ((l_sfp_id > (MAX_SFP-1)) || (l_sfp_id < 0))
      {
        printf ("ERROR>> l_spf_id: %d \n", l_sfp_id);  fflush (stdout);
        goto bad_event; 
      }
      if ((l_feb_id > (MAX_SLAVE-1)) || (l_feb_id < 0))
      {
        printf ("ERROR>> l_feb_id: %d \n", l_feb_id); fflush (stdout);
        goto bad_event; 
      }
      if ((l_cha_id > (N_CHA-1)) || (l_cha_id < 0))
      {
        if (l_cha_id != 0xff)
        { 
          printf ("ERROR>> l_cha_id: %d \n", l_cha_id); fflush (stdout);
          goto bad_event;
        }
      }

      if ( ((l_cha_head & 0xff) >> 0) != 0x34 )
      {
        printf ("ERROR>> channel header type is not 0x34 \n");
        goto bad_event;
      }

      if ( (l_cha_head & 0xff000000) == 0xff000000) // special channel 0xff for E,t from fpga 
      {
        //printf ("                            summary channel \n");
        // special channel data size
        l_cha_size = *pl_tmp++;
        //printf ("l_cha_head: 0x%x \n", l_cha_head); sleep (1);
        //printf ("l_cha_size: 0x%x \n", l_cha_size);

        l_spec_head = *pl_tmp++;
        if ( (l_spec_head & 0xff000000) != 0xaf000000)
        {  
          printf ("ERROR>> E,t summary: wrong header is 0x%x, must be: 0x%x\n",
                                                 (l_spec_head & 0xff000000)>>24, 0xaf);              
          goto bad_event;
          //sleep (1); 
        }
        ll_trg_time  = (Long64_t)*pl_tmp++;
        ll_time      = (Long64_t)*pl_tmp++;
        ll_trg_time += ((ll_time & 0xffffff) << 32);

        l_n_hit = (l_cha_size - 16) >> 3;
        //printf ("#hits: %d \n", l_n_hit);
        if (l_trig_type_triva == 1) // physics event
        { 
          h_hitpat[l_sfp_id][l_feb_id]->Fill (-1, 1);

          for (l_i=0; l_i<l_n_hit; l_i++)
          {
            l_dat = *pl_tmp++;      // hit time from fpga (+ other info)
            l_hit_cha_id              = (l_dat & 0xf0000000) >> 28;
            l_n_hit_in_cha            = (l_dat & 0xf000000)  >> 24;

            l_more_than_1_hit_in_cha  = (l_dat & 0x400000)   >> 22;
            l_e_filt_out_of_trig_wind = (l_dat & 0x200000)   >> 21;
            l_only_one_hit_in_cha     = (l_dat & 0x100000)   >> 20;
            
            l_ch_hitpat[l_sfp_id][l_feb_id][l_hit_cha_id] = l_n_hit_in_cha;

            if (l_only_one_hit_in_cha == 1)
            {
              l_1_hit_ct++;
            }
            
            if (l_more_than_1_hit_in_cha == 1)
            {
              l_more_1_hit_ct++;
              //printf ("%d More than 1 hit found for SFP: %d FEBEX: %d CHA: %d:: %d \n",
              //l_more_1_hit_ct, l_sfp_id, l_feb_id, l_hit_cha_id, l_n_hit_in_cha);
              fflush (stdout);
            }
            if (l_e_filt_out_of_trig_wind == 1)
            {
              l_e_filt_out_of_trig_wind_ct++;
              printf ("%d energy filter out of trigger window <<<<<<<<<<<<<<<<<<<<<<\n", l_e_filt_out_of_trig_wind_ct);
            }

            if ((l_more_than_1_hit_in_cha == 1) && (l_only_one_hit_in_cha == 1))
            {
              printf ("ERROR>> haeh? \n"); fflush (stdout);
            }  

            if (l_only_one_hit_in_cha == 1)
            {
              l_hit_time_sign = (l_dat & 0x8000) >> 15;
              l_hit_time = l_dat & 0x7ff;     // positive := AFTER  trigger, relative to trigger time
              if (l_hit_time_sign == 1)       // negative sign
              {
                l_hit_time = l_hit_time * (-1); // negative := BEFORE trigger, relative to trigger time
              }
              //printf ("cha: %d, hit fpga time:  %d \n", l_hit_cha_id,  l_hit_time);
              h_trgti_hitti[l_sfp_id][l_feb_id][l_hit_cha_id]->Fill (l_hit_time);
              //h_hitpat[l_sfp_id][l_feb_id]->Fill (l_hit_cha_id, 1);
            }
            h_hitpat[l_sfp_id][l_feb_id]->Fill (l_hit_cha_id, l_n_hit_in_cha);

            l_dat = *pl_tmp++;      // energy from fpga (+ other info)
            l_hit_cha_id2  = (l_dat & 0xf0000000) >> 28;

            if ( (l_dat & 0xfffff0) != 0xeeeee0)
            {
              if (l_hit_cha_id != l_hit_cha_id2)
              {
                printf ("ERROR>> hit channel ids differ in energy and time data word\n");
                goto bad_event;
              }
              if ((l_hit_cha_id > (N_CHA-1)) || (l_hit_cha_id < 0))
              {
                printf ("ERROR>> hit channel id: %d \n", l_hit_cha_id); fflush (stdout);
                goto bad_event;
              }
            
              if (l_more_than_1_hit_in_cha == 1)
              {
                l_more_1_hit_first_energy_ok_ct++;
              }

              if ( (l_only_one_hit_in_cha == 1) || (l_more_than_1_hit_in_cha == 1) )
              {
                l_good_energy_ct++;
                l_fpga_energy_sign = (l_dat & 0x800000) >> 23;
                //l_fpga_energy      =  l_dat & 0x7ffff;      // positiv
                l_fpga_energy      =  l_dat & 0x3fffff;     // positiv
                if (l_fpga_energy_sign == 1)                // negative sign
                {
                  l_fpga_energy = l_fpga_energy * (-1);     // negative
                }
                //printf ("cha: %d, hit fpga energy: %d \n", l_hit_cha_id2,  l_fpga_energy);
                //printf ("sfp: %d, feb: %d, cha: %d \n", l_sfp_id, l_feb_id, l_hit_cha_id);
                h_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id]->Fill (l_fpga_energy);

                if (l_fpga_energy < -1000000)
                {
                  //printf ("l_dat: 0x%x, fpga energy: 0x%x \n", l_dat, l_fpga_energy);  
                }

                //l_fpga_e_found [l_sfp_id][l_feb_id][l_hit_cha_id] = 1;
                //l_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id] = l_fpga_energy;
              }
            }
            else 
            {
              l_eeeeee_ct++;
              if ( (l_dat & 0xffffff) == 0xeeeee1) {l_eeeee1_ct++;}
              if ( (l_dat & 0xffffff) == 0xeeeee2) {l_eeeee2_ct++;}
              if ( (l_dat & 0xffffff) == 0xeeeee3) {l_eeeee3_ct++;}

              if (l_only_one_hit_in_cha    == 1) {l_1_hit_and_eeeeee_ct++;}
              if (l_more_than_1_hit_in_cha == 1) {l_more_1_hit_and_eeeeee_ct++;}  
                  
              //printf ("#%d evt nr: %d channel %2d: 0x%8x found \n", l_eeeeee_ct, l_evt_ct, l_hit_cha_id, l_dat); fflush (stdout);
            }
          }
        }
        l_spec_trail = *pl_tmp++;
        if ( (l_spec_trail & 0xff000000) != 0xbf000000)
        {  
          printf ("ERROR>> E,t summary: wrong header is 0x%x, must be: 0x%x\n",
                                                 (l_spec_trail & 0xff000000)>>24, 0xbf);              
          goto bad_event;
          //sleep (1); 
        }
      }
      else // real channel 
      {
        //printf ("real channel \n");
        // channel data size
        l_cha_size = *pl_tmp++;

        // trace header
        l_trace_head = *pl_tmp++;
        //printf ("trace header \n");
        if ( ((l_trace_head & 0xff000000) >> 24) != 0xaa)
        {
          printf ("ERROR>> trace header id is not 0xaa \n");
          goto bad_event; 
        }

        l_fpga_filt_on_off = (l_trace_head & 0x80000) >> 19;
        //l_fpga_filt_mode   = (l_trace_head & 0x40000) >> 18;
        //printf ("fpga filter on bit: %d, fpga filter mode: %d \n", l_fpga_filt_on_off, l_fpga_filt_mode);
        //fflush (stdout);
        //sleep (1);

        if (l_trig_type == 1) // physics event
        {
          if (l_first_trace[l_sfp_id][l_feb_id] == 0)
          {
            l_first_trace[l_sfp_id][l_feb_id] = 1; 
            h_hitpat_tr[l_sfp_id][l_feb_id]->Fill (-1, 1);
          }
          h_hitpat_tr[l_sfp_id][l_feb_id]->Fill (l_cha_id, 1);
          l_ch_hitpat_tr[l_sfp_id][l_feb_id][l_cha_id]++;

          // now trace
          l_trace_size = (l_cha_size/4) - 2;     // in longs/32bit

          //das folgende kommentierte noch korrigieren!
          //falls trace + filter trace: cuttoff bei 2000 slices, da 4fache datenmenge!

          //if (l_trace_size != (TRACE_SIZE>>1))
          //{
          //  printf ("ERROR>> l_trace_size: %d \n", l_trace_size); fflush (stdout);
          //  goto bad_event;
          //}

          if (l_fpga_filt_on_off == 0) // only trace. no fpga filter trace data
          {
            for (l_l=0; l_l<l_trace_size; l_l++)   // loop over traces 
            {
              // disentangle data
              l_dat_fir = *pl_tmp++;
              l_dat_sec = l_dat_fir;

              #ifdef USE_MBS_PARAM
              l_adc_type = (l_trace_head & 0x800000) >> 23; 
              #else
              l_adc_type = (l_sfp_adc_type[l_sfp_id] >> l_feb_id) & 0x1;              
              #endif
            
              if (l_adc_type == 0) // 12 bit
              {
                l_dat_fir =  l_dat_fir        & 0xfff;
                l_dat_sec = (l_dat_sec >> 16) & 0xfff;
              }

              if (l_adc_type == 1)  // 14 bit
              { 
                l_dat_fir =  l_dat_fir        & 0x3fff;
                l_dat_sec = (l_dat_sec >> 16) & 0x3fff;
              } 
              h_trace[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l*2  +1, l_dat_fir);
              h_trace[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l*2+1+1, l_dat_sec);

              h_adc_spect[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_dat_fir * ADC_RES - 1000.);
              h_adc_spect[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_dat_sec * ADC_RES - 1000.);
          
              l_tr[l_l*2]   = l_dat_fir;
              l_tr[l_l*2+1] = l_dat_sec;
            }
            l_trace_size = l_trace_size * 2;
          }

          if (l_fpga_filt_on_off == 1) // trace AND fpga filter data
          {
            for (l_l=0; l_l<(l_trace_size>>1); l_l++)   // loop over traces 
            {
              // disentangle data
              l_dat_trace = *pl_tmp++;
              l_dat_filt  = *pl_tmp++;
              l_filt_sign  =  (l_dat_filt & 0x800000) >> 23; 

              #ifdef USE_MBS_PARAM
              l_adc_type = (l_trace_head & 0x800000) >> 23; 
              #else
              l_adc_type = (l_sfp_adc_type[l_sfp_id] >> l_feb_id) & 0x1;              
              #endif
            
              if (l_adc_type == 0) // 12 bit
              {
                l_dat_trace = l_dat_trace  & 0xfff;
              }

              if (l_adc_type == 1)  // 14 bit
              {
                l_dat_trace = l_dat_trace  & 0x3fff;
              } 
              
              l_dat_filt  = l_dat_filt   & 0x7fffff;
              if (l_filt_sign == 1) {l_dat_filt = l_dat_filt * -1;}

              h_trace      [l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, l_dat_trace);
              h_trapez_fpga[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, l_dat_filt);

              l_tr[l_l] = l_dat_trace;
            }
            l_trace_size = l_trace_size >> 1;
          }

/////////// JAM21: this is generic febex baseline treatment from template analysis
          // range for the trace baselin is set by compiled defines BASE_LINE_SUBT_START and BASE_LINE_SUBT_SIZE
          // find base line value of trace and correct it to baseline 0
          f_bls_val = 0.;
          for (l_l=l_bls_start; l_l<l_bls_stop; l_l++) 
          {
            f_bls_val += (Double_t)l_tr[l_l];
          }
          f_bls_val = f_bls_val / (Double_t)(l_bls_stop - l_bls_start); 
          for (l_l=0; l_l<l_trace_size; l_l++)   // create baseline restored trace 
          {
            f_tr_blr[l_l] =  (Double_t)l_tr[l_l] - f_bls_val;
            h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_tr_blr[l_l]);
            //h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, f_tr_blr[l_l]);
          }

          // find peak and fill histogram
          h_peak  [l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
          h_valley[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());

     
}
        // jump over trace
        //pl_tmp += (l_cha_size >> 2) - 2;          
            
        // trace trailer
        //printf ("trace trailer \n");
        l_trace_trail = *pl_tmp++;
        if ( ((l_trace_trail & 0xff000000) >> 24) != 0xbb)
        {
          printf ("ERROR>> trace trailer id is not 0xbb, ");
          printf ("SFP: %d, FEB: %d, CHA: %d \n", l_sfp_id, l_feb_id, l_cha_id); 
          goto bad_event; 
        }
      }
    }
    else
    {
      printf ("ERROR>> data word neither channel header nor padding word \n");
    }       
  }

  } // while subevents
  /****************End of acual unpacker. below further treatments on histogram data********************************/

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          h_ch_hitpat   [l_i][l_j][l_k]->Fill (l_ch_hitpat   [l_i][l_j][l_k]);  
          h_ch_hitpat_tr[l_i][l_j][l_k]->Fill (l_ch_hitpat_tr[l_i][l_j][l_k]);  
        } // l_k
      } // l_j
    } // if slaves
  } // l_i

/**** END of original Nik code for generic Febex *************************/



 stob= HandleSignalToBackground();
 EvaluateSpills(stob);






//  if ( (l_evt_ct % 1000) == 0)
//  {
//    printf ("------------------------------------------------------\n");
//    printf ("nr of events processed:                  %10u \n", l_evt_ct);
//    printf ("nr of good_energies found:               %10u \n", l_good_energy_ct);
//    printf ("single hits found in trigger window:     %10u \n", l_1_hit_ct);
//    printf ("multi hits found in trigger window:      %10u \n", l_more_1_hit_ct);
//    printf ("multi hits found, first hit energy ok:   %10u \n", l_more_1_hit_first_energy_ok_ct);
//    printf ("nr of events with failed fpga energy:    %10u \n", l_eeeeee_ct);
//    printf ("nr of events with eeeee1:                %10u \n", l_eeeee1_ct);
//    printf ("nr of events with eeeee2:                %10u \n", l_eeeee2_ct);
//    printf ("nr of events with eeeee3:                %10u \n", l_eeeee3_ct);
//    printf ("nr of events with failed fpga energy - \n");
//    printf ("and a single hit found in trigger window %10u \n", l_1_hit_and_eeeeee_ct);
//    printf ("nr of events with failed fpga energy - \n");
//    printf ("and multi hits found in trigger window   %10u \n", l_more_1_hit_and_eeeeee_ct);
//    printf ("------------------------------------------------------\n");
//    fflush (stdout);
//  }

  return kTRUE;

  bad_event:
    GO4_SKIP_EVENT_MESSAGE("Skipped bad event %d", fInput->GetCount());

}


Double_t TAwagsSisProc::HandleSignalToBackground()
{
  // JAM 27-jul-2022: this code is reused from gem csa analysis of 2021. Apply it here to find begin of spill
  // and also for calculating accumulated charge differences for each event
  Double_t sigtoback_average = 0.0;    // use average value of all channels to detect spill start
  Double_t sigminusback_average = 0.0;    //
  Int_t numvals = 0, numvalsdiff = 0;
  for (UInt_t l_i = 0; l_i < MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (UInt_t l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++)
      {
        for (UInt_t l_k = 0; l_k < N_CHA; l_k++)
        {
          Double_t range_back = fxBackgroundRegion->GetXUp() - fxBackgroundRegion->GetXLow();
          Double_t ave_back = 0;
          if (range_back)
            ave_back = fxBackgroundRegion->GetIntegral(h_trace[l_i][l_j][l_k]) / range_back;
          if (ave_back)
            h_background_height[l_i][l_j][l_k]->Fill(ave_back);

          Double_t range_sig = fxSignalRegion->GetXUp() - fxSignalRegion->GetXLow();
          Double_t ave_sig = 0;
          if (range_sig)
            ave_sig = fxSignalRegion->GetIntegral(h_trace[l_i][l_j][l_k]) / range_sig;
          if (ave_sig)
            h_signal_height[l_i][l_j][l_k]->Fill(ave_sig);

          Double_t sigtoback = 0;
          if (ave_back)
            sigtoback = ave_sig / ave_back;
          h_signal_to_background[l_i][l_j][l_k]->Fill(sigtoback);
          if (fPar->fSpillTriggerSFP < 0)
          {
            // no spill trigger set: try with average of all
            sigtoback_average += sigtoback;
            ++numvals;
          }
          else
          {
            if (((UInt_t) fPar->fSpillTriggerSFP == l_i) && (fPar->fSpillTriggerSlave == l_j)
                && (fPar->fSpillTriggerChan == l_k))
            {
              sigtoback_average = sigtoback;    // select spill trigger channel value only
            }

          }

          Double_t sigminusback = ave_sig - ave_back;

          if (fPar->fSpillTriggerSFP < 0)
          {
            // no spill trigger set: try with average of all

            sigminusback_average += sigminusback;
            ++numvalsdiff;
          }
          else
          {
            if (((UInt_t)fPar->fSpillTriggerSFP == l_i) && (fPar->fSpillTriggerSlave == l_j)
                && (fPar->fSpillTriggerChan == l_k))
            {
              sigminusback_average = sigminusback;    // select spill trigger channel value only
            }

          }
          h_signal_minus_background[l_i][l_j][l_k]->Fill(sigminusback);
          fDeltaQ[l_i][l_j][l_k] = sigminusback;    // evaluation of delta Q between subsequent event samples in spill

        }    // l_k
      }    // l_j
    }    // if slaves
  }    // l_i

  if (fPar->fSpillTriggerSFP < 0)
  {
    if (numvals)
      sigtoback_average /= numvals;
    if (numvalsdiff)
      sigminusback_average /= numvalsdiff;
  }
  h_signal_to_background_ave->Fill(sigtoback_average);    // use overview histogram for setting condition
  h_signal_minus_background_ave->Fill(sigminusback_average);
#ifdef  USE_SIGNALTOBACK_RATIO
  return sigtoback_average;
#else
  return sigminusback_average;
#endif
}



void TAwagsSisProc::EvaluateSpills(Double_t sigtoback)
{
  // first find out the state we are in: before, in, after spill:
  if(!fNewSpill && sigtoback>0 && !fxSpillSelector->Test(sigtoback))
   {
     printf("LLLL - Looking for new spill because sigtoback_average=%e is below threshold %e\n", sigtoback, fxSpillSelector->GetXLow());
     std::cout << std::endl;
     fNewSpill=kTRUE;
     fInSpill=kFALSE;
   }
   if(fNewSpill && fxSpillSelector->Test(sigtoback))
 {
   printf("SSSS -Found begin of new spill because sigtoback_average=%e rises above threshold %e\n", sigtoback, fxSpillSelector->GetXLow());
   std::cout << std::endl;
   fInSpill=kTRUE;
 }

   // TODO: breakout of in spill if we exceed the user limit for mbs events numbers at each spill
  if(!fNewSpill && fInSpill &&  fiEventInSpill> fPar->fMaxSpillEvent)
  {
    printf("EEEE - Set end of spill  as number of MBS samples exceed expected maximum %d\n", fiEventInSpill);
    std::cout << std::endl;
    fInSpill=kFALSE;
    // new spill will be searched  only when we are once below threshold
  }


  if(!fInSpill) return;

  // if we are in the spill, put data to combined spill display and output event for mapping:
  h_spill_scaler->AddBinContent(1);

  for (UInt_t l_i=0; l_i<MAX_SFP; l_i++)
   {
     if (l_sfp_slaves[l_i] != 0)
     {
       for (UInt_t l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
       {
         for (UInt_t l_k=0; l_k<N_CHA; l_k++)
         {

           if(fNewSpill)
           {
             // when new spill is detected, copy previous q vs evt and traces to output event and set this valid.
             if (fPar->fMapSpills)
            {
              Double_t value = 0;
#ifdef AWAGS_STORE_TRACES
              for (Int_t bin = 1; bin < h_trace_stitched[l_i][l_j][l_k]->GetNbinsX(); ++bin)
              {
                value = h_trace_stitched[l_i][l_j][l_k]->GetBinContent(bin);
                fOutput->fSpillTrace[l_i][l_j][l_k].push_back(value);
              }
#endif
              for (Int_t bin = 1; bin < h_q_spill[l_i][l_j][l_k]->GetNbinsX(); ++bin)
              {
                value = h_q_spill[l_i][l_j][l_k]->GetBinContent(bin);
                fOutput->fChargeTrend[l_i][l_j][l_k].push_back(value);
              }
               fOutput->SetValid(kTRUE);
            } // if (fPar->fMapSpills)

             // then clear the previous spill histograms:
             h_q_spill [l_i][l_j][l_k]->Reset("");
             h_trace_stitched[l_i][l_j][l_k]->Reset("");
           }
           else
           {
             // only fill histograms when we are still in spill:
           Double_t value=0;
           Int_t stitchbin=0;
           Int_t tracebinmax=h_trace[l_i][l_j][l_k]->GetNbinsX();
           for(Int_t bin=1; bin<tracebinmax; ++bin)
             {
              value=h_trace[l_i][l_j][l_k]->GetBinContent(bin);
              stitchbin=bin+(fiEventInSpill-1)*(tracebinmax -1);
              if((stitchbin > 1) && stitchbin < h_trace_stitched[l_i][l_j][l_k]->GetNbinsX())
              {
                h_trace_stitched[l_i][l_j][l_k]->SetBinContent(stitchbin, value);
                h_trace_stitched_sum[l_i][l_j][l_k]->AddBinContent(stitchbin, value);
              }

              // here stitch within signal region only:
              if(fxSignalRegion->Test(bin-1))
              {
                Double_t winsize=fxSignalRegion->GetXUp() - fxSignalRegion->GetXLow() -1;

                Int_t signalbin= bin+ (fiEventInSpill-1)*winsize;
              if ((signalbin > 1) && signalbin < h_signal_trace_stitched->GetNbinsX())
              {
                h_signal_trace_stitched->AddBinContent(signalbin, value);
                h_signal_trace_stitched_sum->AddBinContent(signalbin, value);
              }
              }
             }

           h_q_spill [l_i][l_j][l_k]->Fill(fiEventInSpill, fDeltaQ[l_i][l_j][l_k]);
           h_q_spill_sum [l_i][l_j][l_k]->Fill(fiEventInSpill, fDeltaQ[l_i][l_j][l_k]);

           } // no new spill
         }
       }
     }
   } // for (UInt_t l_i=0

  if(fNewSpill)
  {
      fNewSpill=kFALSE;
      h_spill_scaler->AddBinContent(2);
      fOutput->fuSpillCount++;
      h_spill_size->Fill(fiEventInSpill);
      // TODO here: copy stitched signal trace to output event
      h_signal_trace_stitched->Reset("");
      fiEventInSpill=0;
  }
  fiEventInSpill++;
}





//--------------------------------------------------------------------------------------------------------

void TAwagsSisProc:: InitDisplay (Int_t l_mode)
{
  Text_t chis[256];
  Text_t chead[256];
  Text_t cfolder[256];
  UInt_t l_i, l_j, l_k;
  UInt_t l_tra_size;
  UInt_t l_trap_n_avg;
//  UInt_t l_left;
//  UInt_t l_right;

  Bool_t firstcheck=kTRUE;

  #ifdef USE_MBS_PARAM
  l_tra_size   = l_trace & 0xffff;
  l_trap_n_avg = l_e_filt >> 21;
  printf ("InitDisplay: trace size: %d, avg size %d \n", l_tra_size, l_trap_n_avg);
  fflush (stdout);
  l_sfp_slaves[0] =  l_slaves & 0xff;   
  l_sfp_slaves[1] = (l_slaves & 0xff00)     >>  8;
  l_sfp_slaves[2] = (l_slaves & 0xff0000)   >> 16;
  l_sfp_slaves[3] = (l_slaves & 0xff000000) >> 24;
  printf ("InitDisplay: # of sfp slaves: 3:%d, 2:%d, 1: %d, 0: %d \n",
          l_sfp_slaves[3], l_sfp_slaves[2], l_sfp_slaves[1], l_sfp_slaves[0]);
  fflush (stdout);
  #else
  l_tra_size   = TRACE_SIZE;
  l_trap_n_avg = TRAPEZ_N_AVG;
  #endif // USE_MBS_PARAM      
  SetMakeWithAutosave(kTRUE);
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Traces/TRACE  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Trace");
          h_trace[l_i][l_j][l_k] = MakeTH1('I', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Traces BLR/TRACE, base line restored SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Trace, base line restored");
          h_trace_blr[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FPGA/FPGA Trapez SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FPGA Trapez");
          h_trapez_fpga[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"FPGA/FPGA Energy(hitlist) SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"FPGA Energy");
          //l_right = 0x1000 * l_trap_n_avg;
          //l_left = -1 * l_right;
          //printf ("depp: %d %d\n", l_left, l_right); fflush (stdout); 
          //h_fpga_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x1000,l_left,l_right);
          h_fpga_e[l_i][l_j][l_k] = MakeTH1('F', chis,chead,100000,-1000000,1000000);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Peaks/PEAK   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Peak");
          h_peak[l_i][l_j][l_k] = MakeTH1('I', chis,chead,0x1000,0,0x4000);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Valleys/VALLEY   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Valley");
          h_valley[l_i][l_j][l_k] = MakeTH1('I', chis,chead,0x1000,0,0x4000);
        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Timediff/Trigger time - Hit time   SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"TRIG-HIT");
          h_trgti_hitti[l_i][l_j][l_k] = MakeTH1('I', chis,chead,2000,-1000,1000);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Hitpat_Cha_List/Channel hit pattern per event (list)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"HITPAT_Cha_List");
          h_ch_hitpat[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
        }

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"Hitpat_Cha_Trace/Channel hit pattern per event (trace)  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"HITPAT_Cha_Trace");
          h_ch_hitpat_tr[l_i][l_j][l_k] = MakeTH1('I', chis,chead,11,-1,10);
        }

        sprintf(chis,"Hitpat_Feb_List/Hit Pattern (list)  SFP: %2d FEBEX: %2d", l_i, l_j);
        sprintf(chead,"Hitpat_List");
        h_hitpat[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

        sprintf(chis,"Hitpat_Feb_Trace/Hit Pattern (trace)  SFP: %2d FEBEX: %2d", l_i, l_j);
        sprintf(chead,"Hitpat_Trace");
        h_hitpat_tr[l_i][l_j] = MakeTH1('I', chis,chead,20,-2,18);

        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"ADC_Spectra/ADC Spectrum [mV]  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"ADC Spectrum");
          h_adc_spect[l_i][l_j][l_k] = MakeTH1('F', chis,chead,16384,-1000,1000);
        }

 
 
 
 

        // JAM2021: define regions for evaluation of average background and signal height:
        fxBackgroundRegion=MakeWinCond("BackgroundRegion",CSA_BASE_START, CSA_BASE_START + CSA_BASE_SIZE);
        fxSignalRegion=MakeWinCond("SignalRegion", CSA_SIGNAL_START, CSA_SIGNAL_START+ CSA_SIGNAL_SIZE);

          // JAM 2021: without the fitter, we define result histograms for background and signal average:
        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          sprintf(chis, "Baselines/Background/Background SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Average background region height SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_background_height[l_i][l_j][l_k] = MakeTH1('F', chis, chead, 0x8000, 0, 0x8000);

          sprintf(chis, "Baselines/Signals/Signals SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Average signal region height SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_signal_height[l_i][l_j][l_k] = MakeTH1('F', chis, chead, 0x8000, 0, 0x8000);

          sprintf(chis, "Baselines/StoB/Signal_background_ratio SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Average signal to background ratio SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_signal_to_background[l_i][l_j][l_k] = MakeTH1('F', chis, chead, 1000, 0, 5);

          sprintf(chis, "Baselines/S-B/Signal_minus_background_SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Average signal minus background SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_signal_minus_background[l_i][l_j][l_k] = MakeTH1('F', chis, chead, 4000, -2000, 2000);

          // JAM2022: here special histograms for awags spill evaluation:





          sprintf(chis, "Spills/Charge/Spillcharge SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Charge trend during last spill SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);

            // TODO first find out if we have changed preset range of spills:
          if(firstcheck)
          {
            TH1* testhis = GetHistogram(chis);
            if (testhis && (testhis->GetNbinsX() != fPar->fNumEventsPerSpill))
            {
              printf("SSSS MBS event range for Spill changed in parameter to %d, recreate spill histograms...\n", fPar->fNumEventsPerSpill);
              SetMakeWithAutosave(kFALSE);
            }
            firstcheck = kFALSE;
          }
          h_q_spill[l_i][l_j][l_k] = MakeTH1('F', chis, chead, fPar->fNumEventsPerSpill, 0, fPar->fNumEventsPerSpill,
              "MBS Event index", "#Delta Q");

          sprintf(chis, "Spills/ChargeSum/SpillchargeSum SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Accumulated charge trend during spill SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_q_spill_sum[l_i][l_j][l_k] = MakeTH1('F', chis, chead, fPar->fNumEventsPerSpill, 0,
              fPar->fNumEventsPerSpill, "MBS Event index", "#Delta Q");

          sprintf(chis, "Spills/Traces/Spilltrace  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Stitched traces during last spill SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_trace_stitched[l_i][l_j][l_k] = MakeTH1('I', chis, chead, l_tra_size * fPar->fNumEventsPerSpill, 0,
              l_tra_size * fPar->fNumEventsPerSpill);

          sprintf(chis, "Spills/TracesSum/SpilltraceSum  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Accumulated stitched traces during last spill SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          h_trace_stitched_sum[l_i][l_j][l_k] = MakeTH1('I', chis, chead, l_tra_size * fPar->fNumEventsPerSpill, 0,
              l_tra_size * fPar->fNumEventsPerSpill);

        }    // l_k

      } // l_j
    } // l_sfp_slaves
  } // l_i
  ///////////////////////////// Pictures below
  
  // JAM2021 - new picture for baseline without fit:
  for (l_i = 0; l_i < MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++)
      {
        sprintf(cfolder, "Baselines/SFP0%2d_Slave%2d", l_i, l_j);
        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          sprintf(chis, "Picture_baselines_%2d_%2d_%2d", l_i, l_j, l_k);
          TGo4Picture* pic = GetPicture(chis);
          if (pic == 0)
          {
            sprintf(chead, "Trace Baselines SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
            pic = new TGo4Picture(chis, chead);
            pic->SetDivision(2, 2);
            pic->Pic(0, 0)->AddObject(h_trace[l_i][l_j][l_k]);
            //pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
            pic->Pic(0, 0)->AddObject(fxBackgroundRegion);    // one region for all
            pic->Pic(0, 0)->AddObject(fxSignalRegion);    // one region for all
            pic->Pic(0, 1)->AddObject(h_background_height[l_i][l_j][l_k]);
            //pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
            pic->Pic(0, 1)->AddObject(h_signal_height[l_i][l_j][l_k]);
            pic->Pic(1, 0)->AddObject(h_signal_minus_background[l_i][l_j][l_k]);
            pic->Pic(1, 1)->AddObject(h_signal_to_background[l_i][l_j][l_k]);
            AddPicture(pic, cfolder);
          }
        }    // l_k


         /////////////// spill overviews:

          sprintf(cfolder, "Spills/SFP0%2d_Slave%2d", l_i, l_j);
        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          sprintf(chis, "Picture_Spills_%2d_%2d_%2d", l_i, l_j, l_k);
          TGo4Picture* pic = GetPicture(chis);
          if (pic == 0)
          {
            sprintf(chead, "Spill traces SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
            pic = new TGo4Picture(chis, chead);
            pic->SetDivision(2, 2);
            pic->Pic(0, 0)->AddObject(h_trace_stitched[l_i][l_j][l_k]);
            //pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
            pic->Pic(0, 1)->AddObject(h_q_spill[l_i][l_j][l_k]);
            pic->Pic(1, 0)->AddObject(h_trace_stitched_sum[l_i][l_j][l_k]);
            pic->Pic(1, 1)->AddObject(h_q_spill_sum[l_i][l_j][l_k]);
            AddPicture(pic, cfolder);
          }

        } // l_k

      } // l_j
    } //l_sfp_slaves
  } // l_i

}





//----------------------------END OF GO4 SOURCE FILE ---------------------
