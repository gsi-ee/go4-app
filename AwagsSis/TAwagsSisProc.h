// N.Kurz, EE, GSI, 27-Nov-2014

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
#ifndef TUNPACKPROCESSOR_H
#define TUNPACKPROCESSOR_H

//#define WR_TIME_STAMP     1   // white rabbit time stamp is head of data
#define USE_MBS_PARAM     1   // 

#define MAX_SFP           4
#define MAX_SLAVE        16
#define N_CHA            16

#ifdef WR_TIME_STAMP
#define SUB_SYSTEM_ID      0x100 // sub-system identifier pci express

#define TS__ID_L16         0x0f7
#define TS__ID_M16         0x1f7
#define TS__ID_H16         0x2f7
#endif // WR_TIME_STAMP

#define ADC_RES            2000./16384.   // mV

#ifdef USE_MBS_PARAM
 #define MAX_TRACE_SIZE    8000   // in samples 
 #define MAX_TRAPEZ_N_AVG  1000   // in samples
#else 
 #define     TRACE_SIZE    1024   // in samples 
 #define     TRAPEZ_N_AVG    64   // in samples
 // nr of slaves on SFP 0    1  2  3
 //                     |    |  |  |
 #define NR_SLAVES    { 0,   9, 0, 0} 
 #define ADC_TYPE     { 0, 0xffff, 0, 0} // 12 bit: 0, 14 bit: 1
                                         // bit 0 fuer slave module 0 
                                         // bit n fuer slave module n (max n = 31)
#endif

#define CSA_BASE_START    20
#define CSA_BASE_SIZE    1400
#define CSA_SIGNAL_START 1600
#define CSA_SIGNAL_SIZE  1400

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define BASE_LINE_SUBT_START  0
#define BASE_LINE_SUBT_SIZE   150

#include "TGo4EventProcessor.h"

class TAwagsSisParam;
class TGo4Fitter;

class TAwagsSisProc : public TGo4EventProcessor {
   public:
      TAwagsSisProc() ;
      TAwagsSisProc(const char* name);
      virtual ~TAwagsSisProc() ;
      void f_make_histo (Int_t); 

      Bool_t BuildEvent(TGo4EventElement* target); // event processing function

     /* here put together MBS events to spill traces and trends*/
      void EvaluateSpills();

 private:
      TGo4MbsEvent  *fInput;  //!

      TAwagsSisParam* fPar;


      Bool_t fNewSpill; //!< flag for begin of new spill
      Bool_t fInSpill; //!< flag which is on during spill signals
      Int_t fiEventInSpill; //!< index of MBS event in the spill



      Double_t fDeltaQ[MAX_SFP][MAX_SLAVE][N_CHA]; //!< relative charge difference ("sig minus back" value)


      TH1          *h_spill_scaler; //!< count spills and events
      TH1          *h_signal_to_background_ave; //!<average of all channels, monitor spill on criterium

      TH1          *h_trace        [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_trace_blr    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_trapez_fpga  [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_fpga_e       [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_peak         [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_valley       [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_trgti_hitti  [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_ch_hitpat    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_hitpat       [MAX_SFP][MAX_SLAVE];         //!
      TH1          *h_ch_hitpat_tr [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_hitpat_tr    [MAX_SFP][MAX_SLAVE];         //!
      TH1          *h_adc_spect    [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      
      
      TH1          *h_q_spill        [MAX_SFP][MAX_SLAVE][N_CHA];  //!< delta Q trend during spill, vs MBS event index
      TH1          *h_q_spill_sum    [MAX_SFP][MAX_SLAVE][N_CHA];  //!< delta Q trend during spill, vs MBS event index, accum
      TH1          *h_trace_stitched [MAX_SFP][MAX_SLAVE][N_CHA];  //!< stitched traces for each spill
      TH1          *h_trace_stitched_sum [MAX_SFP][MAX_SLAVE][N_CHA];  //!< stitched traces for each spill, accumulated


        /** defines the region for the background baseline evaluation - here before awags readout reset*/
	  TGo4WinCond* fxBackgroundRegion;

	  /** defines the region for the signal baseline evaluation - here after awags reset */
	  TGo4WinCond* fxSignalRegion;
      
       /** average height of trace contents in Window fxBackgroundRegion*/
	  TH1          *h_background_height [MAX_SFP][MAX_SLAVE][N_CHA];


	  /** average height of trace contents in Window fxSignalRegion*/
      TH1          *h_signal_height [MAX_SFP][MAX_SLAVE][N_CHA ];

      /** ratios of average contents between fxSignalRegion/fxBackgroundRegion*/
      TH1          *h_signal_to_background [MAX_SFP][MAX_SLAVE][N_CHA ];

      /** difference between average contents of fxSignalRegion and fxBackgroundRegion*/
      TH1          *h_signal_minus_background[MAX_SFP][MAX_SLAVE][N_CHA ];
      
      
      ClassDef(TAwagsSisProc,1)
};
#endif //TUNPACKPROCESSOR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
