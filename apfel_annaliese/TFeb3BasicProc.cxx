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
#include "TFeb3BasicProc.h"
#include "stdint.h"

#include <vector>
#include <algorithm>

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TGo4WinCond.h"
#include "TCutG.h"
#include "snprintf.h"


#include "TGo4MbsEvent.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CondArray.h"
#include "TGo4Picture.h"
//#include "TFeb3BasicParam.h" // done in header file JAM
#include "TGo4Fitter.h"
#include "TGo4FitPeakFinder.h"
#include "TApfelModel.h"
#include "TGo4FitModelGauss1.h"
#include "TGo4FitParameter.h"
#include "TGo4FitDataHistogram.h"
#include "TGo4Analysis.h"


#define APFELFIT_USE_DRAWOBJECT 1



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

/** JAM: this is used to check if we have to rebuild the 2d trace histos*/
static Int_t l_lastbinscaledown=PEXOR_TRACE_BINSCALE;

//***********************************************************
TFeb3BasicProc::TFeb3BasicProc() : TGo4EventProcessor("Proc"),fEventSequenceNumber(0)
{
  cout << "**** TFeb3BasicProc: Create instance " << endl;
}
//***********************************************************
TFeb3BasicProc::~TFeb3BasicProc()
{
  cout << "**** TFeb3BasicProc: Delete instance " << endl;
}
//***********************************************************
// this one is used in standard factory
TFeb3BasicProc::TFeb3BasicProc(const char* name) : TGo4EventProcessor(name),fEventSequenceNumber(0)
{
  cout << "**** TFeb3BasicProc: Create instance " << name << endl;

  //printf ("Histograms created \n");  fflush (stdout);
  fPar = dynamic_cast<TFeb3BasicParam*>(MakeParameter("Feb3BasicParam", "TFeb3BasicParam", "set_Feb3BasicParam.C"));
  l_first=0; // JAM need this to avoid crash after second submit

}
//-----------------------------------------------------------
// event function
Bool_t TFeb3BasicProc::BuildEvent(TGo4EventElement* target)
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
  //UInt_t         l_hit_id;
  UInt_t         l_hit_cha_id;
  Long64_t       ll_time;
  Long64_t       ll_trg_time;
  //Long64_t       ll_hit_time;
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
  UInt_t         l_fpga_e_found   [MAX_SFP][MAX_SLAVE][N_CHA]; 
  UInt_t         l_trapez_e       [MAX_SFP][MAX_SLAVE][N_CHA];  
  UInt_t         l_fpga_e         [MAX_SFP][MAX_SLAVE][N_CHA]; 

  UInt_t         l_dat_fir;
  UInt_t         l_dat_sec;

  UInt_t         l_bls_start = BASE_LINE_SUBT_START;
  UInt_t         l_bls_stop  = BASE_LINE_SUBT_START + BASE_LINE_SUBT_SIZE; // 
  Double_t       f_bls_val=0.;

  Int_t       l_fpga_filt_on_off;
  Int_t       l_fpga_filt_mode;
  Int_t       l_dat_trace;
  Int_t       l_dat_filt;
  Int_t       l_filt_sign;   

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
  //cout << "**** TFeb3BasicProc: Skip trigger event"<<endl;
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
  //while((psubevt = fInput->NextSubEvent()) != 0) // loop over subevents
  //{

  psubevt = fInput->NextSubEvent(); // only one subevent    
  
  //printf ("         psubevt: 0x%x \n", (UInt_t)psubevt); fflush (stdout);
  //printf ("%d -------------next event-----------\n", l_evt_ct);
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
    printf ("pl_se_dat: 0x%lx, ", (unsigned long) pl_se_dat);
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

  if (l_first == 0 || l_lastbinscaledown!=fPar->fGridTraceDownscale)
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
          l_fpga_e_found[l_i][l_j][l_k] = 0;
          l_trapez_e    [l_i][l_j][l_k] = 0;
          l_fpga_e      [l_i][l_j][l_k] = 0;
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

                l_fpga_e_found [l_sfp_id][l_feb_id][l_hit_cha_id] = 1;
                l_fpga_e[l_sfp_id][l_feb_id][l_hit_cha_id] = l_fpga_energy;
              }
            }
            else 
            {
              l_eeeeee_ct++;
              if (l_dat == 0xeeeee1) {l_eeeee1_ct++;}
              if (l_dat == 0xeeeee2) {l_eeeee2_ct++;}
              if (l_dat == 0xeeeee3) {l_eeeee3_ct++;}

              if (l_only_one_hit_in_cha    == 1) {l_1_hit_and_eeeeee_ct++;}
              if (l_more_than_1_hit_in_cha == 1) {l_more_1_hit_and_eeeeee_ct++;}  
                  
              //printf ("%d 0xeeeeee found \n", l_eeeeee_ct);
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
        l_fpga_filt_mode   = (l_trace_head & 0x40000) >> 18;
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

          // find base line value of trace and correct it to baseline 0

#ifdef USE_BASELINE_CONDITION
          int denom=c_baseline_region->GetXUp()- c_baseline_region->GetXLow();
          f_bls_val = 0.;
          if(denom)
            f_bls_val=c_baseline_region->GetIntegral(h_trace[l_sfp_id][l_feb_id][l_cha_id]) /denom;

#else
          f_bls_val = 0.;
          for (l_l=l_bls_start; l_l<l_bls_stop; l_l++) 
          {
            f_bls_val += (Double_t)l_tr[l_l];
          }
          f_bls_val = f_bls_val / (Double_t)(l_bls_stop - l_bls_start);
#endif
          for (l_l=0; l_l<l_trace_size; l_l++)   // create baseline restored trace 
          {
            f_tr_blr[l_l] =  (Double_t)l_tr[l_l] - f_bls_val;
            h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->Fill (l_l, f_tr_blr[l_l]);

            // JAM here is trace data for wire mapping available
            // either buffer to output event, or reverse search for board, grid wire here-
            // neither of this, we just use trace ble histograms as data buffer and use them in grid loop below.
            //h_trace_blr[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent (l_l+1, f_tr_blr[l_l]);
          }

          // find peak and fill histogram
          h_peak  [l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMaximum ());
          h_valley[l_sfp_id][l_feb_id][l_cha_id]->Fill (h_trace[l_sfp_id][l_feb_id][l_cha_id]->GetMinimum ());


          // here optionally try to fit peaks with the apfel model:
          if (fPar->fDoPeakFit)
          {
              DoMultiPeakFit(l_sfp_id, l_feb_id,l_cha_id);
          }
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


  FillGrids(); // JAM here is mapping of traceBLR to grid wires-



bad_event:


  if ( (l_evt_ct % 10000) == 0)
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
  } 

  return kTRUE;
}



