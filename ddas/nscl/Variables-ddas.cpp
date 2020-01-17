#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

//#include <config.h>
#include <RConfigure.h>
#include <string>
#include <fstream>
#include <iostream>
#include <istream>
#include <iomanip>

#include "Variables-ddas.h"

using namespace std;

bdecayvClockCalibrator::bdecayvClockCalibrator()
{

}
bdecayvClockCalibrator::~bdecayvClockCalibrator()
{

}

void bdecayvClockCalibrator::Initialize()
{
  calib        = 0.1952;
  scale        = 32768.;
  max          = 10.;
  max_daughter = 10;
}

bdecayvPinCalibrator::bdecayvPinCalibrator()
{

}
bdecayvPinCalibrator::~bdecayvPinCalibrator()
{

}

void bdecayvPinCalibrator::Initialize()
{
  slope     = 1.;
  intercept = 0.;
  thresh    = 0;
  uld       = 0;
  uld2      = 0;
}

bdecayvTacCalibrator::bdecayvTacCalibrator()
{

}
bdecayvTacCalibrator::~bdecayvTacCalibrator()
{

}

void bdecayvTacCalibrator::Initialize() 
{
  i2ncorr      = 1.;
  i2ncorrpos   = 0.;
  i2scorr      = 1.;
  i2scorrpos   = 0.;
  pin01corr1    = 1.;
  pin01corr2    = 0.;
  pin01corrpos = 0.;

  i2scorr_offset = 0;
  pin01corr = 0;
  pin01corr_offset = 0;
  pin02corr = 0;
  pin02corr_offset = 0;
}

bdecayvDSSDCalibrator::bdecayvDSSDCalibrator()
{

}
bdecayvDSSDCalibrator::~bdecayvDSSDCalibrator()
{

}

void bdecayvDSSDCalibrator::Initialize()
{
  for(int i = 0; i<41; i++)
    {
      hioffset[i] = 0.;
      hislope[i]  = 1.;
      loslope[i]  = 1.;
    }
}

bdecayvDSSDThresh::bdecayvDSSDThresh()
{
}
bdecayvDSSDThresh::~bdecayvDSSDThresh()
{
}

void bdecayvDSSDThresh::Initialize()
{
  for(int i = 0; i<41; i++)
    {  
      hithresh[i] = 35000.;
      lothresh[i] = 35000.;
      duld[i]     = 0.;
      loduld[i]   = 0.;
    }
}


bdecayvHitFlag::bdecayvHitFlag()
{
}
bdecayvHitFlag::~bdecayvHitFlag()
{
}
void bdecayvHitFlag::Initialize()
{
  pin01   = 0;
  pin02   = 0;
  veto    = 0;
  fronthi = 0;
  frontlo = 0;
  frontloall=0;
  backhi  = 0;
  backlo  = 0;
  backloall=0;
  sun = 0;
}  

bdecayvCorrelation::bdecayvCorrelation()
{
}
bdecayvCorrelation::~bdecayvCorrelation()
{
}
void bdecayvCorrelation::Initialize()
{
  reset      = 0;
  resetclock = 0;
  minimplant = 0.;
}

bdecayvPidCalibrator::bdecayvPidCalibrator()
{
}
bdecayvPidCalibrator::~bdecayvPidCalibrator()
{
}

void bdecayvPidCalibrator::Initialize()
{
  de1_scale       = 1.;
  de1_offset      = 0.;
  de1_tof_corr    = 0.;
  de1_tof_offset  = 0.;
  de2_scale       = 1.;
  de2_offset      = 0.;
  de2_tof_corr    = 0.;
  de2_tof_offset  = 0.;
  isum_scale      = 1.;
  isum_offset     = 0.;
  isum_tof_corr   = 0.;
  isum_tof_offset = 0.;
  imax_scale      = 1.;
  imax_offset     = 0.;
  imax_tof_corr   = 0.;
  imax_tof_offset = 0.;
}

bdecayvSuNCalibrator::bdecayvSuNCalibrator()
{
}
bdecayvSuNCalibrator::~bdecayvSuNCalibrator()
{
}
void bdecayvSuNCalibrator::Initialize()
{
  for(int i=0; i<24;i++)
    {
      pslope[24] = 0;
      pintercept[24] = 0;
    }
  for(int i=0; i<8;i++)
    {
      thresh[i] = 0;
      slope[i] = 0;
      intercept[i] = 0;
      square[i] = 0;
    }
  tot_square = 0;
  tot_slope  = 0;
  tot_intercept = 0;
  
}



betadecayvariables::betadecayvariables()
{

}
betadecayvariables::~betadecayvariables()
{

}
void betadecayvariables::Initialize()
{
  clock.Initialize();
  clockisomer.Initialize();
  pin01.Initialize();
  pin02.Initialize();
  veto.Initialize();
  tac.Initialize();
  front.Initialize();
  back.Initialize();
  fthresh.Initialize();
  bthresh.Initialize();
  hit.Initialize();
  corr.Initialize();
  pid.Initialize();
}

