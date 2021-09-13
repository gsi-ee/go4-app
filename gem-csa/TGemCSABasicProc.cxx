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
 * Analysis for the GEM CSA tests in December 2019
 * Added mapping to original unpacker step based on TFeb3BasicProc
 * v.01 on 10-Dec-2019 by JAM (j.adamczewski@gsi.de)
 * v.02 on 24-Jan-2020 by JAM - merge with additional code from Nik
 *
 *
 * */




#include "TGemCSABasicProc.h"
#include "TGemCSABasicEvent.h"
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
#include "TGemCSABasicParam.h"
#include "TGo4Fitter.h"

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
TGemCSABasicProc::TGemCSABasicProc() : TGo4EventProcessor("Proc")
{
  cout << "**** TGemCSABasicProc: Create instance " << endl;
}
//***********************************************************
TGemCSABasicProc::~TGemCSABasicProc()
{
  cout << "**** TGemCSABasicProc: Delete instance " << endl;
  l_first=0; // for resubmit mode: init histogram pointers correctly
}
//***********************************************************
// this one is used in standard factory
TGemCSABasicProc::TGemCSABasicProc(const char* name) : TGo4EventProcessor(name)
{
  cout << "**** TGemCSABasicProc: Create instance " << name << endl;
  l_first = 0;  

  fParam = (TGemCSABasicParam*) MakeParameter("GemCSABasicParam", "TGemCSABasicParam", "set_GemCSABasicParam.C");



  //printf ("Histograms created \n");  fflush (stdout);
}
//-----------------------------------------------------------
// event function
Bool_t TGemCSABasicProc::BuildEvent(TGo4EventElement* target)
{  // called by framework. We dont fill any output event here at all

TGemCSABasicEvent* outevent= dynamic_cast<TGemCSABasicEvent*>  (target);
if(outevent==0)  GO4_STOP_ANALYSIS_MESSAGE(
    "NEVER COME HERE: output event is not configured, wrong class!")

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



#ifdef      USE_OLD_CSA_BASELINES
  Double_t       f_csa_base_val=0.;
  Double_t       f_csa_signal_val=0.;
  Double_t       f_csa_sum_sig=0.;
  Double_t       f_ref_peak=0;
  Double_t       f_ref_cha_base_val=0;
  Double_t       f_csa_max_sig=0; 
  Int_t          l_csa_max_cha=0xff;
  Double_t       f_csa_pad_e_a[256];
  Double_t       f_csa_pad_e_b[MAX_SFP][MAX_SLAVE][N_CHA];   
#endif

  TGo4MbsSubEvent* psubevt;

  fInput = (TGo4MbsEvent* ) GetInputEvent();
  if(fInput == 0)
  {
    cout << "AnlProc: no input event !"<< endl;
    return kFALSE;
  }


  l_trig_type_triva = fInput->GetTrigger();
  if (l_trig_type_triva == 1)
  {
     l_evt_ct_phys++;
  }

  //if(fInput->GetTrigger() > 11)
  //{
  //cout << "**** TGemCSABasicProc: Skip trigger event"<<endl;
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
  psubevt->GetControl();
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
    f_make_histo (0);
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
#ifdef      USE_OLD_CSA_BASELINES
          f_csa_pad_e_b [l_i][l_j][l_k] = 0;
#endif
		}
        h_hitpat     [l_i][l_j]->Fill (-2, 1);  
        h_hitpat_tr  [l_i][l_j]->Fill (-2, 1);  
        l_first_trace[l_i][l_j] = 0;
      }
    }
  }
#ifdef      USE_OLD_CSA_BASELINES
	h_csa_pad_e->Reset ("");
	
  for (l_i=0; l_i<256; l_i++)
	{
		f_csa_pad_e_a[l_i] = 0.;
	}
	
  if ((l_sfp_id == 0) && (l_feb_id == 0))
  {
    //f_csa_sum_sig = 0.;
    l_csa_max_cha = 0xff;
    f_csa_max_sig = 0.;
  }

	f_csa_sum_sig = 0.;
#endif
	
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


#ifdef      USE_OLD_CSA_BASELINES
          if ((l_sfp_id == 0) && (l_feb_id == 0) && (l_cha_id == 14))
          {
            f_ref_peak = (Double_t)h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum (); 
          }

          // this is additional baseline evaluation for 2020 beamtime. provided for comparison.
          // find average (febex samples) baseline value
          f_csa_base_val = 0.;
          for (l_l=CSA_BASE_START; l_l<(CSA_BASE_START + CSA_BASE_SIZE); l_l++) 
          {
            f_csa_base_val += (Double_t)l_tr[l_l];
          }
          f_csa_base_val = f_csa_base_val / (Double_t)CSA_BASE_SIZE;
          //printf ("f_csa_base_val %f \n", f_csa_base_val);          
          h_csa_base [l_sfp_id][l_feb_id][l_cha_id]->Fill (f_csa_base_val);

          // find average (febex samples) signal value
          f_csa_signal_val = 0.;
          for (l_l=CSA_SIGNAL_START; l_l<(CSA_SIGNAL_START + CSA_SIGNAL_SIZE); l_l++) 
          {
            f_csa_signal_val += (Double_t)l_tr[l_l];
          }
          f_csa_signal_val = f_csa_signal_val / (Double_t)CSA_SIGNAL_SIZE;
          h_csa_signal [l_sfp_id][l_feb_id][l_cha_id]->Fill (f_csa_signal_val - f_csa_base_val);

          f_csa_pad_e_b[l_sfp_id][l_feb_id][l_cha_id] = f_csa_signal_val - f_csa_base_val; 
					
          if ( ((l_sfp_id == 0) && (l_feb_id > 0)) || (l_sfp_id == 1) )
					{

						if ((f_csa_signal_val - f_csa_base_val) < 0.)
						{	
              f_csa_sum_sig += f_csa_signal_val - f_csa_base_val;
						}	
					}		