void TFeb3BasicProc::FillGrids()
{
  // JAM here is mapping of traceBLR to grid wires-
  if (!fPar->fDoGridMapping)
    return;
  for (int g = 0; (g < fPar->fNumGrids) && (g < PEXOR_APFEL_GRIDS); g++)
  {
    h_grid_x_profile[g]->Reset("");
    h_grid_y_profile[g]->Reset("");
    h_grid_xvstrace[g]->Reset("");
    h_grid_yvstrace[g]->Reset("");

    Double_t val = 0;
    Int_t sfp = -1, febex = -1;
    for (int wire = 0; wire < PEXOR_APFEL_WIRES; ++wire)
    {
      int bidX = fPar->fGridBoardID_X[g][wire];
      if (bidX >= 0)
      {
        // evaluate sfp and slave from board id here:
        fPar->FindBoardIndices(bidX, sfp, febex);
        if (sfp >= 0 && febex >= 0)
        {
          //printf("FFFFFFFF find board %d  indices sfp:%d, febex:%d \n",bidX, sfp,febex); std::cout<<std::endl;

          int channelX = fPar->fGridChannel_X[g][wire];
          if (channelX >= 0)
          {
            TH1* theTrace = h_trace_blr[sfp][febex][channelX];
            //printf("FFFFFFFF trace pointer of channelX %d: 0x%x \n",channelX, (ulong) theTrace); std::cout<<std::endl;
            for (int t = 0; (theTrace != 0) && (t < theTrace->GetNbinsX() - 1); ++t)
            {
              val = theTrace->GetBinContent(t + 1);
//              if (val < 0)
//                printf("FFFFFFFF negative value %f for t=%d in board %d, channelX %d wire %d:\n", val, t, bidX,
//                    channelX, wire);
//              std::cout << std::endl;
// JAM negatve values may happen with baseline corrected traces...
              h_grid_x_profile[g]->Fill(wire, val);
              h_grid_x_profile_sum[g]->Fill(wire, val);
              h_grid_xvstrace[g]->Fill(wire, t, val);
              h_grid_xvstrace_sum[g]->Fill(wire, t, val);

            }
          }    // if channelX
        }    // if sfp febex
        else
        {
          printf("Warning: sfp=%d febex=%d for board %d, please check configuration!\n", sfp, febex, bidX);
          std::cout << std::endl;
        }
      }    // if bidX
      int bidY = fPar->fGridBoardID_Y[g][wire];
      if (bidY >= 0)
      {
        // evaluate sfp and slave from board id here:
        fPar->FindBoardIndices(bidY, sfp, febex);
        if (sfp >= 0 && febex >= 0)
        {
          int channelY = fPar->fGridChannel_Y[g][wire];
          if (channelY >= 0)
          {
            TH1* theTrace = h_trace_blr[sfp][febex][channelY];
            //printf("FFFFFFFF trace pointer of channelY %d: 0x%x \n",channelY,(ulong) theTrace); std::cout<<std::endl;
            for (int t = 0; (theTrace != 0) && (t < theTrace->GetNbinsX() - 1); ++t)
            {
              val = theTrace->GetBinContent(t + 1);
//              if (val < 0)
//                printf("FFFFFFFF negative value %f for t=%d in board %d, channelY %d :\n", val, t, bidY, channelY);
//              std::cout << std::endl;

              h_grid_y_profile[g]->Fill(wire, val);
              h_grid_y_profile_sum[g]->Fill(wire, val);
              h_grid_yvstrace[g]->Fill(wire, t, val);
              h_grid_yvstrace_sum[g]->Fill(wire, t, val);
            }
          }    // if channelY
        }    // if sfp febex
        else
        {
          printf("Warning: sfp=%d febex=%d for board %d, please check configuration!\n", sfp, febex, bidY);
          std::cout << std::endl;
        }

      }    // if bidY
    }    // wires
  }    // grid index


  fEventSequenceNumber++; // for the moment, we just increase local number since last submit/analysis start


  if (fPar->fSlowMotionStart > 0)
      if (fEventSequenceNumber >=fPar->fSlowMotionStart)
        GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode at event of sequence number %d",
            fEventSequenceNumber);




}




