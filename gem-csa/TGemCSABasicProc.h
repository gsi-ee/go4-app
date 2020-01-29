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

/**
 * Analysis for the GEM CSA tests in December 2019
 * Added mapping to original unpacker step based on TFeb3BasicProc
 * v.01 on 10-Dec-2019 by JAM (j.adamczewski@gsi.de)
 *
 *
 * */

#ifndef TUNPACKPROCESSOR_H
#define TUNPACKPROCESSOR_H

//#define WR_TIME_STAMP     1   // white rabbit time stamp is head of data
#define USE_MBS_PARAM     1   // 

// defined in event structure:
//#define MAX_SFP           4
//#define MAX_SLAVE        16
//#define N_CHA            16

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

#define CSA_BASE_START    520
#define CSA_BASE_SIZE    100
#define CSA_SIGNAL_START 600
#define CSA_SIGNAL_SIZE   50

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define BASE_LINE_SUBT_START  0
#define BASE_LINE_SUBT_SIZE   150


/** here  one may tune number of polygon fit parameters
 * = order +1 JAM2020*/
#define GEMCSA_FITPOLYPARS 2

/** range for fit polynom parameter a0 histograming*/
#define GEMCSA_RANGE_A0 10000

/** range for fit polynom parameter a1 histograming*/
#define GEMCSA_RANGE_A1 0.5

/** number of bins for fit parameter histograms*/
#define GEMCSA_FITPARBINS 1000


#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include "TGemCSABasicEvent.h"

class TGemCSABasicParam;
class TGo4Fitter;

class TGemCSABasicProc : public TGo4EventProcessor {
   public:
      TGemCSABasicProc() ;
      TGemCSABasicProc(const char* name);
      virtual ~TGemCSABasicProc() ;
      void f_make_histo (Int_t); 

      Bool_t BuildEvent(TGo4EventElement* target); // event processing function

      /** perform the linear baseline fit for the raw trace of given sfp, slave, channel
       * returns false if fit could not be done*/
      Bool_t DoTraceFit(UInt_t sfp, UInt_t slave, UInt_t chan);

 private:
      TGo4MbsEvent  *fInput;  //!

      TGemCSABasicParam* fParam;

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
      TH1          *h_csa_base     [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_csa_signal   [MAX_SFP][MAX_SLAVE][N_CHA];  //!
      TH1          *h_csa_sum_sig;
      TH1          *h_peak_ref_sig;
      TH2          *h_peak_ref__sum_csa;
      TH2          *h_peak_ref__sum_csa2;
	  TH1          *h_csa_pad_e;


	  /** JAM Jan-2020: following objects are for the linear baseline shape fitter: */

	  /** we use a single fitter object that is assigned to data*/
	//  TGo4Fitter *fxFitter;

	  /** defines the region for the baseline fit*/
	  TGo4WinCond* fxFitRegion;


	  /** will contain fit result curves: */
	  TH1          *h_trace_fitresult        [MAX_SFP][MAX_SLAVE][N_CHA];  //!


	  /** histograms the polygon parameters of each channel fit*/
	  TH1          *h_fit_par        [MAX_SFP][MAX_SLAVE][N_CHA][GEMCSA_FITPOLYPARS]; //!
      

	  /** histograms the chi2 of each channel fit*/
	  TH1          *h_fit_chi2        [MAX_SFP][MAX_SLAVE][N_CHA]; //!


	  /** histograms the polygon parameters of all channel fits*/
	  TH1          *h_fit_a_all[GEMCSA_FITPOLYPARS]; //!







      ClassDef(TGemCSABasicProc,1)
};
#endif //TUNPACKPROCESSOR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