void betadecayvariables::ReadDSSD()
{
  //ifstream calfile("/user/e17009/RootAnalysis/cal/DSSDInit.txt");
  // JAM2020: replace absolute path with relative to our working dir. TODO: pass filename to this method!
  ifstream calfile("cal/DSSDInit.txt");
  int linenum = 0;

  int location = -1;
  double value = 0;
  
  if(!calfile)
    {
      cout << "Unable to open file cal/DSSDInit.txt" << endl;
    }
  else
    {
      //read in cal file until end of file
      while(calfile)
	{
	  if(calfile && (isdigit(calfile.peek())))
	    {
	      linenum++;
	      calfile >> location >> value;
	      
	      if(linenum < 41) front.hioffset[location-1] = value;
	      else if(linenum < 81) back.hioffset[location-1] = value;
	      else if(linenum < 121) front.hislope[location-1] = value;
	      else if(linenum < 161) back.hislope[location-1] = value;
	      else if(linenum < 201) fthresh.hithresh[location-1] = value;
	      else if(linenum < 241) bthresh.hithresh[location-1] = value;
	      else if(linenum < 281) fthresh.duld[location-1] = value;
	      else if(linenum < 321) bthresh.duld[location-1] = value;
	      else if(linenum < 361) fthresh.lothresh[location-1] = value;
	      else if(linenum < 401) bthresh.lothresh[location-1] = value;
	      else if(linenum < 441) fthresh.loduld[location-1] = value;
	      else if(linenum < 481) bthresh.loduld[location-1] = value;
	      else if(linenum < 521) front.loslope[location-1] = value;
	      else if(linenum < 561) back.loslope[location-1] = value;
	    } // line read
	  else
	    {
	      //ignore line
	      calfile.ignore(1000,'\n');
	    }
	} // end while read
    }
  calfile.close();
}

void betadecayvariables::ReadOther()
{
  //ifstream calfile("/user/e17009/RootAnalysis/cal/OtherInit.txt");
  // JAM2020: replace absolute path with relative to our working dir. TODO: pass filename to this method!
    ifstream calfile("cal/OtherInit.txt");
  int linenum = 0;

  double value = 0;
 
  if(!calfile)
    {
      cout << "Unable to open file cal/OtherInit.txt" << endl;
    }
  else
    {
      //read in cal file until end of file
      while(calfile)
	{
	  if(calfile && (isdigit(calfile.peek())))
	    {
	      linenum++;
	      calfile >> value;
	      //cout << "line: " << linenum << " value: " << value << endl;

	      if(linenum == 1) clock.scale = value;
	      if(linenum == 2) clock.calib = value;
	      if(linenum == 3) clock.max = value;
	      if(linenum == 4) corr.minimplant = value;
	      if(linenum == 5) clockisomer.max = value;
	      if(linenum == 6) clockisomer.calib = value;
	      if(linenum == 7) pin01.slope = value;
	      if(linenum == 8) pin01.intercept = value;
	      if(linenum == 9) pin01.thresh = value;
	      if(linenum == 10) pin02.slope = value;
	      if(linenum == 11) pin02.intercept = value;
	      if(linenum == 12) pin02.thresh = value;
	      if(linenum == 13) tac.i2ncorr = value;
	      if(linenum == 14) tac.i2ncorrpos = value;
	      if(linenum == 15) tac.pin01corr1 = value;
	      if(linenum == 16) tac.pin01corr2 = value;
	      if(linenum == 17) tac.i2scorr = value;
	      if(linenum == 18) tac.i2scorr_offset = value;
	      if(linenum == 19) tac.pin01corr = value;
	      if(linenum == 20) tac.pin01corr_offset = value;
	      if(linenum == 21) tac.pin02corr = value;
	      if(linenum == 22) tac.pin02corr_offset = value;
	      if(linenum == 23) veto.thresh = value;
	      if(linenum == 24) veto.uld = value;
	      if(linenum == 25) veto.uld2 = value;
	      
	    } // line read
	  else
	    {
	      //ignore line
	      calfile.ignore(1000,'\n');
	    }
	} // end while read
    }
  calfile.close();
}


void betadecayvariables::ReadSuN()
{
  //ifstream calfile("/user/e17009/RootAnalysis/cal/SuNInit.txt");
  // JAM2020: replace absolute path with relative to our working dir. TODO: pass filename to this method!
  ifstream calfile("cal/SuNInit.txt");
  int linenum = 0;
  
  int location = -1;
  double value = 0;
  
  if(!calfile)
    {
      cout << "Unable to open file cal/SuNInit.txt" << endl;
    }
  else
    {
      //read in cal file until end of file
      while(calfile)
	{
	  if(calfile && (isdigit(calfile.peek())))
	    {
	      linenum++;

	      if(linenum<=80) calfile >> location >> value;
	      else calfile >> value;
	      
	      if(linenum <= 24) SuN.pslope[location] = value;
	      else if(linenum <= 48) SuN.pintercept[location] = value;
	      else if(linenum <= 56) SuN.thresh[location] = value;
	      else if(linenum <= 64) SuN.slope[location] = value;
	      else if(linenum <= 72)
		{
		  SuN.intercept[location] = value;
		  //cout << "sun intercept " << location << " " << SuN.intercept[location] << endl;
		}
	      else if(linenum <= 80) SuN.square[location] = value;
	      
	      else if(linenum == 81)
		{
		  SuN.tot_square = value;
		  //cout << "square " << SuN.tot_square << endl;
		}
	      else if(linenum == 82) SuN.tot_slope = value;
	      else if(linenum == 83) SuN.tot_intercept = value;
	      
      } // line read
	  else
	    {
	      //ignore line
	      calfile.ignore(1000,'\n');
	    }
	} // end while read
      
    }
  calfile.close();
}
