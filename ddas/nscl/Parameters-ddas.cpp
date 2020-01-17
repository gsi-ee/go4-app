/*
  File:  bdecayP.h
  This file creates the parameter tree structure and must be
  edited in parallel with bdecayParameters_standalone.h
  mod 1Sep05 by JPC for 03034 and 05028
*/
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>
#include <istream>
#include <iomanip>

//#ifdef HAVE_STD_NAMESPACE
using namespace std;
//#endif

#include <RConfigure.h>
#include <string>

#include "Parameters-ddas.h"

//#if !defined(__CINT__)
ClassImp(bdecayBit);
ClassImp(bdecayClock);
ClassImp(bdecayTrace);
ClassImp(bdecayTac);
ClassImp(bdecayAdc);
ClassImp(bdecayTdc);
ClassImp(bdecayTime);
ClassImp(bdecayPin);
ClassImp(bdecayDssd);
//ClassImp(bdecayDssdout);
ClassImp(bdecayTotal);
ClassImp(bdecayPid);
ClassImp(bdecayCorr);
ClassImp(bdecaySuN);
ClassImp(bdecayDdasDiagnostics);
ClassImp(betadecay);
ClassImp(rootdataout);
//#endif

betadecay::betadecay()
{
  //cout << "new  betadecay class " << endl;

  // bit.Reset();
  // clock.Reset();
  // tac.Reset();
  // for(int i=0;i<21;i++){
  //   adc[i].Reset();
  //   tdc[i].Reset();
  //   time[i].Reset();
  // }
  // pin01.Reset();
  // pin02.Reset();
  // pin03.Reset();
  // pin04.Reset();
  // front.Reset();
  // back.Reset();
  // sssd01.Reset();
  // sssd02.Reset();
  // sssd03.Reset();
  // sssd04.Reset();
  // sssd05.Reset();
  // sssd06.Reset();
  // total.Reset();
  // pid.Reset();
  // corr.Reset();
  // sega.Reset();
  // segatotal.Reset();
  // ddasdiagnostics.Reset();
}

betadecay::~betadecay(){

}

rootdataout::rootdataout(){

}
rootdataout::~rootdataout(){

}

void betadecay::Reset()
{
  bit.Reset();
  clock.Reset();
  tac.Reset();
  for(int i=0; i<21; i++)
    {
      adc[i].Reset();
      tdc[i].Reset();
      time[i].Reset();
    }
  pin01.Reset();
  pin02.Reset();
  veto.Reset();
  front.Reset();
  back.Reset();
  total.Reset();
  pid.Reset();
  corr.Reset();
  sun.Reset();
  ddasdiagnostics.Reset();
}

void rootdataout::Reset()
{
  clock.Reset();
  tac.Reset();
  pin01.Reset();
  pin02.Reset();
  veto.Reset();
  front.Reset();
  back.Reset();
  pid.Reset();
  corr.Reset();
  sun.Reset();
  ddasdiagnostics.Reset();
}

void bdecayBit::Reset()
{
  calhitmask = -1;
  corrmask = -1;;
  eventstart = -1;
}

void bdecayClock::Reset()
{
  current = 0;
  initial = 0;
}

void bdecayTrace::Reset()
{
  trace.clear();
  traceresult.clear();
}

int bdecayTrace::ProcessTrace(int crateid, int slotid, int chanid)
{
  int ov, un;
  ov = un = 0;
  
  double max;
  max = 0;
  
  double result;
  result = 0;
  
  double baseline = 0;
  double energy = 0;
  
  Int_t blo, bhi;
  blo = 100;
  bhi = 250;
  
  Int_t elo, ehi;
  elo = 350;
  ehi = 500;
  
  if(slotid>=2 && slotid<=5)
    {
      for(int i=0; i<trace.size();i++)
	{
	  if(trace[i] > 4090) ov = 1;
	  if(trace[i] < 5) un = 1;
	  
	  if(trace[i] > max) max = trace[i];
	  
	  if( (i>blo) && (i<bhi) )
	    {
	      baseline += trace[i];
	    }
	  if( (i>elo) && (i<ehi) )
	    {
	      energy += trace[i];
	    }
	}
      
      double std;
      // determine scatter around baseline
      for(int i=blo; i<bhi; i++)
	{
	  std = (trace[i] - (baseline/(bhi-blo)))*(trace[i] - (baseline/(bhi-blo)));
	}
      std = TMath::Sqrt(std/(bhi-blo));
      
      energy = energy - baseline;
      energy = energy / 20.;
      
      max = max - (baseline/(bhi-blo));
      
      traceresult.push_back(energy);
      traceresult.push_back(ov);
      traceresult.push_back(un);
      traceresult.push_back(std);
      traceresult.push_back(max);
      
      if(ov) result = -1;
      if(un) result = -2;
      
    }
}

void bdecayTac::Reset()
{
  pin01rf = -1;
  pin01xfp = -1;
  pin01xfptime = -1;
  pin01i2n = -1;
  pin01i2s = -1;
  pin02i2n = -1;
  pin02i2ntime = -1;
  pin02xfp = -1;
  pin02xfptime = -1;
  i2pos = -1;
  i2postdc = -1;
  i2ntdc = -1;
  i2stdc = -1;
  i2ncorr = -1;
  i2scorr = -1;
  i2poscorr = -1;
  pin01i2nmultiset = 0;
  pin01i2smultiset = 0;
  pin02i2nmultiset = 0;
  pin01xfpmultiset = 0;
  pin02xfpmultiset = 0;
  i2posmultiset = 0;
  i2pos_cfdtime = 0;
  i2postime = 0;
  calc_i2pos = -1;
  calc_i2pos_mixed = -1;
  pin01i2n_ns = -1;
  pin01i2s_ns = -1;
  pin02i2n_ns = -1;
  i2pos_ns = -1;
  tof1_corr = 0;
  tof2_corr = 0;
}

