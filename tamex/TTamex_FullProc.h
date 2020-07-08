
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

#define STATISTIC 200000

#define DUMP_BAD_EVENT 1 fTimeDiff[i][j]

#define COARSE_CT_RANGE  0x800  // 11 bits

#define MAX_SSY        2                // maximum number of sub-systems (readout pcs in nxm system)
#define MAX_SFP        4 
#define MAX_TAM        8                // maximum febex/tamex per sfp
#define MAX_CHA_INPUT 33                // A) maximum physical input channels per module. must be modulo 4
#define MAX_CHA       MAX_CHA_INPUT * 1 // B) leading egdes + trailing edges + qtc trailing edges
//#define MAX_CHA_INPUT  32                // A) maximum physical input channels per module. must be modulo 4
//#define MAX_CHA        MAX_CHA_INPUT * 2 // B) leading egdes + trailing edges + qtc trailing edges

                             // it seems that only "leading" edge bit is set for 0-47 channels
                             // therefore "MAX_CHA_INPUT 48" and only "MAX_CHA_INPUT * 1"   
                             // this has changed to previous version and comments A) and B) are wrong
                             // so called 17th channel should would appear according to chahit as
                             // channel nr 48, therefore 49 channels in total

#define N_DEEP_AN      4                // deep analysis for first N_DEEP_AN channels specified below.
                                        // must be even nr.
//#define MAX_CHA_AN    64                // total nr. of channels analyzed. must be modulo 4
#define MAX_HITS       1                // max. number of hits per channel accepted
//#define MAX_HITS      10                // max. number of hits per channel accepted

// select 16 channels to be analyzed
// test index:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
//               |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
//#define SSY_ID { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } 
#define SSY_ID { 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 }
//#define SSY_ID { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 } 

#define SFP_ID { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } 
//#define SFP_ID { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
//#define SFP_ID { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 } 
//#define SFP_ID { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 }

#define TAM_ID { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } 
//#define TAM_ID { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } 
//#define TAM_ID { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 } 
//#define TAM_ID { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } 
//#define TAM_ID { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 } 

//#define CHA_ID { 0, 1, 2, 3, 4, 5, 6, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32 }
#define CHA_ID { 1, 2, 3, 4, 5, 6, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,0 }
//#define CHA_ID { 1, 2, 3, 4, 9,11,13,15,17,19,21,23,25,27,29,31, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30,32, 0,33,34,35,18,50,19,51,20,52,21,53,22,54,23,55,24,56,25,57,26,58,27,59,28,60,29,61,30,62,31,63 }
//#define CHA_ID { 1,17, 3,19, 5,21, 7,23, 9,25,11,27,13,29,15,31, 2,18, 4,20, 6,22, 8,24,10,26,12,28,14,30,16,32, 0,33,34,35,18,50,19,51,20,52,21,53,22,54,23,55,24,56,25,57,26,58,27,59,28,60,29,61,30,62,31,63 }
//#define CHA_ID { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,32 } 
//#define CHA_ID { 1, 1, 3, 3, 5, 5, 7, 7, 9, 9,11,11,13,13,15,15,17,17,19,19,21,21,23,23,25,25,27,27,29,29,31,31, 2, 2, 4, 4, 6, 6, 8, 8,10,10,12,12,14,14,16,16,18,18,20,20,22,22,24,24,26,26,28,28,30,30,32,32 }
//#define CHA_ID { 1, 1, 3, 3, 5, 5, 7, 7, 9, 9,11,11,13,13,15,15,17,17,19,19,21,21,23,23,25,25,27,27,29,29,31,31, 1, 1, 3, 3, 5, 5, 7, 7, 9, 9,11,11,13,13,15,15,17,17,19,19,21,21,23,23,25,25,27,27,29,29,31,31 }
//#define CHA_ID { 1, 2, 3, 4, 5,11, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32, 0,33,34,35,18,50,19,51,20,52,21,53,22,54,23,55,24,56,25,57,26,58,27,59,28,60,29,61,30,62,31,63 }
//#define CHA_ID {10,12,14,16,22,24,20,15,17,19,21,23,25,27,29,31, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30,32, 0,33,34,35,18,50,19,51,20,52,21,53,22,54,23,55,24,56,25,57,26,58,27,59,28,60,29,61,30,62,31,63 }

