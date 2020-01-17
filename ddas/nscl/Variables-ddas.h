#ifndef __BDECAYVARIABLES_H
#define __BDECAYVARIABLES_H
/** JAM 16-Jan-2020: added include guard here*/

/*      File:  Variables.h
    	Variable definitions for the beta-decay setup
*/


// Clock Calibrator

class bdecayvClockCalibrator
{  
 public:
  double calib;
  double scale;
  double max;
  double max_daughter;
  
 public:
  bdecayvClockCalibrator();
  ~bdecayvClockCalibrator();
  void Initialize();
};

// Pin Calibrator

class bdecayvPinCalibrator 
{
 public:
  double slope;
  double intercept;
  double thresh;
  double uld;
  double uld2;

 public:
  bdecayvPinCalibrator();
  ~bdecayvPinCalibrator();
  void Initialize();
};

// TAC Calibrator

class bdecayvTacCalibrator
{
 public:
  double i2ncorr;
  double i2ncorrpos;
  double i2scorr;
  double i2scorrpos;
  double pin01corr1;
  double pin01corr2;
  double pin01corrpos;

  double i2scorr_offset;
  double pin01corr;
  double pin01corr_offset;
  double pin02corr;
  double pin02corr_offset;

 public:
  bdecayvTacCalibrator();
  ~bdecayvTacCalibrator();
  void Initialize();
};

// DSSD Calibrator

class bdecayvDSSDCalibrator
{
 public:
  double hioffset[41];
  double hislope[41];
  double loslope[41];

 public:
  bdecayvDSSDCalibrator();
  ~bdecayvDSSDCalibrator();
  void Initialize();
};

// DSSD Threshold

class bdecayvDSSDThresh
{
 public:
  double hithresh[41];
  double lothresh[41];
  double loduld[41];
  double duld[41];

 public:
  bdecayvDSSDThresh();
  ~bdecayvDSSDThresh();
  void Initialize();
};

// Hit flags

class bdecayvHitFlag 
{
 public:
  int pin01;
  int pin02;
  int veto;
  int fronthi;
  int frontlo;
  int frontloall;
  int backhi;
  int backlo;
  int backloall;
  int sun;

 public:
  bdecayvHitFlag();
  ~bdecayvHitFlag();
  void Initialize();
};

// Correlation

class bdecayvCorrelation 
{
 public:
  int reset;
  int resetclock; // Reset 50MHz clock
  double minimplant;

 public:
  bdecayvCorrelation();
  ~bdecayvCorrelation();
  void Initialize();
};

// PID

class bdecayvPidCalibrator
{
 public:
  double de1_scale;
  double de1_offset;
  double de1_tof_corr;
  double de1_tof_offset;
  double de2_scale;
  double de2_offset;
  double de2_tof_corr;
  double de2_tof_offset;
  double isum_scale;
  double isum_offset;
  double isum_tof_corr;
  double isum_tof_offset;
  double imax_scale;
  double imax_offset;
  double imax_tof_corr;
  double imax_tof_offset;

 public:
  bdecayvPidCalibrator();
  ~bdecayvPidCalibrator();
  void Initialize();
};

// PID

// SuN

class bdecayvSuNCalibrator
{
 public:
  double pslope[24];
  double pintercept[24];
  double thresh[8];
  double slope[8];
  double intercept[8];
  double square[8];
  double tot_square;
  double tot_slope;
  double tot_intercept;

 public:
  bdecayvSuNCalibrator();
  ~bdecayvSuNCalibrator();
  void Initialize();
};




/* The entire beta decay setup */
class betadecayvariables
{
 public:
  bdecayvClockCalibrator clock;
  bdecayvClockCalibrator clockisomer;
  bdecayvPinCalibrator pin01;
  bdecayvPinCalibrator pin02;
  bdecayvPinCalibrator veto;
  bdecayvTacCalibrator tac;
  bdecayvDSSDCalibrator front;
  bdecayvDSSDCalibrator back;
  bdecayvDSSDThresh fthresh;
  bdecayvDSSDThresh bthresh;
  bdecayvHitFlag hit;
  bdecayvCorrelation corr;
  bdecayvPidCalibrator pid;
  bdecayvSuNCalibrator SuN;

 public:
  betadecayvariables();
  ~betadecayvariables();

  void Initialize();
  void ReadDSSD();
  void ReadOther();
  void ReadSuN();
};

#endif
