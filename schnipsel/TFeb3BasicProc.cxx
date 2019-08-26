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

#include "Riostream.h"

using namespace std;

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TVirtualFFT.h"
#include "TMath.h"
#include "snprintf.h"

#include "TGo4MbsEvent.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CondArray.h"
#include "TGo4Picture.h"
#include "TFeb3BasicParam.h"
#include "TGo4Condition.h"

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
TFeb3BasicProc::TFeb3BasicProc() : TGo4EventProcessor("Proc")
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
TFeb3BasicProc::TFeb3BasicProc(const char* name) : TGo4EventProcessor(name)
{
  cout << "**** TFeb3BasicProc: Create instance " << name << endl;

  fPar = dynamic_cast<TFeb3BasicParam*>(MakeParameter("Feb3BasicParam", "TFeb3BasicParam", "set_Feb3BasicParam.C"));

  //printf ("Histograms created \n");  fflush (stdout);
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
  UInt_t         l_hit_id;
  UInt_t         l_hit_cha_id;
  Long64_t       ll_time;
  Long64_t       ll_trg_time;
  Long64_t       ll_hit_time;
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

  UInt_t         l_trapez_e_found [MAX_SFP][MAX_SLAVE][N_CHA];  
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
    printf ("pl_se_dat: 0x%x, ", pl_se_dat);
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
        //printf ("OUT OF RANGE>> l_cha_id: %d at event %d\n", l_cha_id, l_evt_ct); fflush (stdout);
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
              h_trace[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent(l_l + 1, l_dat_trace);
              h_trapez_fpga[l_sfp_id][l_feb_id][l_cha_id]->SetBinContent(l_l + 1, l_dat_filt);
              l_tr[l_l] = l_dat_trace;
            }
            l_trace_size = l_trace_size >> 1;
          }

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

        // here evaluation of sample ffts:
        DoFFT();

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

//--------------------------------------------------------------------------------------------------------

void TFeb3BasicProc:: f_make_histo (Int_t l_mode)
{
  Text_t chis[256];
  Text_t chead[256];
  UInt_t l_i, l_j, l_k;
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
      }
    }
  } // for sfp
  sprintf(chis,"Traces/FFT/TRACE Sample");
  sprintf(chead,"Trace sample");
  h_sample_trace= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis,"Traces/FFT/TRACE Reference");
  sprintf(chead,"Trace reference");
  h_ref_trace= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis,"Traces/FFT/TRACE Difference");
  sprintf(chead,"Trace difference");
  h_samples_diff= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis,"Traces/FFT/FFT Sample");
  sprintf(chead,"Trace sample FFT");
  h_sample_fft= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis,"Traces/FFT/FFT Reference");
  sprintf(chead,"Trace reference FFT");
  h_ref_fft= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis,"Traces/FFT/FFT Difference");
  sprintf(chead,"Trace FFT difference");
  h_fft_diff= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis,"Traces/FFT/TRACE from FFT Difference");
  sprintf(chead,"Trace retransformed from FFT difference");
  h_sample_trace_retransformed= MakeTH1('D', chis,chead,l_tra_size,0,l_tra_size);

  sprintf(chis, "FFTWindow");
  c_window_fft=MakeWinCond(chis,1024, 3072,h_sample_trace->GetName());


}