#define N_CAL_EVT               (ULong64_t) 100000
#define N_PHY_TREND_PRINT       (ULong64_t) 1000000

#define N_DELTA_T   400000
//#define N_DELTA_T   100000
#define N_BIN       100000
#define N_TIM       10000

#define N_TR_BINS   100000  
#define N_COARSE    30
#define CYCLE_TIME    (Double_t) 5000

//#define TRIG_WIN_SIZE      200     // in clock cycles 
#define HITPAT_CT_RANGE    10

#define N_BIN_T    600
#define RESET_VAL -100000

#include "TGo4EventProcessor.h"

#include "TTamex_FullEvent.h"

class TTamex_FullParam;
class TTamex_FullEvent;
class TGo4Fitter;

class TTamex_FullProc : public TGo4EventProcessor {
   public:
      TTamex_FullProc() ;
      TTamex_FullProc(const char* name);
      virtual ~TTamex_FullProc() ;

      Bool_t BuildEvent(TGo4EventElement* target); // event processing function

 private:
      TGo4MbsEvent  *fInput; //!
      TTamex_FullEvent* fOutput; //!

      TTamex_FullParam* fPar;

      TH1   *h_box[MAX_SSY][MAX_SFP][MAX_TAM][MAX_CHA];  // box histogram in SFP id / TAMEX id / CHANNEL nr coordinates

      TH1   *h_err_box[MAX_SSY][MAX_SFP][MAX_TAM][MAX_CHA];  // box histogram in SFP id / TAMEX id / CHANNEL nr coordinates
 
      TH1   *h_tim[MAX_CHA_AN];                          // box histogram in test channel coordinates

      TH1   *h_sum[MAX_CHA_AN];                          // sum histogram in test channel coordinates

      TH2   *h_raw_tim_corr[N_DEEP_AN>>1];               // raw time correlatian ch1-ch0, ch3-ch2, ...

      TH1   *h_cal_tim_diff[MAX_CHA_AN][MAX_CHA_AN];     // calibrated channel time differences

      TH1   *h_cal_tim_diff_wic[MAX_CHA_AN][MAX_CHA_AN]; // calibrated chan. time diff. 
      TH1   *h_cal_tim_diff_woc[MAX_CHA_AN][MAX_CHA_AN]; // with a (wic) and without (woc)
                                                         // clock 
      TH1   *h_coarse_diff[MAX_CHA_AN][MAX_CHA_AN];      // coarse ctr differences

      TH1   *h_hitpat[MAX_CHA_AN];                       // test channel hit pattern

      TH1   *h_coarse[MAX_CHA_AN];                       // coarse ctr distribution

      TH1   *h_cal_tim_diff_te[MAX_CHA_AN][MAX_CHA_AN]; // calibrated channel time differences

      TH1   *h_cal_tim_diff_tr_av[MAX_CHA_AN][MAX_CHA_AN]; //

      TH1   *h_cal_tim_diff_tr_rms[MAX_CHA_AN][MAX_CHA_AN]; //

      TH2   *h_7_5_vs_11_9; 

      TGo4Picture      *fPicture;

   ClassDef(TTamex_FullProc,1)
};

static  UInt_t l_err_catch = 0;
static  UInt_t l_prev_err_catch = 0;
static  UInt_t l_err_ssy [MAX_CHA];
static  UInt_t l_err_sfp [MAX_CHA];
static  UInt_t l_err_tam [MAX_CHA];
static  UInt_t l_err_cha [MAX_CHA];
static  UInt_t l_prev_err_ssy [MAX_CHA];
static  UInt_t l_prev_err_sfp [MAX_CHA];
static  UInt_t l_prev_err_tam [MAX_CHA];
static  UInt_t l_prev_err_cha [MAX_CHA];
static  UInt_t l_num_err;
static  UInt_t l_prev_num_err;

#endif //TUNPACKPROCESSOR_H


//----------------------------END OF GO4 SOURCE FILE ---------------------
