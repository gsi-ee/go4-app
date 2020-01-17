/*      File: Parameters.h
	Parameter definitions for the beta-decay setup
	Author: Colin Morton
	Date: September 2001
	Altered to C++ class structure from C struct structure
	July09 by HLC
*/

#ifndef __BDECAYPARAMETERS_H
#define __BDECAYPARAMETERS_H

#include "TObject.h"
#include "TMath.h"
#include <cstring>


using namespace std;

// Bit registers


class bdecayBit : public TObject
{
 private:

 public:
  int calhitmask;
  int corrmask;
  int eventstart;

 public:
  bdecayBit() {};
  ~bdecayBit() {};
  void Reset();

  ClassDef(bdecayBit,1);
};

/* Clock */
class bdecayClock: public TObject
{
 private:

 public:
  double  time;
  double  current;
  double  initial;

 public: 
  bdecayClock() {};
  ~bdecayClock() {};
  void Reset();

  ClassDef(bdecayClock,1);
};

/* trace capture */
class bdecayTrace : public TObject
{
 private:

 public:
  vector<UShort_t> trace;
  vector<Double_t> traceresult;

 public:
  bdecayTrace() {};
  ~bdecayTrace() {};
  void Reset();
  int ProcessTrace(int crateid, int slotid, int chanid);
  
  ClassDef(bdecayTrace,1);
};

/* TACs */
class bdecayTac: public TObject
{
 private:
  
 public:
  int  pin01rf;
  int  pin01xfp;
  int  pin02xfp;
  int  pin01i2n;
  int  pin01i2s;
  int  pin02i2n;
  double pin02i2ntime;
  double  pin01xfptime;
  double  pin02xfptime;
  double i2postime;
  int  i2pos;
  int  i2postdc;
  int  i2ntdc;
  int  i2stdc;
  int  i2ncorr;
  int  i2scorr;
  int  i2poscorr;
  int pin01i2nmultiset;
  int pin01i2smultiset;
  int pin02i2nmultiset;
  int  pin01xfpmultiset;
  int  pin02xfpmultiset;
  int i2posmultiset;
  double i2pos_cfdtime;
  double calc_i2pos;
  double calc_i2pos_mixed;
  double pin01i2n_ns;
  double pin01i2s_ns;
  double pin02i2n_ns;
  double i2pos_ns;
  double tof1_corr;
  double tof2_corr;

 public:
  bdecayTac() {};
  ~bdecayTac() {};
  void Reset();

  ClassDef(bdecayTac,1);
};

/* ADCs */
class bdecayAdc: public TObject
{
 private:
  
 public:
  int channel[16];
  bdecayTrace chantrace[16];

 public:
  bdecayAdc() {};
  ~bdecayAdc() {};
  void Reset();
  
  ClassDef(bdecayAdc,1);
};

/* TDCs */
class bdecayTdc: public TObject
{
 private:

 public:
  double time[16];

 public:
  bdecayTdc() {};
  ~bdecayTdc() {};
  void Reset();

  ClassDef(bdecayTdc,1);
};

/* TIMES */
class bdecayTime: public TObject
{
 private:

 public:
  double timelow[16];
  double timehigh[16];
  double timefull[16];
  double timecfd[16];
  
 public:
  bdecayTime() {};
  ~bdecayTime() {};
  void Reset();

  ClassDef(bdecayTime,1);
};

/* PIN */
class bdecayPin: public TObject
{
 private:
  
 public:
  int energy;
  int ecorr;
  double time;
  int ecal;
  int eventtdc;
  int multiset;
  
 public:
  bdecayPin() {};
  ~bdecayPin() {};
  void Reset();

  ClassDef(bdecayPin,1);
};

/* DSSD */
class bdecayDssd: public TObject
{
 private:
  
 public:
  int hienergy[17];
  int hiecal[17];
  int vhiecal[17];
  double hitime[17];
  int  hieventtdc[17];
  int loenergy[17];
  int loecal[17];
  int vloecal[17];
  double lotime[17];
  int loeventtdc[17];
  int himultiset[16];
  int lomultiset[16];
  
  int  icent;
  int  dcent;
  int  imax;
  double imaxtime;
  int  dmax;
  double dmaxtime;
  int imaxch;
  double vimaxch;
  int dmaxch;
  double vdmaxch;
  int imult;
  int dmult;
  int isum;
  int dsum;
  
  //bdecayTrace tracehi[17];
  //bdecayTrace tracelo[17];

 public:
  bdecayDssd() {};
  ~bdecayDssd() {};
  void Reset();

  ClassDef(bdecayDssd,1);
};

/* class bdecayDssdout : public TObject */
/* { */
/*  public: */
/*   int  icent; */
/*   int  dcent; */
/*   int  imax; */
/*   double imaxtime; */
/*   int  dmax; */
/*   double dmaxtime; */
/*   int imaxch; */
/*   double vimaxch; */
/*   int dmaxch; */
/*   double vdmaxch; */
/*   int imult; */
/*   int dmult; */
/*   int isum; */
/*   int dsum; */

/*   int hienergy[17]; */
/*   int loenergy[17]; */
/*   int hiecal[17]; */
/*   int loecal[17]; */
  
/*   /\* int himultiset[16]; *\/ */
/*   /\* int lomultiset[16]; *\/ */

/*   //bdecayTrace tracehi[17]; */
/*   //bdecayTrace tracelo[17]; */