#endif

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
        }
      }
    }
  }
#ifdef      USE_OLD_CSA_BASELINES
  //printf ("f_csa_sum_sig %f \n", f_csa_sum_sig); 
  h_csa_sum_sig->Fill (f_csa_sum_sig * -1.);
	/*
  h_peak_ref_sig->Fill (f_ref_peak - f_ref_cha_base_val);    
  h_peak_ref__sum_csa->Fill (f_csa_sum_sig, f_ref_peak - f_ref_cha_base_val);

  if ((l_csa_max_cha == 4) || (l_csa_max_cha == 5))
  {
    h_peak_ref__sum_csa2->Fill (f_csa_sum_sig, f_ref_peak - f_ref_cha_base_val);
  }
    */
#endif

  // JAM 10-DEC-2019: for mapping, we first copy the raw traces into our output event:
  {
  Double_t value=0;
  for (l_i=0; l_i<MAX_SFP; l_i++)
   {
     if (l_sfp_slaves[l_i] != 0)
     {
       for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
       {
         for (l_k=0; l_k<N_CHA; l_k++)
         {

           if(fParam->fMapTraces)
           {
             // may disable filling output event to speed up things in first step JAM
             for(int bin=1; bin<h_trace[l_i][l_j][l_k]->GetNbinsX(); ++bin)
             {
                 value=h_trace[l_i][l_j][l_k]->GetBinContent(bin);
                 outevent->fTrace[l_i][l_j][l_k].push_back(value);
             }

             for(int bin=1; bin<h_trace_blr[l_i][l_j][l_k]->GetNbinsX(); ++bin)
             {
               value=h_trace_blr[l_i][l_j][l_k]->GetBinContent(bin);
               outevent->fTraceBLR[l_i][l_j][l_k].push_back(value);
             }

             for(int bin=1; bin<h_trapez_fpga[l_i][l_j][l_k]->GetNbinsX(); ++bin)
             {
               value=h_trapez_fpga[l_i][l_j][l_k]->GetBinContent(bin);
               outevent->fTraceFPGA[l_i][l_j][l_k].push_back(value);
             }
           } // map traces end

           /** JAM 28-Jan-2020: also do the optional fit within this loop:*/
           DoTraceFit(l_i,l_j,l_k);

           /** JAM 10-Sep-2021: do more fast and simple evaluation of average heights here:*/
           Double_t range_back=fxBackgroundRegion->GetXUp()-fxBackgroundRegion->GetXLow();
           Double_t ave_back= 0;
           if(range_back) ave_back=fxBackgroundRegion->GetIntegral(h_trace[l_i][l_j][l_k]) / range_back;
           if(ave_back) h_background_height [l_i][l_j][l_k]->Fill(ave_back);

           Double_t range_sig=fxSignalRegion->GetXUp()-fxSignalRegion->GetXLow();
           Double_t ave_sig= 0;
           if(range_sig) ave_sig=fxSignalRegion->GetIntegral(h_trace[l_i][l_j][l_k]) / range_sig;
           if(ave_sig) h_signal_height [l_i][l_j][l_k]->Fill(ave_sig);

           Double_t sigtoback=0;
           sigtoback=ave_sig/ave_back;
           if(sigtoback) h_signal_to_background [l_i][l_j][l_k]->Fill(sigtoback);

           Double_t sigminusback=ave_sig -ave_back;
           h_signal_minus_background [l_i][l_j][l_k]->Fill(sigminusback);



           }
       }
     }
   }
  }

#ifdef      USE_OLD_CSA_BASELINES
  // also historic mapping from 2020 test beam.
