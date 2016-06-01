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

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define BASE_LINE_SUBT_START  0
#define BASE_LINE_SUBT_SIZE   150

/* JAM this define switches on baseline substraction with user defined condition region*/
#define USE_BASELINE_CONDITION 1




#include "TGo4EventProcessor.h"

//class TFeb3BasicParam;
#include "TFeb3BasicParam.h"
//class TGo4Fitter;

class TGo4Condition;

class TFeb3BasicProc : public TGo4EventProcessor {
   public:
      TFeb3BasicProc() ;
      TFeb3BasicProc(const char* name);
      virtual ~TFeb3BasicProc() ;
      void f_make_histo (Int_t); 
      void FillGrids();

      Bool_t BuildEvent(TGo4EventElement* target); // event processing function

 private:
      TGo4MbsEvent  *fInput;  //!

      TFeb3BasicParam *fPar; //!

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

      // JAM here some test histograms for Sven Loechners APFEL grid test:

      TH1          *h_grid_x_profile[PEXOR_APFEL_GRIDS]; //! grid lateral x profile
      TH1          *h_grid_y_profile[PEXOR_APFEL_GRIDS]; //! grid lateral y profile
      TH2          *h_grid_xvstrace[PEXOR_APFEL_GRIDS]; //! grid x versus trace time
      TH2          *h_grid_yvstrace[PEXOR_APFEL_GRIDS]; //! grid y versus trace time
      TH1          *h_grid_x_profile_sum[PEXOR_APFEL_GRIDS]; //! grid lateral x profile accumulated
      TH1          *h_grid_y_profile_sum[PEXOR_APFEL_GRIDS]; //! grid lateral y profile accumulated
      TH2          *h_grid_xvstrace_sum[PEXOR_APFEL_GRIDS]; //! grid x versus trace time accumulated
      TH2          *h_grid_yvstrace_sum[PEXOR_APFEL_GRIDS]; //! grid y versus trace time accumulated


      TGo4Condition* c_baseline_region; //! dynamically adjust region where baseline is evaluated

      UInt_t fEventSequenceNumber; //! event number since begin



   ClassDef(TFeb3BasicProc,2)
};
#endif //TUNPACKPROCESSOR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