void TFeb3BasicProc::DoFFT()
{
  if (fPar->fDoFFT)
  {
    // first provide the sample and reference histograms as defined in parameter:
    // check validity of indices:
    Bool_t valid=kTRUE;
    if(fPar->fSFP_sample<0 || fPar->fSFP_sample>=MAX_SFP || l_sfp_slaves[fPar->fSFP_sample]==0) valid=kFALSE;
    if(fPar->fSFP_ref<0 || fPar->fSFP_ref>=MAX_SFP || l_sfp_slaves[fPar->fSFP_ref]==0) valid=kFALSE;
    if(!valid)
        GO4_STOP_ANALYSIS_MESSAGE("DoFFT: sfp id invalid! sfp_sample=%d, sfp_ref=%d \n", fPar->fSFP_sample, fPar->fSFP_ref);

    if(fPar->fSlave_sample <0 || (uint) fPar->fSlave_sample>=l_sfp_slaves[fPar->fSFP_sample]) valid=kFALSE;
    if(fPar->fSlave_ref <0 || (uint) fPar->fSlave_ref>=l_sfp_slaves[fPar->fSFP_ref]) valid=kFALSE;
    if(!valid)
      GO4_STOP_ANALYSIS_MESSAGE("DoFFT: febex id invalid! slave_sample=%d, slave_ref=%d \n", fPar->fSlave_sample, fPar->fSlave_ref);

    if(fPar->fChannel_sample <0 || fPar->fChannel_sample>=N_CHA) valid=kFALSE;
    if(fPar->fChannel_ref <0 || fPar->fChannel_ref>=N_CHA) valid=kFALSE;
    if(!valid)
      GO4_STOP_ANALYSIS_MESSAGE("DoFFT: channel id invalid! channel_sample=%d, channel_ref=%d \n", fPar->fChannel_sample, fPar->fChannel_ref);


    // copy sample and ref traces to work histograms:


    h_sample_trace->Reset("");
    h_sample_trace->Add( h_trace[fPar->fSFP_sample][fPar->fSlave_sample][fPar->fChannel_sample]);
    h_ref_trace->Reset("");
    h_ref_trace->Add( h_trace[fPar->fSFP_ref][fPar->fSlave_ref][fPar->fChannel_ref]);


    // plain difference of sample and ref traces:
    h_samples_diff->Reset("");
    h_samples_diff->Add(h_sample_trace);
    h_samples_diff->Add(h_ref_trace,-1);


    // try to normalize traces before fft:
//    Double_t integral=0;
//    integral= h_sample_trace->Integral();
//    if(integral!=0) h_sample_trace->Scale(1/integral);
//    integral= h_ref_trace->Integral();
//    if(integral!=0) h_ref_trace->Scale(1/integral);


    TString opt = fPar->fFFTOptions;    // ROOT fft parameters, user defined
//         Available transform types:
//         FFT:
//         - "C2CFORWARD" - a complex input/output discrete Fourier transform (DFT)
//                          in one or more dimensions, -1 in the exponent
//         - "C2CBACKWARD"- a complex input/output discrete Fourier transform (DFT)
//                          in one or more dimensions, +1 in the exponent
//         - "R2C"        - a real-input/complex-output discrete Fourier transform (DFT)
//                          in one or more dimensions,
//         - "C2R"        - inverse transforms to "R2C", taking complex input
//                          (storing the non-redundant half of a logically Hermitian array)
//                          to real output
//         - "R2HC"       - a real-input DFT with output in halfcomplex format,
//                          i.e. real and imaginary parts for a transform of size n stored as
//                          r0, r1, r2, ..., rn/2, i(n+1)/2-1, ..., i2, i1
//         - "HC2R"       - computes the reverse of FFTW_R2HC, above
//         - "DHT"        - computes a discrete Hartley transform

    //1st parameter:
    //  Possible flag_options:
    //  "ES" (from "estimate") - no time in preparing the transform, but probably sub-optimal
    //       performance
    //  "M" (from "measure") - some time spend in finding the optimal way to do the transform
    //  "P" (from "patient") - more time spend in finding the optimal way to do the transform
    //  "EX" (from "exhaustive") - the most optimal way is found
    //  This option should be chosen depending on how many transforms of the same size and
    //  type are going to be done. Planning is only done once, for the first transform of this
    //  size and type.

    //Examples of valid options: "R2C ES ", "C2CF M", "DHT P ", etc.

    Int_t N = h_sample_trace->GetNbinsX();
    Double_t *in = new Double_t[N];
    Double_t *rin = new Double_t[N];
    Double_t *out = new Double_t[N];


    // partial fft from window condition for sample and ref traces:
    h_sample_fft->Reset("");
    h_ref_fft->Reset("");
    Int_t Npart = 0;
    Double_t re=0, im=0;
    TVirtualFFT *thefft;
    for (Int_t ix = 0; ix < N; ++ix)
    {
      if (c_window_fft->Test(ix))
      {
        in[ix] = h_sample_trace->GetBinContent(ix + 1);
        rin[ix] = h_ref_trace->GetBinContent(ix + 1);
//        if(ix<1044)
//          printf("x:%d nsample:%f nref:%f\t",ix, in[ix], rin[ix]);
        Npart++;
      }

    }
//    printf("\n-N=%d, Npart =%d for event %d, fft options=%s\n",N,Npart,l_evt_ct, opt.Data());



    // sample trace:
    DoFilter(in, Npart);
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(in);
    thefft->Transform();
    for (Int_t i = 0; i < Npart; i++)
    {
      thefft->GetPointComplex(i, re, im);
      h_sample_fft->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
//      if(i<20)
//        printf("-Got fft point %d : re=%e im=%e ffthisto=%e samphisto=%e\n",i,re,im, h_sample_fft->GetBinContent(i+1), in[i]);
    }



    // normalize fft spectrum to integral 1:
//    integral= h_sample_fft->Integral();
//    if(integral!=0) h_sample_fft->Scale(1/integral);

    // ref trace:
    DoFilter(rin, Npart);
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(rin);
    thefft->Transform();
    for (Int_t i = 0; i < Npart; i++)
    {
      thefft->GetPointComplex(i, re, im);
      h_ref_fft->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
    }


    // normalize fft spectrum to integral 1:
//    integral= h_ref_fft->Integral();
//    if(integral!=0) h_ref_fft->Scale(1/integral);

      // differences of traces and ffts:

    h_fft_diff->Reset("");
    // plain difference of sample and ref traces:
    h_fft_diff->Reset("");
    h_fft_diff->Add(h_sample_fft);
    h_fft_diff->Add(h_ref_fft,-1);

    // retransformation of difference:
    h_sample_trace_retransformed->Reset("");
    for (Int_t ix = 0; ix < Npart; ++ix)
       {
           out[ix] = h_fft_diff->GetBinContent(ix + 1);
       }
    DoFilter(out, Npart);
    thefft = TVirtualFFT::FFT(1, &Npart, opt.Data());
    thefft->SetPoints(out);
    thefft->Transform();
      for (Int_t i = 0; i < Npart; i++)
      {
        thefft->GetPointComplex(i, re, im);
        h_sample_trace_retransformed->SetBinContent(i + 1, TMath::Sqrt(re * re + im * im));
      }

    delete[] in;
    delete[] rin;
    delete[] out;

  }    // if dofft
}

void TFeb3BasicProc::DoFilter(Double_t* array, Int_t N)
{
  // please compare https://en.wikipedia.org/wiki/Window_function
  if(fPar->fFilterType==TFeb3BasicParam::FIL_NONE) return;
  Double_t factor=0;
  for (Int_t i = 0; i < N; i++)
  {
    // generalized cosine filter:
    factor = fPar->fFilterCoeff[0]; // JAM put this out of sum loop to avoid nasty numerical problems with TMath::Cos ???
    for (Int_t j = 1; j < HitDet_FILTERCOEFFS; ++j)
    {
      factor += fPar->fFilterCoeff[j] * TMath::Cos((double) j * 2.0 * TMath::Pi() * (double) i / (double)(N - 1));
    }    // for j
    array[i] *= factor;
  }    // for i

}








//----------------------------END OF GO4 SOURCE FILE ---------------------