f_csa_pad_e_a[0] = f_csa_pad_e_b[1][1][0];
  f_csa_pad_e_a[1] = f_csa_pad_e_b[0][7][8];   // nc
  f_csa_pad_e_a[2] = f_csa_pad_e_b[1][0][0];
  f_csa_pad_e_a[3] = f_csa_pad_e_b[0][0][8];   // nc
  f_csa_pad_e_a[4] = f_csa_pad_e_b[1][1][1];
  f_csa_pad_e_a[5] = f_csa_pad_e_b[0][7][9];   // nc
  f_csa_pad_e_a[6] = f_csa_pad_e_b[1][0][1];
  f_csa_pad_e_a[7] = f_csa_pad_e_b[0][0][9];   // nc
  f_csa_pad_e_a[8] = f_csa_pad_e_b[1][1][2];
  f_csa_pad_e_a[9] = f_csa_pad_e_b[0][7][10];   // nc
  f_csa_pad_e_a[10] = f_csa_pad_e_b[1][0][2];
  f_csa_pad_e_a[11] = f_csa_pad_e_b[0][0][10];   // nc
  f_csa_pad_e_a[12] = f_csa_pad_e_b[1][1][3];
  f_csa_pad_e_a[13] = f_csa_pad_e_b[0][7][11];   // nc
  f_csa_pad_e_a[14] = f_csa_pad_e_b[1][0][3];
  f_csa_pad_e_a[15] = f_csa_pad_e_b[0][0][11];   // nc
  f_csa_pad_e_a[16] = f_csa_pad_e_b[1][1][4];
  f_csa_pad_e_a[17] = f_csa_pad_e_b[0][7][12];   // nc
  f_csa_pad_e_a[18] = f_csa_pad_e_b[1][0][4];
  f_csa_pad_e_a[19] = f_csa_pad_e_b[0][0][12];   // nc
  f_csa_pad_e_a[20] = f_csa_pad_e_b[1][1][5];
  f_csa_pad_e_a[21] = f_csa_pad_e_b[0][7][13];   // nc
  f_csa_pad_e_a[22] = f_csa_pad_e_b[1][0][5];
  f_csa_pad_e_a[23] = f_csa_pad_e_b[0][0][13];   // nc
  f_csa_pad_e_a[24] = f_csa_pad_e_b[1][1][6];
  f_csa_pad_e_a[25] = f_csa_pad_e_b[0][7][14];   // nc
  f_csa_pad_e_a[26] = f_csa_pad_e_b[1][0][6];
  f_csa_pad_e_a[27] = f_csa_pad_e_b[0][0][14];   // nc
  f_csa_pad_e_a[28] = f_csa_pad_e_b[1][1][7];
  f_csa_pad_e_a[29] = f_csa_pad_e_b[0][7][15];   // nc
  f_csa_pad_e_a[30] = f_csa_pad_e_b[1][0][7];
  f_csa_pad_e_a[31] = f_csa_pad_e_b[0][0][15];   // nc
  f_csa_pad_e_a[32] = f_csa_pad_e_b[1][1][8];
  f_csa_pad_e_a[33] = f_csa_pad_e_b[0][7][0];   // nc
  f_csa_pad_e_a[34] = f_csa_pad_e_b[1][0][8];
  f_csa_pad_e_a[35] = f_csa_pad_e_b[0][0][0];   // nc
  f_csa_pad_e_a[36] = f_csa_pad_e_b[1][1][9];
  f_csa_pad_e_a[37] = f_csa_pad_e_b[0][7][1];   // nc
  f_csa_pad_e_a[38] = f_csa_pad_e_b[1][0][9];
  f_csa_pad_e_a[39] = f_csa_pad_e_b[0][0][1];   // nc
  f_csa_pad_e_a[40] = f_csa_pad_e_b[1][1][10];
  f_csa_pad_e_a[41] = f_csa_pad_e_b[0][7][2];   // nc
  f_csa_pad_e_a[42] = f_csa_pad_e_b[1][0][10];
  f_csa_pad_e_a[43] = f_csa_pad_e_b[0][0][2];   // nc
  f_csa_pad_e_a[44] = f_csa_pad_e_b[1][1][11];
  f_csa_pad_e_a[45] = f_csa_pad_e_b[0][7][3];   // nc
  f_csa_pad_e_a[46] = f_csa_pad_e_b[1][0][11];
  f_csa_pad_e_a[47] = f_csa_pad_e_b[0][0][3];   // nc
  f_csa_pad_e_a[48] = f_csa_pad_e_b[1][1][12];
  f_csa_pad_e_a[49] = f_csa_pad_e_b[0][7][4];   // nc
  f_csa_pad_e_a[50] = f_csa_pad_e_b[1][0][12];
  f_csa_pad_e_a[51] = f_csa_pad_e_b[0][0][4];   // nc
  f_csa_pad_e_a[52] = f_csa_pad_e_b[1][1][13];
  f_csa_pad_e_a[53] = f_csa_pad_e_b[0][7][5];   // nc
  f_csa_pad_e_a[54] = f_csa_pad_e_b[1][0][13];
  f_csa_pad_e_a[55] = f_csa_pad_e_b[0][0][5];   // nc
  f_csa_pad_e_a[56] = f_csa_pad_e_b[1][1][14];
  f_csa_pad_e_a[57] = f_csa_pad_e_b[0][7][6];   // nc
  f_csa_pad_e_a[58] = f_csa_pad_e_b[1][0][14];
  f_csa_pad_e_a[59] = f_csa_pad_e_b[0][0][6];   // nc
  f_csa_pad_e_a[60] = f_csa_pad_e_b[1][1][15];
  f_csa_pad_e_a[61] = f_csa_pad_e_b[0][7][7];   // nc
  f_csa_pad_e_a[62] = f_csa_pad_e_b[1][0][15];
  f_csa_pad_e_a[63] = f_csa_pad_e_b[0][0][7];   // nc
  f_csa_pad_e_a[64] = f_csa_pad_e_b[1][3][0];
  f_csa_pad_e_a[65] = f_csa_pad_e_b[0][2][8];
  f_csa_pad_e_a[66] = f_csa_pad_e_b[1][2][0];
  f_csa_pad_e_a[67] = f_csa_pad_e_b[0][1][8];
  f_csa_pad_e_a[68] = f_csa_pad_e_b[1][3][1];
  f_csa_pad_e_a[69] = f_csa_pad_e_b[0][2][9];
  f_csa_pad_e_a[70] = f_csa_pad_e_b[1][2][1];
  f_csa_pad_e_a[71] = f_csa_pad_e_b[0][1][9];
  f_csa_pad_e_a[72] = f_csa_pad_e_b[1][3][2];
  f_csa_pad_e_a[73] = f_csa_pad_e_b[0][2][10];
  f_csa_pad_e_a[74] = f_csa_pad_e_b[1][2][2];
  f_csa_pad_e_a[75] = f_csa_pad_e_b[0][1][10];
  f_csa_pad_e_a[76] = f_csa_pad_e_b[1][3][3];
  f_csa_pad_e_a[77] = f_csa_pad_e_b[0][2][11];
  f_csa_pad_e_a[78] = f_csa_pad_e_b[1][2][3];
  f_csa_pad_e_a[79] = f_csa_pad_e_b[0][1][11];
  f_csa_pad_e_a[80] = f_csa_pad_e_b[1][3][4];
  f_csa_pad_e_a[81] = f_csa_pad_e_b[0][2][12];
  f_csa_pad_e_a[82] = f_csa_pad_e_b[1][2][4];
  f_csa_pad_e_a[83] = f_csa_pad_e_b[0][1][12];
  f_csa_pad_e_a[84] = f_csa_pad_e_b[1][3][5];
  f_csa_pad_e_a[85] = f_csa_pad_e_b[0][2][13];
  f_csa_pad_e_a[86] = f_csa_pad_e_b[1][2][5];
  f_csa_pad_e_a[87] = f_csa_pad_e_b[0][1][13];
  f_csa_pad_e_a[88] = f_csa_pad_e_b[1][3][6];
  f_csa_pad_e_a[89] = f_csa_pad_e_b[0][2][14];
  f_csa_pad_e_a[90] = f_csa_pad_e_b[1][2][6];
  f_csa_pad_e_a[91] = f_csa_pad_e_b[0][1][14];
  f_csa_pad_e_a[92] = f_csa_pad_e_b[1][3][7];
  f_csa_pad_e_a[93] = f_csa_pad_e_b[0][2][15];
  f_csa_pad_e_a[94] = f_csa_pad_e_b[1][2][7];
  f_csa_pad_e_a[95] = f_csa_pad_e_b[0][1][15];
  f_csa_pad_e_a[96] = f_csa_pad_e_b[1][3][8];
  f_csa_pad_e_a[97] = f_csa_pad_e_b[0][2][0];
  f_csa_pad_e_a[98] = f_csa_pad_e_b[1][2][8];
  f_csa_pad_e_a[99] = f_csa_pad_e_b[0][1][0];
  f_csa_pad_e_a[100] = f_csa_pad_e_b[1][3][9];
  f_csa_pad_e_a[101] = f_csa_pad_e_b[0][2][1];
  f_csa_pad_e_a[102] = f_csa_pad_e_b[1][2][9];
  f_csa_pad_e_a[103] = f_csa_pad_e_b[0][1][1];
  f_csa_pad_e_a[104] = f_csa_pad_e_b[1][3][10];
  f_csa_pad_e_a[105] = f_csa_pad_e_b[0][2][2];
  f_csa_pad_e_a[106] = f_csa_pad_e_b[1][2][10];
  f_csa_pad_e_a[107] = f_csa_pad_e_b[0][1][2];
  f_csa_pad_e_a[108] = f_csa_pad_e_b[1][3][11];
  f_csa_pad_e_a[109] = f_csa_pad_e_b[0][2][3];
  f_csa_pad_e_a[110] = f_csa_pad_e_b[1][2][11];
  f_csa_pad_e_a[111] = f_csa_pad_e_b[0][1][3];
  f_csa_pad_e_a[112] = f_csa_pad_e_b[1][3][12];
  f_csa_pad_e_a[113] = f_csa_pad_e_b[0][2][4];
  f_csa_pad_e_a[114] = f_csa_pad_e_b[1][2][12];
  f_csa_pad_e_a[115] = f_csa_pad_e_b[0][1][4];
  f_csa_pad_e_a[116] = f_csa_pad_e_b[1][3][13];
  f_csa_pad_e_a[117] = f_csa_pad_e_b[0][2][5];
  f_csa_pad_e_a[118] = f_csa_pad_e_b[1][2][13];
  f_csa_pad_e_a[119] = f_csa_pad_e_b[0][1][5];
  f_csa_pad_e_a[120] = f_csa_pad_e_b[1][3][14];
  f_csa_pad_e_a[121] = f_csa_pad_e_b[0][2][6];
  f_csa_pad_e_a[122] = f_csa_pad_e_b[1][2][14];
  f_csa_pad_e_a[123] = f_csa_pad_e_b[0][1][6];
  f_csa_pad_e_a[124] = f_csa_pad_e_b[1][3][15];
  f_csa_pad_e_a[125] = f_csa_pad_e_b[0][2][7];
  f_csa_pad_e_a[126] = f_csa_pad_e_b[1][2][15];
  f_csa_pad_e_a[127] = f_csa_pad_e_b[0][1][7];
  f_csa_pad_e_a[128] = f_csa_pad_e_b[1][7][0];
  f_csa_pad_e_a[129] = f_csa_pad_e_b[0][6][8];
  f_csa_pad_e_a[130] = f_csa_pad_e_b[1][6][0];
  f_csa_pad_e_a[131] = f_csa_pad_e_b[0][5][8];
  f_csa_pad_e_a[132] = f_csa_pad_e_b[1][7][1];
  f_csa_pad_e_a[133] = f_csa_pad_e_b[0][6][9];
  f_csa_pad_e_a[134] = f_csa_pad_e_b[1][6][1];
  f_csa_pad_e_a[135] = f_csa_pad_e_b[0][5][9];
  f_csa_pad_e_a[136] = f_csa_pad_e_b[1][7][2];
  f_csa_pad_e_a[137] = f_csa_pad_e_b[0][6][10];
  f_csa_pad_e_a[138] = f_csa_pad_e_b[1][6][2];
  f_csa_pad_e_a[139] = f_csa_pad_e_b[0][5][10];
  f_csa_pad_e_a[140] = f_csa_pad_e_b[1][7][3];
  f_csa_pad_e_a[141] = f_csa_pad_e_b[0][6][11];
  f_csa_pad_e_a[142] = f_csa_pad_e_b[1][6][3];
  f_csa_pad_e_a[143] = f_csa_pad_e_b[0][5][11];
  f_csa_pad_e_a[144] = f_csa_pad_e_b[1][7][4];
  f_csa_pad_e_a[145] = f_csa_pad_e_b[0][6][12];
  f_csa_pad_e_a[146] = f_csa_pad_e_b[1][6][4];
  f_csa_pad_e_a[147] = f_csa_pad_e_b[0][5][12];
  f_csa_pad_e_a[148] = f_csa_pad_e_b[1][7][5];
  f_csa_pad_e_a[149] = f_csa_pad_e_b[0][6][13];
  f_csa_pad_e_a[150] = f_csa_pad_e_b[1][6][5];
  f_csa_pad_e_a[151] = f_csa_pad_e_b[0][5][13];
  f_csa_pad_e_a[152] = f_csa_pad_e_b[1][7][6];
  f_csa_pad_e_a[153] = f_csa_pad_e_b[0][6][14];
  f_csa_pad_e_a[154] = f_csa_pad_e_b[1][6][6];
  f_csa_pad_e_a[155] = f_csa_pad_e_b[0][5][14];
  f_csa_pad_e_a[156] = f_csa_pad_e_b[1][7][7];
  f_csa_pad_e_a[157] = f_csa_pad_e_b[0][6][15];
  f_csa_pad_e_a[158] = f_csa_pad_e_b[1][6][7];
  f_csa_pad_e_a[159] = f_csa_pad_e_b[0][5][15];
  f_csa_pad_e_a[160] = f_csa_pad_e_b[1][7][8];
  f_csa_pad_e_a[161] = f_csa_pad_e_b[0][6][0];
  f_csa_pad_e_a[162] = f_csa_pad_e_b[1][6][8];
  f_csa_pad_e_a[163] = f_csa_pad_e_b[0][5][0];
  f_csa_pad_e_a[164] = f_csa_pad_e_b[1][7][9];
  f_csa_pad_e_a[165] = f_csa_pad_e_b[0][6][1];
  f_csa_pad_e_a[166] = f_csa_pad_e_b[1][6][9];
  f_csa_pad_e_a[167] = f_csa_pad_e_b[0][5][1];
  f_csa_pad_e_a[168] = f_csa_pad_e_b[1][7][10];
  f_csa_pad_e_a[169] = f_csa_pad_e_b[0][6][2];
  f_csa_pad_e_a[170] = f_csa_pad_e_b[1][6][10];
  f_csa_pad_e_a[171] = f_csa_pad_e_b[0][5][2];
  f_csa_pad_e_a[172] = f_csa_pad_e_b[1][7][11];
  f_csa_pad_e_a[173] = f_csa_pad_e_b[0][6][3];
  f_csa_pad_e_a[174] = f_csa_pad_e_b[1][6][11];
  f_csa_pad_e_a[175] = f_csa_pad_e_b[0][5][3];
  f_csa_pad_e_a[176] = f_csa_pad_e_b[1][7][12];
  f_csa_pad_e_a[177] = f_csa_pad_e_b[0][6][4];
  f_csa_pad_e_a[178] = f_csa_pad_e_b[1][6][12];
  f_csa_pad_e_a[179] = f_csa_pad_e_b[0][5][4];
  f_csa_pad_e_a[180] = f_csa_pad_e_b[1][7][13];
  f_csa_pad_e_a[181] = f_csa_pad_e_b[0][6][5];
  f_csa_pad_e_a[182] = f_csa_pad_e_b[1][6][13];
  f_csa_pad_e_a[183] = f_csa_pad_e_b[0][5][5];
  f_csa_pad_e_a[184] = f_csa_pad_e_b[1][7][14];
  f_csa_pad_e_a[185] = f_csa_pad_e_b[0][6][6];
  f_csa_pad_e_a[186] = f_csa_pad_e_b[1][6][14];
  f_csa_pad_e_a[187] = f_csa_pad_e_b[0][5][6];
  f_csa_pad_e_a[188] = f_csa_pad_e_b[1][7][15];
  f_csa_pad_e_a[189] = f_csa_pad_e_b[0][6][7];
  f_csa_pad_e_a[190] = f_csa_pad_e_b[1][6][15];
  f_csa_pad_e_a[191] = f_csa_pad_e_b[0][5][7];
  f_csa_pad_e_a[192] = f_csa_pad_e_b[1][5][0];
  f_csa_pad_e_a[193] = f_csa_pad_e_b[0][4][8];
  f_csa_pad_e_a[194] = f_csa_pad_e_b[1][4][0];
  f_csa_pad_e_a[195] = f_csa_pad_e_b[0][3][8];
  f_csa_pad_e_a[196] = f_csa_pad_e_b[1][5][1];
  f_csa_pad_e_a[197] = f_csa_pad_e_b[0][4][9];
  f_csa_pad_e_a[198] = f_csa_pad_e_b[1][4][1];
  f_csa_pad_e_a[199] = f_csa_pad_e_b[0][3][9];
  f_csa_pad_e_a[200] = f_csa_pad_e_b[1][5][2];
  f_csa_pad_e_a[201] = f_csa_pad_e_b[0][4][10];
  f_csa_pad_e_a[202] = f_csa_pad_e_b[1][4][2];
  f_csa_pad_e_a[203] = f_csa_pad_e_b[0][3][10];
  f_csa_pad_e_a[204] = f_csa_pad_e_b[1][5][3];
  f_csa_pad_e_a[205] = f_csa_pad_e_b[0][4][11];
  f_csa_pad_e_a[206] = f_csa_pad_e_b[1][4][3];
  f_csa_pad_e_a[207] = f_csa_pad_e_b[0][3][11];
  f_csa_pad_e_a[208] = f_csa_pad_e_b[1][5][4];
  f_csa_pad_e_a[209] = f_csa_pad_e_b[0][4][12];
  f_csa_pad_e_a[210] = f_csa_pad_e_b[1][4][4];
  f_csa_pad_e_a[211] = f_csa_pad_e_b[0][3][12];
  f_csa_pad_e_a[212] = f_csa_pad_e_b[1][5][5];
  f_csa_pad_e_a[213] = f_csa_pad_e_b[0][4][13];
  f_csa_pad_e_a[214] = f_csa_pad_e_b[1][4][5];
  f_csa_pad_e_a[215] = f_csa_pad_e_b[0][3][13];
  f_csa_pad_e_a[216] = f_csa_pad_e_b[1][5][6];
  f_csa_pad_e_a[217] = f_csa_pad_e_b[0][4][14];
  f_csa_pad_e_a[218] = f_csa_pad_e_b[1][4][6];
  f_csa_pad_e_a[219] = f_csa_pad_e_b[0][3][14];
  f_csa_pad_e_a[220] = f_csa_pad_e_b[1][5][7];
  f_csa_pad_e_a[221] = f_csa_pad_e_b[0][4][15];
  f_csa_pad_e_a[222] = f_csa_pad_e_b[1][4][7];
  f_csa_pad_e_a[223] = f_csa_pad_e_b[0][3][15];
  f_csa_pad_e_a[224] = f_csa_pad_e_b[1][5][8];
  f_csa_pad_e_a[225] = f_csa_pad_e_b[0][4][0];
  f_csa_pad_e_a[226] = f_csa_pad_e_b[1][4][8];
  f_csa_pad_e_a[227] = f_csa_pad_e_b[0][3][0];
  f_csa_pad_e_a[228] = f_csa_pad_e_b[1][5][9];
  f_csa_pad_e_a[229] = f_csa_pad_e_b[0][4][1];
  f_csa_pad_e_a[230] = f_csa_pad_e_b[1][4][9];
  f_csa_pad_e_a[231] = f_csa_pad_e_b[0][3][1];
  f_csa_pad_e_a[232] = f_csa_pad_e_b[1][5][10];
  f_csa_pad_e_a[233] = f_csa_pad_e_b[0][4][2];
  f_csa_pad_e_a[234] = f_csa_pad_e_b[1][4][10];
  f_csa_pad_e_a[235] = f_csa_pad_e_b[0][3][2];
  f_csa_pad_e_a[236] = f_csa_pad_e_b[1][5][11];
  f_csa_pad_e_a[237] = f_csa_pad_e_b[0][4][3];
  f_csa_pad_e_a[238] = f_csa_pad_e_b[1][4][11];
  f_csa_pad_e_a[239] = f_csa_pad_e_b[0][3][3];
  f_csa_pad_e_a[240] = f_csa_pad_e_b[1][5][12];
  f_csa_pad_e_a[241] = f_csa_pad_e_b[0][4][4];
  f_csa_pad_e_a[242] = f_csa_pad_e_b[1][4][12];
  f_csa_pad_e_a[243] = f_csa_pad_e_b[0][3][4];
  f_csa_pad_e_a[244] = f_csa_pad_e_b[1][5][13];
  f_csa_pad_e_a[245] = f_csa_pad_e_b[0][4][5];
  f_csa_pad_e_a[246] = f_csa_pad_e_b[1][4][13];
  f_csa_pad_e_a[247] = f_csa_pad_e_b[0][3][5];
  f_csa_pad_e_a[248] = f_csa_pad_e_b[1][5][14];
  f_csa_pad_e_a[249] = f_csa_pad_e_b[0][4][6];
  f_csa_pad_e_a[250] = f_csa_pad_e_b[1][4][14];
  f_csa_pad_e_a[251] = f_csa_pad_e_b[0][3][6];
  f_csa_pad_e_a[252] = f_csa_pad_e_b[1][5][15];
  f_csa_pad_e_a[253] = f_csa_pad_e_b[0][4][7];
  f_csa_pad_e_a[254] = f_csa_pad_e_b[1][4][15];
  f_csa_pad_e_a[255] = f_csa_pad_e_b[0][3][7];
	
  for (l_i=0; l_i<256; l_i++)
	{
		h_csa_pad_e->SetBinContent (l_i, f_csa_pad_e_a[l_i]);
	}