//--------------------------------------------------------------------------------------------------------

void TFeb3BasicProc:: f_make_histo (Int_t l_mode)
{
  Text_t chis[256];
  Text_t chead[256];
  UInt_t l_i, l_j, l_k, l_p;
  UInt_t l_tra_size;
  UInt_t l_trap_n_avg;
  UInt_t l_left;
  UInt_t l_right;

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

        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          sprintf(chis, "Traces BLR/TRACE, base line restored SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Trace, base line restored");
          h_trace_blr[l_i][l_j][l_k] = MakeTH1('F', chis, chead, l_tra_size, 0, l_tra_size);
        }

        // JAM2016: add histograms to contain results of optional multi peak fit:

        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          sprintf(chis, "PEAKFITS/FITTRACES/FITTRACE of base line restored SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j,
              l_k);
          sprintf(chead, "Fit to Trace, base line restored");
          h_trace_blr_fit[l_i][l_j][l_k] = MakeTH1('F', chis, chead, l_tra_size, 0, l_tra_size);
        }
        for (l_k = 0; l_k < N_CHA; l_k++)
                {
          for(l_p=0; l_p<5; l_p++)
          {
                sprintf(chis, "PEAKFITS/FITTRACES/FITPEAKS of base line restored SFP: %2d FEBEX: %2d CHAN: %2d PEAK: %2d",
                    l_i, l_j, l_k, l_p);
                sprintf(chead, "Fit to BLRTrace %2d %2d %2d, model %2d",l_i, l_j, l_k, l_p);
                h_trace_blr_fitmodel[l_i][l_j][l_k][l_p] = MakeTH1('F', chis, chead, l_tra_size, 0, l_tra_size);
          }
         }


        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          // number of found/fitted peak
          sprintf(chis, "PEAKFITS/MULT/Number of Peaks SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Numpeaks %2d %2d %2d", l_i, l_j, l_k);
          h_num_peaks[l_i][l_j][l_k] = MakeTH1('I', chis, chead, 20, 0., 20., "number of peaks");
        }

        for (l_k = 0; l_k < N_CHA; l_k++)
         {
          // sigma noise from non-peak region:
          sprintf(chis, "PEAKFITS/NOISE/Noise Sigma SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Peak %2d %2d %2d", l_i, l_j, l_k);
          h_sigma[l_i][l_j][l_k] = MakeTH1('I', chis, chead, 1000, 0., 100., "baseline height sigma");
         }


        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          // peak position from bin
          sprintf(chis, "PEAKFITS/HEIGHT/FitPeakHeight SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Peak %2d %2d %2d", l_i, l_j, l_k);
          h_peak_fit[l_i][l_j][l_k] = MakeTH1('I', chis, chead, 2000, 0., 20000., "pulseheight");
        }

        for (l_k = 0; l_k < N_CHA; l_k++)
        {

          // baseline from window
          sprintf(chis, "PEAKFITS/BASELINE/FitBaseline SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Baseline %2d  %2d %2d", l_i, l_j, l_k);
          h_baseline[l_i][l_j][l_k] = MakeTH1('I', chis, chead, 500, 0., 4000., "pulseheight");
        }




        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          // mean position from window
          sprintf(chis, "PEAKFITS/MEANPOS/FitPulseMean SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Peak position %2d  %2d %2d", l_i, l_j, l_k);
          h_meanpos[l_i][l_j][l_k] = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size, "time");
        }
        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          // pulse maximum position from fit
          sprintf(chis, "PEAKFITS/MAXIMUMPOS/FitPulsePos SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Fitted peak position %2d  %2d %2d", l_i, l_j, l_k);
          h_fit_pos[l_i][l_j][l_k] = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size, "time");
        }

        for (l_k = 0; l_k < N_CHA; l_k++)
                  {
          // differences between multiple peak position from fit
          sprintf(chis, "PEAKFITS/DELTAPOS/FitPulseDeltaPos SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Fitted peak position differences %2d  %2d %2d", l_i, l_j, l_k);
          h_fit_deltapos[l_i][l_j][l_k] = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size,
              "delta time");
        }


        for (l_k = 0; l_k < N_CHA; l_k++)
        {
          // differences between multiple peak position from fit
          sprintf(chis, "PEAKFITS/DELTAEDGE/FitPulseDeltaEdge SFP: %2d FEBEX: %2d CHAN: %2d", l_i, l_j, l_k);
          sprintf(chead, "Fitted pulse edge differences %2d  %2d %2d", l_i, l_j, l_k);
          h_fit_deltaedge[l_i][l_j][l_k] = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size,
              "delta time");
        }