/*   bdecayDssdout() {}; */
/*   ~bdecayDssdout() {}; */
/*   void Reset(); */

/*   ClassDef(bdecayDssdout,1); */
/* }; */

/* Total */
class bdecayTotal: public TObject
{
 private:
  
 public:
  int dsum;
  int dmax;
  int isum;
  int imax;
  
 public:
  bdecayTotal() {};
  ~bdecayTotal() {};
  void Reset();

  ClassDef(bdecayTotal,1);
};

/* PID */
class bdecayPid: public TObject
{
 private:
  
 public:
  int de1;
  int de2;
  int pin01i2n;
  int pin01i2s;
  int pin02i2n;
  int isum;
  int imax;
  
 public:
  bdecayPid() {};
  ~bdecayPid() {};
  void Reset();

  ClassDef(bdecayPid,1);
};

/* Correlator */
class bdecayCorr: public TObject
{
 private:
  
 public:
  double dtimplant;
  double itime;
  double minimplant_time;
  int ide1;
  int ide2;
  // int ide3;
  int irawde1; //raw Pin1 dE 
  int irawde2; // raw Pin2 dE
  int ii2pos; // i2pos of implant
  int iisum;
  int iimax;
  int itof1; //corrected Pin1-I2N TOF of implant
  int itof2; //corrected Pin2-I2N TOF of implant
  int irawtof1; //raw TOF Pin1-I2N 
  int irawtof2; //raw TOF Pin2-I2N
  /* double gtimecal; */
  /* double gtime; */
  /* int gde1; */
  /* int gde2; */
  /* int gde3; */
  /* int gisum; */
  /* int gimax; */
  /* int gtof; */
  double dtimecal;
  double dtime;
  int dfrontch;
  int dbackch;
  int frontch;
  int backch;
  int dde1; //corrected Pin1 dE of implant correlated to decay
  int dde2; //corrected Pin2 dE of implant correlated to decay
  //int dde3;
  int drawde1; //raw Pin1 dE
  int drawde2; //raw Pin2 dE
  int di2pos; //i2 pos of implant correlated to decay
  int disum;
  int dimax;
  int dtof1; //corrected Pin1-I2N TOF of implant correlated to decay
  int dtof2; //corrected Pin2-I2N TOF of implant correlated to decay
  int drawtof1; //raw Pin1-I2N TOF of implant correlated to decay
  int drawtof2; //raw Pin2-I2N TOF of implant correlated to decay
  int flag;
  int mult;
  int dmult;
  int dmax;
  int dsum;
  int dnumcorr; //temp SNL
  int field;
  

  int dtof_PID; //corrected Pin2-I2N TOF that falls within gates (to be used for decay PID)
  int dde1_PID; //corrected Pin1 dE that falls within gates (to be used for decay PID)
  
  /* double gdtimecal; */
  /* double gdtime; */
  /* int gdde1; */
  /* int gdde2; */
  /* int gdde3; */
  /* int gdisum; */
  /* int gdimax; */
  /* int gdtof; */
  /* double gdfronthiecal[17]; */
  /* double gdbackhiecal[17]; */

 public:
  bdecayCorr() {};
  ~bdecayCorr() {};
  void Reset();

  ClassDef(bdecayCorr,2);
};

/* SuN */
class bdecaySuN: public TObject
{
 private:
  
 public:
  double rpmt[24];
  double tpmt[24];
  double gmpmt[24];
  double rseg[8];
  double cseg[8];
  
  double totalmult;
  double total;
  double total_cal;
  double midseg_total;
  double midseg_total_cal;

  
 public:
  bdecaySuN() {};
  ~bdecaySuN() {};
  void Reset();

  ClassDef(bdecaySuN,1);
};

/* DDAS diagnostics */
class bdecayDdasDiagnostics: public TObject
{
 private:

 public:
  int cmult;
  int eventlength;
  int tdiffevent;
  int adchit[20];
  int overflow[16];
  int finishcode[16];

  
 public:
  bdecayDdasDiagnostics() {};
  ~bdecayDdasDiagnostics() {};
  void Reset();
  
  ClassDef(bdecayDdasDiagnostics,1);
};

/* The entire beta-decay set-up */
class betadecay: public TObject
{
 public:
  bdecayBit bit;
  bdecayClock clock;
  bdecayTac tac;
  bdecayAdc adc[21];
  bdecayTdc tdc[21];
  bdecayTime time[21];
  bdecayPin pin01;
  bdecayPin pin02;
  bdecayPin veto;
  bdecayDssd front;
  bdecayDssd back;
  bdecayTotal total;
  bdecayPid pid;
  bdecayCorr corr;
  bdecaySuN sun;
  bdecayDdasDiagnostics ddasdiagnostics;

 public:
  betadecay();
  ~betadecay();
  void Reset();

  ClassDef(betadecay,1);
};

class rootdataout : public TObject
{
 public:
  bdecayClock clock;
  bdecayTac tac;
  bdecayPin pin01;
  bdecayPin pin02;
  bdecayPin veto;
  /* bdecayDssdout front; */
  /* bdecayDssdout back; */
  bdecayDssd front;
  bdecayDssd back;
  bdecayPid pid;
  bdecayCorr corr;
  bdecaySuN sun;
  bdecayDdasDiagnostics ddasdiagnostics;
 public:
  rootdataout();
  ~rootdataout();
  void Reset();

  ClassDef(rootdataout,1);
};
#endif