#endif




bad_event:


  if ( (l_evt_ct % 1000) == 0)
  {
    printf ("------------------------------------------------------\n");
    printf ("nr of events processed:                  %10u \n", l_evt_ct);
    printf ("nr of good_energies found:               %10u \n", l_good_energy_ct);
    printf ("single hits found in trigger window:     %10u \n", l_1_hit_ct);
    printf ("multi hits found in trigger window:      %10u \n", l_more_1_hit_ct);
    printf ("multi hits found, first hit energy ok:   %10u \n", l_more_1_hit_first_energy_ok_ct);
    printf ("nr of events with failed fpga energy:    %10u \n", l_eeeeee_ct);
    printf ("nr of events with eeeee1:                %10u \n", l_eeeee1_ct);
    printf ("nr of events with eeeee2:                %10u \n", l_eeeee2_ct);
    printf ("nr of events with eeeee3:                %10u \n", l_eeeee3_ct);
    printf ("nr of events with failed fpga energy - \n");
    printf ("and a single hit found in trigger window %10u \n", l_1_hit_and_eeeeee_ct);
    printf ("nr of events with failed fpga energy - \n");
    printf ("and multi hits found in trigger window   %10u \n", l_more_1_hit_and_eeeeee_ct);
    printf ("------------------------------------------------------\n");
    fflush (stdout);
  } 
 } // while subevents


  if (fParam->fSlowMotion)
     {
         Int_t evnum=fInput->GetCount();
         GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
     }

  return kTRUE;
}