void bdecayAdc::Reset()
{
  memset(channel,-1,16*sizeof(int));
  for(int i=0; i<16; i++)
    {
      chantrace[i].Reset();
    }
}

void bdecayTdc::Reset()
{
  memset(time,0,16*sizeof(double));
}

void bdecayTime::Reset()
{
  memset(timelow, 0, 16*sizeof(double));
  memset(timehigh, 0, 16*sizeof(double));
  memset(timefull, 0, 16*sizeof(double));
  memset(timecfd, 0, 16*sizeof(double));
}

void bdecayPin::Reset()
{
  energy = -1;
  ecorr = -1;
  time = 0;
  ecal = -1;
  eventtdc = -1;
  multiset = 0;
}

void bdecayDssd::Reset()
{
  memset(hienergy,-1,17*sizeof(int));
  memset(hiecal, -1,17*sizeof(int));
  memset(vhiecal, -1,17*sizeof(int));
  memset(hitime,0,17*sizeof(double));
  memset(hieventtdc, -1,17*sizeof(int));
  memset(loenergy, -1,17*sizeof(int));
  memset(loecal, -1,17*sizeof(int));
  memset(vloecal, -1,17*sizeof(int));
  memset(lotime,0,17*sizeof(double));
  memset(loeventtdc, -1,17*sizeof(int));
  memset(himultiset,0,16*sizeof(int));
  memset(lomultiset,0,16*sizeof(int));

  icent = -1;
  dcent = -1;
  imax = 0;
  dmax = 0;
  imaxch = -1;
  vimaxch= -1;
  dmaxch = -1;
  vdmaxch= -1;
  imult = 0;
  dmult = 0;
  isum = -1;
  dsum = -1;
  imaxtime = 0;
  dmaxtime = 0;
}

// void bdecayDssdout::Reset(){

//   for(int i=0; i<17;i++){
//     hienergy[i] = 0;
//     loenergy[i] = 0;
//     hiecal[i] = 0;
//     loecal[i] = 0;
//   }
  
//   icent = -1;
//   dcent = -1;
//   imax = -1;
//   dmax = -1;
//   imaxch = -1;
//   vimaxch= -1;
//   dmaxch = -1;
//   vdmaxch= -1;
//   imult = 0;
//   dmult = 0;
//   isum = -1;
//   dsum = -1;
//   imaxtime = -1;
//   dmaxtime = -1;
// }


void bdecayTotal::Reset()
{
  dsum = -1;
  dmax = -1;
  isum = -1;
  imax = -1;
}

void bdecayPid::Reset()
{
  de1 = -1;
  de2 = -1;
  pin01i2n = -1;
  pin01i2s = -1;
  pin02i2n = -1;
  isum = -1;
  imax = -1;
}

void bdecayCorr::Reset()
{
  dtimplant = -1;
  itime = -1;
  minimplant_time = -1;
  ide1 = -1;
  ide2 = -1;
  // ide3 = -1;
  irawde1 = -1;
  irawde2 = -1;
  ii2pos = -1;
  iisum = -1;
  iimax = -1;
  itof1 = -1;
  itof2 = -1;
  irawtof1 = -1;
  irawtof2 = -1;
  // gtimecal = -1;
  // gtime = -1;
  // gde1 = -1;
  // gde2 = -1;
  // gde3 = -1;
  // gisum = -1;
  // gimax = -1;
  // gtof = -1;
  dtimecal = -1;
  dtime = -1;
  dfrontch = -1;
  dbackch = -1;
  frontch = -1;
  backch = -1;
  dde1 = -1;
  dde2 = -1;
  //dde3 = -1;
  drawde1 = -1;
  drawde2 = -1;
  di2pos = -1;
  disum = -1;
  dimax = -1;
  dtof1 = -1;
  dtof2 = -1;
  drawtof1 = -1;
  drawtof2 = -1;
  flag = -1;
  mult = 0;
  dmult = 0;
  dmax = -1;
  dsum = -1;
  dnumcorr = 0;
  field = 0;

  dtof_PID = 0;
  dde1_PID = 0;

  // gdtimecal = -1;
  // gdtime = -1;
  // gdde1 = -1;
  // gdde2 = -1;
  // gdde3 = -1;
  // gdisum = -1;
  // gdimax = -1;
  // gdtof = -1;

  // for(int z=0;z<17;z++)
  //   {
  //     gdfronthiecal[z] = gdbackhiecal[z] = 0;
  //   }
}

void bdecaySuN::Reset()
{
  memset(rpmt,0,24*sizeof(double));
  memset(tpmt,0,24*sizeof(double));
  memset(gmpmt, 0,24*sizeof(double));
  memset(rseg,0,8*sizeof(double));
  memset(cseg, 0,8*sizeof(double));

  totalmult = 0;
  total = 0;
  total_cal =0;
  midseg_total = 0;
  midseg_total_cal = 0;
}

void bdecayDdasDiagnostics::Reset()
{
  cmult = 0;
  eventlength = -1;
  tdiffevent = -1;
  memset(adchit, -1,20*sizeof(int));
  memset(overflow,0,16*sizeof(int));
  memset(finishcode,0,16*sizeof(int));
}