////////////////////// end multi peak fit


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
          l_right = 0x1000 * l_trap_n_avg;
          l_left = -1 * l_right;
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
      } //  for (l_j=0;
    } //  if (l_sfp_slaves[l_i] != 0)
  } //  for (l_i=0;



  // JAM some sum histograms for fit results:

  sprintf(chis, "PEAKFITS/MULT/Number of Peaks_ALL");
  sprintf(chead, "Numpeaks all slaves");
  h_num_peaks_all = MakeTH1('I', chis, chead, 20, 0., 20., "number of peaks");

  sprintf(chis, "PEAKFITS/HEIGHT/FitPeakHeight_ALL");
  sprintf(chead, "Peakfit height all slaves");
  h_peak_fit_all = MakeTH1('I', chis, chead, 2000, 0., 20000., "pulseheight");

  sprintf(chis, "PEAKFITS/BASELINE/FitBaseline_ALL");
  sprintf(chead, "Baseline all slaves");
  h_baseline_all = MakeTH1('I', chis, chead, 500, 0., 4000., "pulseheight");

  sprintf(chis, "PEAKFITS/NOISE/Noise Sigma_ALL");
  sprintf(chead, "Noise sigma all slaves");
  h_sigma_all = MakeTH1('I', chis, chead, 1000, 0., 100., "baseline height sigma");

  sprintf(chis, "PEAKFITS/MEANPOS/FitPulseMean_ALL");
  sprintf(chead, "Peak position all slaves");
  h_meanpos_all = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size, "time");

  sprintf(chis, "PEAKFITS/MAXIMUMPOS/FitPulsePos_ALL");
  sprintf(chead, "Fitted peak position all slaves");
  h_fit_pos_all = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size, "time");

  sprintf(chis, "PEAKFITS/DELTAPOS/FitPulseDeltaPos_ALL");
  sprintf(chead, "Fitted peak position differences all slaves");
  h_fit_deltapos_all = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size,   "delta time");

  sprintf(chis, "PEAKFITS/DELTAEDGE/FitPulseDeltaEdge_ALL");
  sprintf(chead, "Fitted pulse edge position differences all slaves");
  h_fit_deltaedge_all = MakeTH1('I', chis, chead, l_tra_size, 0, l_tra_size,   "delta time");







 // JAM put here additional histograms with mapped grid wires:


  int binscaledown =fPar->fGridTraceDownscale; // use this to decrease granularity for the 2d plots

     if (l_lastbinscaledown!=binscaledown)
       {
           if(binscaledown<=0) binscaledown=1;
           printf ("f_make_histo: rebuilding 2d grid displays with new scaledown %d (previous: %d) \n", binscaledown, l_lastbinscaledown);
           l_lastbinscaledown=binscaledown;
           TGo4Analysis::Instance()->SetMakeWithAutosave(false); // overwrite old ones if any
       }


  for (int g = 0; (g < fPar->fNumGrids) && (g < PEXOR_APFEL_GRIDS); g++)
  {
    int gid = fPar->fGridDeviceID[g];

    sprintf(chis, "Grids/Grid_%2d/X_Profile_%2d", gid, gid);
    sprintf(chead, "Grid %2d X Profile (single event)", gid);
    h_grid_x_profile[g] = MakeTH1('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES, "X wire number", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_x_profile[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }

    sprintf(chis, "Grids/Grid_%2d/X_ProfileSum_%2d", gid, gid);
    sprintf(chead, "Grid %2d X Profile (accumulated)", gid);
    h_grid_x_profile_sum[g] = MakeTH1('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES, "X wire number", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_x_profile_sum[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }
    sprintf(chis, "Grids/Grid_%2d/Y_Profile_%2d", gid, gid);
    sprintf(chead, "Grid %2d Y Profile (single event)", gid);
    h_grid_y_profile[g] = MakeTH1('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES, "Y wire number", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_y_profile[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }
    sprintf(chis, "Grids/Grid_%2d/Y_ProfileSum_%2d", gid, gid);
    sprintf(chead, "Grid %2d Y Profile (accumulated)", gid);
    h_grid_y_profile_sum[g] = MakeTH1('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES, "Y wire number", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_y_profile_sum[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }






    sprintf(chis, "Grids/Grid_%2d/X_Trace_%2d", gid, gid);
    sprintf(chead, "Grid %2d X vs Trace time (single event)", gid);
    h_grid_xvstrace[g] = MakeTH2('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES, l_tra_size / binscaledown,
        0, l_tra_size, "X wire number", "trace time", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_xvstrace[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }

    sprintf(chis, "Grids/Grid_%2d/X_TraceSum_%2d", gid, gid);
    sprintf(chead, "Grid %2d X vs Trace time (accumulated)", gid);
    h_grid_xvstrace_sum[g] = MakeTH2('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES,
        l_tra_size / binscaledown, 0, l_tra_size, "X wire number", "trace time", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_xvstrace_sum[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }

    sprintf(chis, "Grids/Grid_%2d/Y_Trace_%2d", gid, gid);
    sprintf(chead, "Grid %2d Y vs Trace time (single event)", gid);
    h_grid_yvstrace[g] = MakeTH2('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES, l_tra_size / binscaledown,
        0, l_tra_size, "Y wire number", "t", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_yvstrace[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }

    sprintf(chis, "Grids/Grid_%2d/Y_TraceSum_%2d", gid, gid);
    sprintf(chead, "Grid %2d Y vs Trace time (accumulated)", gid);
    h_grid_yvstrace_sum[g] = MakeTH2('D', chis, chead, PEXOR_APFEL_WIRES, 0, PEXOR_APFEL_WIRES,
        l_tra_size / binscaledown, 0, l_tra_size, "Y wire number", "t", "N");
    if (IsObjMade())
    {
      for (int bin = 0; bin < PEXOR_APFEL_WIRES; ++bin)
      {
        sprintf(chead, "%2d", bin);
        h_grid_yvstrace_sum[g]->GetXaxis()->SetBinLabel(1 + bin, chead);
      }
    }



  }    // grid

  TGo4Analysis::Instance()->SetMakeWithAutosave(true); // for dynamic bin scale down feature
















  // finally we add some condition to define actual baseline region:

  c_baseline_region = MakeWinCond("BLR_Region",BASE_LINE_SUBT_START, BASE_LINE_SUBT_START+BASE_LINE_SUBT_SIZE);


  // range for peak fitter. We begin directly after the baseline region at initialization time:
  //c_peakfit_region = MakeWinCond("Peakfit_Region",c_baseline_region->GetXUp(), c_baseline_region->GetXUp()+BASE_LINE_SUBT_SIZE);

  c_peakfit_region = MakeWinCond("Peakfit_Region",600, 800); // better: defaults that work with test file JAM

  // threshold of peak maximum before we try to find peaks and fit:
 c_peakheight_threshold = MakeWinCond("Peakfit_Threshold",1000, 20000);


} // end function


Bool_t TFeb3BasicProc::DoMultiPeakFit(UInt_t sfp, UInt_t feb, UInt_t ch)
{
  // JAM: this function is originally taken from old ApfelProc for DoublePeak and other fit (<-Sergei L. improvements)
  // we changed it a bit to allow more than one peak and use root peak finder more suitable for this kind of data?
  //also new mode for iterative fit of peaks

  Double_t baseline(0), ampl(0), position(0), sigma(0);
  Int_t numpeaks(0);
  Double_t posfit[fPar->fFitMaxPeaks + 1];
  Double_t edgefit[fPar->fFitMaxPeaks + 1];



  TH1* hist=h_trace_blr[sfp][feb][ch];
  TH1* fithist=h_trace_blr_fit[sfp][feb][ch];
  fithist->Reset(""); // need to remove previous fit display first!
  for(int p=0; p<MAX_SHOWN_FITMODELS;++p)
       {
              h_trace_blr_fitmodel[sfp][feb][ch][p]->Reset("");
       }


  // first we look for position of maximum in the intended peakfit region:

  position=c_peakfit_region->GetXMax(hist);



  /////////////////////////
    // here we add some peak height threshold before fitting from another version of apfel analysis :-)

// since we work on the baseline corrected traces, do not need the background estimation beforehand here!
//    double k0 = fitter.GetParValue("Pol_0.Ampl");
//    double k1 = fitter.GetParValue("Pol_1.Ampl");
      double k0=0;; // assume that zero line is baseline
      double k1=0.;


    // first we get maximum amplitude with respect to fitted baseline within peak fit region:
    double maxdiff(0);
    int bin = int(c_peakfit_region->GetXLow());
    double maxpos = bin;

    baseline = k0 + bin*k1;


    while (bin<= c_peakfit_region->GetXUp()) {
       double diff = TMath::Abs(k0 + bin*k1 - hist->GetBinContent(bin+1));

       if (diff > maxdiff) {
           maxdiff = diff;
           maxpos = bin;
       }
       bin++;
       // cout << "bin = " << bin << "  value = " << func << endl;
    }

    //then calculate some sigma for the rest of the trace:
    double sum0(0), sum1(0), sum2(0);
    while (bin <= hist->GetNbinsX()) {
       double diff = k0 + bin*k1 - hist->GetBinContent(bin+1);
       sum0 += 1.;
       sum1 += diff;
       sum2 += diff*diff;

       bin++;
    }

    sigma = TMath::Sqrt(sum2/sum0 - TMath::Power(sum1/sum0, 2));

    ampl = maxdiff;

    h_sigma[sfp][feb][ch]->Fill(sigma);
    h_sigma_all->Fill(sigma);

    if (!c_peakheight_threshold->Test(maxdiff)) {
       for (int n=0;n<fithist->GetNbinsX();n++)
          fithist->SetBinContent(n+1, k0+n*k1);
       return kFALSE;
    }

  // here switch to  use either peak finder or successive apfelmodel
  TGo4Fitter fitter("Fitter", TGo4Fitter::ff_chi_square, kTRUE);
  if (fPar->fDoPeakIteration)
  {

      TGo4FitDataHistogram* hd=fitter.AddH1("data1", hist, kFALSE, c_peakfit_region->GetXLow(), c_peakfit_region->GetXUp());
      hd->SetExcludeLessThen(-1000.0); // TODO: parameter entry - usually fitter will exclude everything below 0 from fit!
      fitter.AddPolynomX("data1", "Pol", 0);
    // initial values for first apfelmodel peak from simple evaluation over condition
    Double_t peakampl = maxdiff;
    Double_t peakpos = maxpos;
    for (int n = 0; n < fPar->fFitMaxPeaks; ++n)
    {
      TApfelModel* model = new TApfelModel(Form("Peak%d", n));
      fitter.AddModel("data1", model);
#ifdef       APFELMODEL_USE_AMPLITUDEESTIMATION
      model->SetParsValues(peakampl * 25, 3., 12., peakpos - 15);
#else
      model->SetParsValues( 3., 12., peakpos - 15, peakampl*0.8);
#endif
      model->FindPar("N")->SetFixed(kTRUE);
      fitter.SetParValue("Pol_0.Ampl", k0);
      //fitter.SetParValue("Pol_1.Ampl", k1);

      fitter.SetMemoryUsage(2);

      //   fitter.Print("Pars");
      for(int t=0;t<3;++t)
        fitter.DoActions();

      numpeaks = n+1;
      // here check if we need another model component:

      TH1* allmodel = dynamic_cast<TH1*>(fitter.CreateDrawObject("abc", "data1", kTRUE));
      if (allmodel == 0)
      {
        std::cout << "CreateDrawObject failed for " << hist->GetName() << std::endl;
        return kFALSE;
      }
      double maxdeltafit(0);
      int maxdeltapos(0);
      for (int bin = int(c_peakfit_region->GetXLow()); bin <int(c_peakfit_region->GetXUp()) ; bin++)
      {
        double deltafit = TMath::Abs(allmodel->GetBinContent(bin + 1) - hist->GetBinContent(bin + 1));
        if (deltafit > maxdeltafit)
        {
          maxdeltafit = deltafit;
          maxdeltapos = bin;
        }
      }
      delete allmodel;
      if(maxdeltafit<fPar->fPeakIterationDelta) break; // do not add another apfel peak if we are "good enough"
      peakpos=maxdeltapos; // point of maximum deviation will get another apfel peak
      peakampl=maxdeltafit;
    }    // for n


    if (numpeaks >= fPar->fFitMaxPeaks) return false; // do not account peaks that exceed limit

    baseline = fitter.GetModel(0)->GetAmplValue();
  }
  else
{

  TGo4Fitter fitterpre("FitterPre", TGo4Fitter::ff_chi_square, kTRUE); // this one is just for peak finder
// add histogram to fitter, which should be fitted
   fitterpre.AddH1("data1", hist, kFALSE, c_peakfit_region->GetXLow(), c_peakfit_region->GetXUp());
// Add peak finder for data1, which not remove previous models and
// will use 1-order polynom for background
   TGo4FitPeakFinder* finder = new TGo4FitPeakFinder("Finder", "data1", kFALSE, 0);

   finder->SetupForSecond(fPar->fFitRootPF_Par);
   // JAM experience shows that root TSpectrum fitter is best for this use case
   // we may adjust the finding by user parameter containing the "line width"
   // TODO: implement switches and parameters for other peak finders?

   fitterpre.AddActionAt(finder, 0);
   fitterpre.SetMemoryUsage(2);

   fitterpre.DoActions(kTRUE);

   numpeaks = fitterpre.GetNumModel() - 1;

   if ((numpeaks < 1) || (numpeaks> fPar->fFitMaxPeaks)) return false;

   baseline = fitterpre.GetModel(0)->GetAmplValue();
   /////////////////////////////

   // if gaussian fit was successful, we replace gauss by apfel models and fit again!
   fitter.AddH1("data1", hist, kFALSE, c_peakfit_region->GetXLow(), c_peakfit_region->GetXUp());
   fitter.AddPolynomX("data1", "Pol", 0);
   for (int n=1; n<fitterpre.GetNumModel(); n++) {
      TGo4FitModelGauss1* m = dynamic_cast<TGo4FitModelGauss1*> (fitterpre.GetModel(n));
      double pos(0), ampl(0); //, width(0)
      m->GetPosition(0, pos);
//      m->GetWidth(0, width);
      ampl = m->GetAmplValue();

      TApfelModel* model = new TApfelModel(Form("Peak%d",n-1));
      fitter.AddModel("data1", model);

      model->SetParsValues(ampl*25, 3., 12., pos-15);
      model->FindPar("N")->SetFixed(kTRUE);
   }

   fitter.SetMemoryUsage(2);
   for(int t=0;t<3;++t)
     fitter.DoActions();
    k0 = fitter.GetParValue("Pol_0.Ampl");
   //k1 = fitter.GetParValue("Pol_1.Ampl");
}
//////////////////////// end variant with or without peak finder




#ifdef   APFELFIT_USE_DRAWOBJECT
// this does not work properly? try alternative approach with Eval
   TH1* h1 = dynamic_cast<TH1*> (fitter.CreateDrawObject("abc", "data1", kTRUE));
   if (h1!=0) {
      std::cout <<"Created draw model for histogram "<<hist->GetName() <<" of size"<<  h1->GetNbinsX()<< std::endl;
      //int binsize= TMath::Min(fithist->GetNbinsX(), h1->GetNbinsX());
      std::cout <<"Copy to display object "<<fithist->GetName() <<" of size"<<  fithist->GetNbinsX()<< std::endl;
      for (int n=0;n<fithist->GetNbinsX();n++)
        fithist->SetBinContent(n+1, h1->GetBinContent(n+1));
      delete h1;
   }

   // now try to get apfel peaks separately:
   for(int p=0; (p<numpeaks) && (p<MAX_SHOWN_FITMODELS) ;++p)
      {
         h1 = dynamic_cast<TH1*> (fitter.CreateDrawObject("abc", "data1", kTRUE, Form("Peak%d",p)));
         if (h1!=0) {
           for (int n=0;n<h1->GetNbinsX();n++)
             h_trace_blr_fitmodel[sfp][feb][ch][p]->SetBinContent(n+1, h1->GetBinContent(n+1));
         }
      }



#else
   Double_t val=0;
   for (int n=0;n<fithist->GetNbinsX();n++)
   {
     //         fithist->SetBinContent(n+1, h1->GetBinContent(n+1));
     val=k0 + n*k1;
    for(int p=0; p<numpeaks;++p)
       {
         TApfelModel* model = dynamic_cast<TApfelModel*> (fitter.GetModel(p+1));
         //ampl = model->GetAmplValue(); // JAM do we need this?
         val+= model->Evaluate(n);
       }
     fithist->SetBinContent(n+1, val);
   }
   // todo: different histograms for the fit model components, or access fitter from gui to check components?
#endif




   std::cout <<"Found peak positions for "<<hist->GetName() <<": "<< std::endl;
   for(int p=0; p<numpeaks;++p)
   {
     TApfelModel* apfel1 = dynamic_cast<TApfelModel*> (fitter.GetModel(p+1));
   //cout << "tau = " << tau << "  shift = " << shift << endl;
     if(apfel1==0) {
       std::cout <<p<<": zero apfel model found!!!"<<std::endl;
       continue;
     }

     posfit[p] = apfel1->GetParValue("Tau") + apfel1->GetParValue("Shift"); // model centroid positions
     edgefit[p] = apfel1->GetParValue("Tau")/2 + apfel1->GetParValue("Shift"); // model rising edge half maximum positions

     std::cout <<p<<": tau="<<apfel1->GetParValue("Tau")<<", shift="<<apfel1->GetParValue("Shift");
     std::cout <<", pos="<<  posfit[p];
#ifdef       APFELMODEL_USE_AMPLITUDEESTIMATION
     std::cout <<", amplitude:"<< apfel1->GetAmplValue()<<std::endl;
     ampl+=apfel1->GetAmplValue(); // add amplitudes of all apfel models for the first guess, later ind ampl arrays?
#else
     std::cout <<", amplitude:"<< apfel1->GetParValue("ApfelAmp")<<std::endl;;
     ampl+=apfel1->GetParValue("ApfelAmp");
#endif
   }



   //////////////////////



 // on successful fit fill corresponding histograms with results!
  h_num_peaks[sfp][feb][ch]->Fill(numpeaks);
  h_num_peaks_all->Fill(numpeaks);
  if (numpeaks > 0)
  {
    h_peak_fit[sfp][feb][ch]->Fill(ampl);
    h_peak_fit_all->Fill(ampl);

    h_meanpos[sfp][feb][ch]->Fill(position);
    h_meanpos_all->Fill(position);
    h_baseline[sfp][feb][ch]->Fill(baseline);
    h_baseline_all->Fill(baseline);


    std::vector<double> posfitsort;
    std::vector<double> edgefitsort;
    for (int p = 0; p < numpeaks; ++p)
    {
      h_fit_pos[sfp][feb][ch]->Fill(posfit[p]);
      h_fit_pos_all->Fill(posfit[p]);

      // sort positions before calculating difference
      posfitsort.push_back(posfit[p]);
      edgefitsort.push_back(edgefit[p]);

    }    // for p unsorted

    std::sort(posfitsort.begin(), posfitsort.end());
    std::sort(edgefitsort.begin(), edgefitsort.end());

    // differences between peaks from sorted centroid and rising edge positions:
    for (int p = 0; p < numpeaks; ++p)
    {
      if (p > 0)
           {

             Double_t deltap = posfitsort[p] - posfitsort[p - 1];
             h_fit_deltapos[sfp][feb][ch]->Fill(deltap);
             h_fit_deltapos_all->Fill(deltap);

             deltap = edgefitsort[p] - edgefitsort[p - 1];
             h_fit_deltaedge[sfp][feb][ch]->Fill(deltap);
             h_fit_deltaedge_all->Fill(deltap);
           }
    } // for p sorted
  }    // numpeaks>0
   return kTRUE;
}









//----------------------------END OF GO4 SOURCE FILE ---------------------