//--------------------------------------------------------------------------------------------------------

void TGemCSABasicProc::f_make_histo (Int_t l_mode)
{
  Text_t chis[256];
  Text_t chead[256];
  Text_t cfolder[256];
  UInt_t l_i, l_j, l_k;
  UInt_t l_tra_size;
  UInt_t l_trap_n_avg;
//  UInt_t l_left;
//  UInt_t l_right;

  #ifdef USE_MBS_PARAM
  l_tra_size   = l_trace & 0xffff;
  l_trap_n_avg = l_e_filt >> 21;
  printf ("f_make_histo: trace size: %d, avg size %d \n", l_tra_size, l_trap_n_avg);
  fflush (stdout);
  l_sfp_slaves[0] =  l_slaves & 0xff;   
  l_sfp_slaves[1] = (l_slaves & 0xff00)     >>  8;
  l_sfp_slaves[2] = (l_slaves & 0xff0000)   >> 16;
  l_sfp_slaves[3] = (l_slaves & 0xff000000) >> 24;
  printf ("f_make_histo: # of sfp slaves: 3:%d, 2:%d, 1: %d, 0: %d \n",
          l_sfp_slaves[3], l_sfp_slaves[2], l_sfp_slaves[1], l_sfp_slaves[0]);
  fflush (stdout);
  #else
  l_tra_size   = TRACE_SIZE;
  l_trap_n_avg = TRAPEZ_N_AVG;
  #endif // USE_MBS_PARAM      

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
#ifdef      USE_OLD_CSA_BASELINES
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"CSA BASE/average baseline SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Average baseline");
          h_csa_base[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,-0x4000,0x4000);  // TODO create new histograms for fitter




        }
        for (l_k=0; l_k<N_CHA; l_k++)
        {
          sprintf(chis,"CSA SIGNAL/average signal - average baseline SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead,"Average baseline");
          h_csa_signal[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,-0x4000,0x4000);
        }
#endif
        // JAM2021: define regions for evaluation of average background and signal height:
        fxBackgroundRegion=MakeWinCond("BackgroundRegion",CSA_BASE_START, CSA_BASE_START + CSA_BASE_SIZE);
        fxSignalRegion=MakeWinCond("SignalRegion", CSA_SIGNAL_START, CSA_SIGNAL_START+ CSA_SIGNAL_SIZE);





        if(fParam->fDoBaselineFits)
        {
            // here fit the height of the

          for (l_k=0; l_k<N_CHA; l_k++)
          {
            sprintf(chis,"BaselineFits/Models/FitModel SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
            sprintf(chead,"Fit Model  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
            h_trace_fitresult[l_i][l_j][l_k] = MakeTH1('F', chis,chead,l_tra_size,0,l_tra_size);
          }

          for (l_k=0; l_k<N_CHA; l_k++)
          {
            sprintf(chis,"BaselineFits/Chi2/Chi2  SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
            sprintf(chead,"Fit Chi2 SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
            h_fit_chi2[l_i][l_j][l_k] = MakeTH1('F', chis,chead,1000,0,10);
          }
          for (int n=0; n<GEMCSA_FITPOLYPARS; ++n)
            {
                Double_t range=GEMCSA_RANGE_A0;
                if(n>0) range=GEMCSA_RANGE_A1; // JAM2020 kludge; later should be array of predefined ranges for each order...


                sprintf(chis,"BaselineFits/A%d/A%2d all channels", n, n);
                sprintf(chead,"Fit Polygon par %d for all channels", n);
                h_fit_a_all[n]= MakeTH1('F', chis,chead,GEMCSA_FITPARBINS,-range,range);

                for (l_k=0; l_k<N_CHA; l_k++)
                     {
                       sprintf(chis,"BaselineFits/A%d/A%2d SFP: %2d FEBEX: %2d CHAN: %2d", n, n, l_i, l_j, l_k);
                       sprintf(chead,"Fit Polygon par %d SFP: %2d FEBEX: %2d CHAN: %2d", n, l_i, l_j, l_k);
                       h_fit_par[l_i][l_j][l_k][n] = MakeTH1('F', chis,chead,GEMCSA_FITPARBINS,-range,range);
                     }


            } // n



        } //   if(fParam->fDoBaselineFits)

          // JAM 2021: without the fitter, we define result histograms for background and signal average:
          for (l_k=0; l_k<N_CHA; l_k++)
                   {
                     sprintf(chis,"Baselines/Background/Background SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     sprintf(chead,"Average background region height SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     h_background_height[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000, 0, 0x8000);

                     sprintf(chis,"Baselines/Signals/Signals SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     sprintf(chead,"Average signal region height SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     h_signal_height[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,0,0x8000);

                     sprintf(chis,"Baselines/StoB/Signal_background_ratio SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     sprintf(chead,"Average signal to background ratio SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     h_signal_to_background[l_i][l_j][l_k] = MakeTH1('F', chis,chead,1000,0,5);

                     sprintf(chis,"Baselines/S-B/Signal_minus_background_SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     sprintf(chead,"Average signal minus background SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
                     h_signal_minus_background[l_i][l_j][l_k] = MakeTH1('F', chis,chead,0x8000,-0x4000,0x4000);
                   }


      }
    }
  }

#ifdef      USE_OLD_CSA_BASELINES
  sprintf(chis,"CSA SIGNAL SUM");
  sprintf(chead,"Summed CSA Signals");
  h_csa_sum_sig =  MakeTH1('F', chis,chead,250,0,100000); 
  sprintf(chis,"PEAK REFERENCE SIGNAL");
  sprintf(chead,"(peak - base) refefence signal ");
  h_peak_ref_sig =  MakeTH1('F', chis,chead,1000,0,1000);

  sprintf (chis,"Correlation Peak Ref vs. CSA sum");
  sprintf (chead,"Peak Refernence vs CSA_SUM ");
  h_peak_ref__sum_csa = MakeTH2 ('F', chis, chead, 200, 0, 2000, 200, 0, 1000);

  sprintf (chis,"Correlation Peak Ref vs. CSA sum 2");
  sprintf (chead,"Peak Refernence vs CSA_SUM 2");
  h_peak_ref__sum_csa2 = MakeTH2 ('F', chis, chead, 200, 0, 2000, 200, 0, 1000);

  sprintf(chis,"Pad Energy");
  sprintf(chead,"Pad Energy (PadNumber)");
  h_csa_pad_e =  MakeTH1('F', chis,chead,256,0,256); 
#endif

  if(fParam->fDoBaselineFits)
        {

    // JAM2020 - put trace and fit into common pictures:

  for (l_i=0; l_i<MAX_SFP; l_i++)
   {
     if (l_sfp_slaves[l_i] != 0)
     {
       for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
       {
         sprintf (cfolder,"BaselineFits/SFP0%2d_Slave%2d",l_i,l_j);
         for (l_k=0; l_k<N_CHA; l_k++)
         {
           sprintf (chis,"PictureTracefit_%2d_%2d_%2d",l_i,l_j,l_k);
           TGo4Picture* pic = GetPicture(chis);
           if (pic == 0)
            {
               sprintf(chead,"Trace Fit SFP: %2d FEBEX: %2d CHAN: %2d",l_i,l_j,l_k);
               pic = new TGo4Picture(chis, chead);
               pic->SetDivision(2, 2);
               pic->Pic(0, 0)->AddObject(h_trace[l_i][l_j][l_k]);
               pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
               pic->Pic(0, 0)->AddObject(h_trace_fitresult[l_i][l_j][l_k]);
               pic->Pic(0, 0)->SetLineAtt(5, 1, 1);    // solid line
               pic->Pic(0, 0)->AddObject(fxBackgroundRegion); // one region for all
               pic->Pic(1, 0)->AddObject (h_fit_chi2[l_i][l_j][l_k]);
               pic->Pic(0, 1)->AddObject ( h_fit_par[l_i][l_j][l_k][0]);
               pic->Pic(1, 1)->AddObject ( h_fit_par[l_i][l_j][l_k][1]);
               AddPicture(pic,cfolder);
            }
         }


         }


       }
     }
   }
  else
  {
    // JAM2021 - new picture for baseline without fit:
    for (l_i=0; l_i<MAX_SFP; l_i++)
       {
         if (l_sfp_slaves[l_i] != 0)
         {
           for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
           {
             sprintf (cfolder,"Baselines/SFP0%2d_Slave%2d",l_i,l_j);
             for (l_k=0; l_k<N_CHA; l_k++)
             {
               sprintf (chis,"Picture_baselines_%2d_%2d_%2d",l_i,l_j,l_k);
               TGo4Picture* pic = GetPicture(chis);
               if (pic == 0)
                {
                   sprintf(chead,"Trace Baselines SFP: %2d FEBEX: %2d CHAN: %2d",l_i,l_j,l_k);
                   pic = new TGo4Picture(chis, chead);
                   pic->SetDivision(2, 2);
                   pic->Pic(0, 0)->AddObject(h_trace[l_i][l_j][l_k]);
                   //pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
                   pic->Pic(0, 0)->AddObject(fxBackgroundRegion); // one region for all
                   pic->Pic(0, 0)->AddObject(fxSignalRegion); // one region for all
                   pic->Pic(0, 1)->AddObject (h_background_height[l_i][l_j][l_k]);
                   //pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
                   pic->Pic(0, 1)->AddObject (h_signal_height[l_i][l_j][l_k]);
                   pic->Pic(1, 0)->AddObject (h_signal_minus_background[l_i][l_j][l_k]);
                   pic->Pic(1, 1)->AddObject (h_signal_to_background[l_i][l_j][l_k]);
                   AddPicture(pic,cfolder);
                }
             }


             }


           }
         }

  }





}



Bool_t TGemCSABasicProc::DoTraceFit(UInt_t sfp, UInt_t feb, UInt_t ch)
{

  if (!fParam->fDoBaselineFits) return kFALSE;
   TH1* hist=h_trace[sfp][feb][ch];
   TH1* fithist=h_trace_fitresult[sfp][feb][ch];
   TH1* fitchi2=h_fit_chi2[sfp][feb][ch];

   TH1* par_hist[GEMCSA_FITPOLYPARS];
   for(int n=0; n<GEMCSA_FITPOLYPARS; ++n)
      {
         par_hist[n] = h_fit_par[sfp][feb][ch][n];
      }

   fithist->Reset(""); // need to remove previous fit display first!

   TGo4Fitter fitter("Fitter", TGo4Fitter::ff_chi_square, kTRUE);

   //TGo4FitDataHistogram* hd=
    fitter.AddH1("data1", hist, kFALSE, fxBackgroundRegion->GetXLow(), fxBackgroundRegion->GetXUp());
   //     hd->SetExcludeLessThen(-1000.0); // TODO: parameter entry - usually fitter will exclude everything below 0 from fit!

   fitter.AddPolynomX("data1", "Pol", GEMCSA_FITPOLYPARS-1);


   for(int t=0;t<3;++t)
          fitter.DoActions();

   //fitter.Print("Pars");

   TH1* h1 = dynamic_cast<TH1*> (fitter.CreateDrawObject("abc", "data1", kTRUE));
      if (h1!=0) {
         //std::cout <<"Created draw model for histogram "<<hist->GetName() <<" of size"<<  h1->GetNbinsX()<< std::endl;
         //int binsize= TMath::Min(fithist->GetNbinsX(), h1->GetNbinsX());
         //std::cout <<"Copy to display object "<<fithist->GetName() <<" of size"<<  fithist->GetNbinsX()<< std::endl;
         for (int n=0;n<fithist->GetNbinsX();n++)
           fithist->SetBinContent(n+1, h1->GetBinContent(n+1));
         delete h1;
      }

  // TODO: fill histograms with fit results
        for(int n=0; n<GEMCSA_FITPOLYPARS; ++n)
        {
          TString parname=TString::Format("Pol_%d.Ampl",n);
          Double_t val = fitter.GetParValue(parname.Data());
         // printf("fit histogram for %d %d %d found value %e of parameter %d - %s \n",
           //   sfp, feb, ch, val, n, parname.Data());
          par_hist[n]->Fill(val);
          h_fit_a_all[n]->Fill(val);
        }

        Double_t ndf=fitter.CalculateNDF();
        Double_t chiquadrat=fitter.CalculateFitFunction();
        if(ndf) chiquadrat=chiquadrat/ndf;
        fitchi2->Fill(chiquadrat);


  return kTRUE;
}




//----------------------------END OF GO4 SOURCE FILE ---------------------
