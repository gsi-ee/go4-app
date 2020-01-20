/*****************************************************************/
/* Correlator.cpp for DSSD Beta Decay - Digital Data Acquisition */
/* Based on standard BCS SpecTcl codes                           */
/* Heather Crawford/Sean Liddick, June 2010                      */
/*****************************************************************/

/********************************************/
/* Correlation flags:                       */
/*                                          */
/*   4 - invalid DSSD implant pixel         */
/*   8 - invalid DSSD decay pixel           */
/*  12 - implantation event                 */
/*  16 - decay event                        */
/*  20 - unknown event                      */
/*  24 - front, no back decay               */
/*  28 - back, no front decay               */
/*  32 - back-2-back implantations          */
/*  36 - back-2-back decays                 */
/*  40 - invalid implant time               */
/*  44 - invalid decay time                 */
/*  48 - exceeded correlation time          */
/*  52 - long-lived implant rejection       */
/*  56 - reset correlation array            */
/*  72 - no implant                         */
/*                                          */
/********************************************/

//#include <config.h>
#include <RConfigure.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "Parameters-ddas.h"
#include "Variables-ddas.h"
#include "Correlator.h"
//#include "Gates.C"
#include "TRandom.h"
#include <TROOT.h>
#define CORRELATION 3

Correlator::Correlator()
{
  cout << "starting correlator" << endl;
  
  rndmflag = 0;
  gatename = "";
}

Correlator::~Correlator()
{  
}

int Correlator::SelectGate(string gatename)
{
  int retval = 1;
  
  gate = (TCutG*)gROOT->FindObject(gatename.c_str());
  
  if(!gate)
    {
      retval = 0; //failed to get a gate
    }
  else 
    {
      retval = 1;
    }
  
  havegate = retval;
  
  return retval;
}

void Correlator::SetRandomFlag(int val, betadecayvariables &bdecayv)
{
  rndmflag = val;
  if(rndmflag == 0) bdecayv.clock.calib = 1;
  if(rndmflag == 1) bdecayv.clock.calib = -1;
}

void Correlator::Reset()
{
  for(int i=1; i<17; i++) 
    {
      for(int j=1; j<17; j++) 
	{
	  implant[i][j].implanted = false;
	  implant[i][j].numcorr = 0;
	  implant[i][j].time = 0;
	  implant[i][j].dE1 = 0;
	  implant[i][j].dE2 = 0;
	  implant[i][j].rawde1 = 0;
	  implant[i][j].rawde2 = 0;
	  implant[i][j].i2pos = 0;
	  implant[i][j].isum = 0;
	  implant[i][j].imax = 0;
	  implant[i][j].tof1 = 0;
	  implant[i][j].tof2 = 0;
	  implant[i][j].rawtof1 = 0;
	  implant[i][j].rawtof2 = 0;
	  implant[i][j].dt = 0;

	  decay[i][j].time = 0;
	  decay[i][j].dE1 = 0;
	  decay[i][j].dE2 = 0;
	  decay[i][j].rawde1 = 0;
	  decay[i][j].rawde2 = 0;
	  decay[i][j].i2pos = 0;
	  decay[i][j].isum = 0;
	  decay[i][j].imax = 0;
	  decay[i][j].tof1 = 0;
	  decay[i][j].tof2 = 0;
	  decay[i][j].rawtof1 = 0;
	  decay[i][j].rawtof2 = 0;
	  decay[i][j].dt = 0;
	  decay[i][j].decaydt = 0;
	}
    }
}

double Correlator::Correlate(betadecay &bdecay, betadecayvariables &bdecayv, int nstripx = 16, int nstripy = 16, int subseg = -1)
{
  //MakeCuts();
  
  int lostripx = 0;
  int histripx = 0 + nstripx -1;
  int lostripy = 0;
  int histripy = 0 + nstripy -1;
  
  double outval = -1.;
  int returnval = 0;
  
  int i, j;
  int numcorr = -1;
  int condition = 0;
  int resetcheck = 0;
  int ibackch = 100;
  int ifrontch = 100;
  int dbackch = 100;
  int dfrontch = 100;
  
  int ifrontmult = 0;
  int ibackmult = 0;

  bool implant_event = false;
  bool implant_tof = false;
  bool decay_event = false;
  bool sun_only = false;
  bool unknown_event = false;
  bool fhitdc_reject = false;
  bool decayenergy = false;
  
  bool implanted = false;
  
  if(bdecayv.corr.reset != 0) 
    {
      cout << "reseting correlator array " << endl;
      Correlator::Reset();
      resetcheck = 56; /* Condition flag will be set to 56 when reset done */
      bdecayv.corr.reset = 0;
    }
  
  /*** Implant/Decay Logic ***/
  /*
    Pixel location is defined by the front and back strips
    showing the maximum energy deposited (in implant or decay)
    in each side of the DSSD.  The strip array runs from 1 to 40.  
    If there is no event data, imaxch and dmaxch are set to 100;
    there are channel checks to reject events with invalid positions.
  */
  
  /* First establish pixel locations for implants/decays */
  /* Implants first... */
  if(subseg == 1)
    {
      ifrontch = (int)(/*2. */ bdecay.front.imaxch);  
      ibackch  = (int)(/*2. */ bdecay.back.imaxch);
    }
  else
    {
      ifrontch = (int)bdecay.front.imaxch;  
      ibackch  = (int)bdecay.back.imaxch;
    }
  
  /* Now decays... */
  if(subseg == 1)
    {
      dfrontch = (int)(/*2. */ bdecay.front.dmaxch);
      dbackch  = (int)(/*2. */ bdecay.back.dmaxch);
    }
  else
    {
      dfrontch = (int)bdecay.front.dmaxch;
      dbackch  = (int)bdecay.back.dmaxch;
    }
  
  // Now establish boolean results of simple requirements to determine
  // implants vs. decays vs. oddball events
  
  /* Low-gain events in PIN1, PIN2, DSSD - implant */
  
  if ((bdecayv.hit.pin01 == 1) && (bdecayv.hit.backlo != 0) && (bdecayv.hit.frontlo != 0) && (bdecayv.hit.veto==0)) 
    {
      if (bdecay.pin01.energy > 11800 && bdecay.pin01.energy < 12800){
	implant_event = true;
	returnval += 2;
        }
    }
  else 
    {
      //cout << "implant event false ";
    }
  
  // JAM 1-2020: enabled the following again, was commented out in found code:
   if(havegate)
     {
       if(gate->IsInside(bdecay.pid.pin01i2n,bdecay.pid.de1))
   	{
   	  implant_event = true;
   	}
       else
   	{
   	  implant_event = false;
   	}
     }
  
  /* High-gain events with no PIN1, PIN2 signals - decays */
  if ((bdecayv.hit.pin01 == 0) && (bdecayv.hit.pin02 == 0) && (bdecayv.hit.fronthi != 0) && (bdecayv.hit.backhi != 0) && (bdecayv.hit.veto==0)) 
    {
      //cout << "decay!" << endl;
      decay_event = true;
      returnval += 16;
    }
  
  /* Construct 48-bit current time */
  double current_time;
  current_time = 0;

  // time in ms
  current_time = bdecay.clock.current;
  //cout << "currenttime: " << bdecay.clock.current << endl;
  
  if(implant_event) 
    {
      /* Check for valid implantation pixel in DSSD */
      if((ibackch >= lostripx) && (ibackch <= histripx) && (ifrontch >= lostripy) && (ifrontch <= histripy)) 
	{
	  condition = 12; /* Good implantation */
	  returnval += 1024;
	  
	  /* Check for existing implant in DSSD pixel */
	  if (implant[ibackch][ifrontch].implanted) 
	    {
	      condition = 32; /* Back-2-back implantation in DSSD */
	    } 
	  else 
	    {
	      implant[ibackch][ifrontch].implanted = true;
	      //cout << "implanted: "<< ibackch << " " << ifrontch  << endl;
	    }

	  //changed snl, time between implants should always be calculated
	  double implant_time;
	  implant_time = 0;
	  
	  /* Get full clock time stamp of last implantation */
	  implant_time = (implant[ibackch][ifrontch].time);	  
	  //  cout << "implant time: " << implant_time << " current time: " << current_time << " fast: " << bdecay.clock.fast << " slow: " << bdecay.clock.slow << endl;
	  if(implant_time > current_time) 
	    {
	      condition = 40; //bad implant time
	    }
	  double deltaT;
	  double deltaT_neg;
	  deltaT_neg = (current_time - implant_time);
	  //deltaT = abs(deltaT_neg)*bdecayv.clock.calib;
	  deltaT = TMath::Abs(deltaT_neg);
	  //deltaT = deltaT_neg; //for checking possible wrong times
	  
	  //cout << "before: " << deltaT_neg << " after: " << deltaT << endl;
	  
	  if(deltaT < 0)
	    {
	      //cout << "reset" << endl;
	      cout << "correlator reset" << " current time: " << current_time << " implant time: " << implant_time << endl;
	      Correlator::Reset();
	    }
	  
	  implant[ibackch][ifrontch].dt = deltaT; /* 1ms per bin */
	  
	  /* Update time and tof -- updated for e17009*/    
	  implant[ibackch][ifrontch].time = current_time/*bdecay.clock.current*/;
	  implant[ibackch][ifrontch].tof1 = bdecay.tac.pin01xfp; //TOF between xfp and PIN01 
	  implant[ibackch][ifrontch].tof2 = bdecay.tac.pin02xfp; //TOF between xfp and PIN02 
	  implant[ibackch][ifrontch].rawtof1 = bdecay.tac.pin01xfp; //Raw TOF between xfp and PIN01 (used for gates)
	  implant[ibackch][ifrontch].rawtof2 = bdecay.tac.pin02xfp; //Raw TOF between xfp and PIN02 (used for gates)
	  
	  /* Update dE from PINs, etc. */
	  implant[ibackch][ifrontch].dE1 = bdecay.pid.de1; //corrected Pin1 energy
	  implant[ibackch][ifrontch].dE2 = bdecay.pid.de2; //corrected Pin2 energy
	  implant[ibackch][ifrontch].rawde1 = bdecay.pin01.ecal;
	  implant[ibackch][ifrontch].rawde2 = bdecay.pin02.ecal;
	  implant[ibackch][ifrontch].i2pos = bdecay.tac.i2pos;
	  implant[ibackch][ifrontch].isum = bdecay.pid.isum;
	  implant[ibackch][ifrontch].imax = bdecay.pid.imax;
	  
	  /* Reset counter */
	  implant[ibackch][ifrontch].numcorr = 0;  
	  
	  bdecay.corr.dtimplant = implant[ibackch][ifrontch].dt;
	  bdecay.corr.itime = implant[ibackch][ifrontch].time;
	  bdecay.corr.ide1  = (long)implant[ibackch][ifrontch].dE1;
	  bdecay.corr.ide2  = (long)implant[ibackch][ifrontch].dE2;
	  bdecay.corr.irawde1 = (long)implant[ibackch][ifrontch].rawde1;
	  bdecay.corr.irawde2 = (long)implant[ibackch][ifrontch].rawde2;
	  bdecay.corr.iisum = (long)implant[ibackch][ifrontch].isum;
	  bdecay.corr.iimax = (long)implant[ibackch][ifrontch].imax;
	  bdecay.corr.itof1  = (long)implant[ibackch][ifrontch].tof1;
	  bdecay.corr.itof2  = (long)implant[ibackch][ifrontch].tof2;
	  bdecay.corr.ii2pos = (long)implant[ibackch][ifrontch].i2pos;
	  bdecay.corr.irawtof1 = (long)implant[ibackch][ifrontch].rawtof1;
	  bdecay.corr.irawtof2 = (long)implant[ibackch][ifrontch].rawtof2;
	  //bdecay.corr.ide1_PID = implant[ibackch][ifrontch].dE1;
	} 
      else 
	{
	  condition = 4; /* Invalid DSSD implantation pixel */
	}
      
    } 
  else if(decay_event) 
    {
      //cout << "loop1" << endl;
      returnval += 2048;
      /* Code will search over user-defined correlation first to 
	 find decay event in DSSD */
      
      int num_corr = 0;
    
     // search over the nearest pixels to find the most recent implant
     // and the correlate with it
      int corr_limit = (CORRELATION - 1)/2;
      int flolimit = dfrontch - corr_limit;
      int fhilimit = dfrontch + corr_limit;
      int blolimit = dbackch - corr_limit;
      int bhilimit = dbackch + corr_limit;

      //added 9.21.18 BL
      //if decay occurs on end strip, need to restrict correlation field 
      if(flolimit < 0) flolimit = 0;
      if(fhilimit > 15) fhilimit = 15;
      if(blolimit < 0) blolimit = 0;
      if(bhilimit > 15) bhilimit = 15;

      //cout << "corr limit: " << corr_limit << " front limits: " << flolimit << "-" << fhilimit << "    back limits: " << blolimit << "-" << bhilimit << endl; 
      
      int front = 500;
      int back = 500;
      
      double tdiffold = 100000000000;
      
      for(int k = flolimit; k < fhilimit+1; k++)
	{
	  for(int l = blolimit; l < bhilimit+1; l++)
	    {
	      if((k >= lostripy) && (k<=histripy) && (l>=lostripx) && (l<=histripx))
		{
		  //cout << "loop2" << endl;
		  if(implant[l][k].implanted)
		    {
		      //cout << "loop2" << endl;
		      //set implanted flag
		      implanted = true;
		      // determine time between now and previous implant
		      //implant time
		      double temp_implanttime = (implant[l][k].time);
		      double tdiff_neg = (current_time-temp_implanttime);
		      double tdiff = TMath::Abs(tdiff_neg);
		      //double tdiff = tdiff_neg; //for checking neg time differences
		      //cout << "current: " << current_time << " implant: " << temp_implanttime << " tdiff: " << tdiff << " tdiffold: " << tdiffold << endl;
		      if((tdiff < tdiffold) && (tdiff > 0))
			{ // take the most recent implant
			  //cout << "most recent" << endl;
			  num_corr++;         //number of implants in field
			  tdiffold = tdiff;
			  front = k;
			  back = l;
			  if(dfrontch == front) bdecay.corr.field = 1;
			  else bdecay.corr.field = 3;
			  if(dbackch == back) bdecay.corr.field = 1;
			  else bdecay.corr.field = 3;
			}
		    }
		}
	    }
	}

      double mintime = 1000000000;
      
      if(implanted) 
	{
	  //cout << "loop3 " << front << " " << back << endl;
	  bdecay.corr.mult = (long)num_corr;

	  bdecay.corr.minimplant_time = mintime;
	  
	  //set decay locations
	  bdecay.corr.dfrontch = dfrontch;
	  bdecay.corr.dbackch  = dbackch;
	  bdecay.corr.frontch  = front;
	  bdecay.corr.backch   = back;
	  
	  /* Check for valid decay (really correlated implant) pixel in DSSD */
	  if((back >= lostripx) && (back <= histripx) && (front >= lostripy) && (front <= histripy)) 
	    {
	      //cout << "decayflag" << endl;
	      condition = 16; /* Good decay flag */

	      //added 9.21.18 BL
	      //need to compare time difference between ions over whole correlation field, not just between ions in the same pixel (generalize so it doesn't matter what the correlation field is)
	      //if correlation field is 3 then you can essentially bypass using min implant time, and that's not good
	      //if(front ==0 || back == 0) cout << "front: " << front << " back: " << back << endl;
	      tdiffold = 100000000000;
	      
	      for(int k = flolimit; k < fhilimit+1; k++)
		{
		  for(int l = blolimit; l < bhilimit+1; l++)
		    {
		      if(k != front && l != back) //check pixels in corr field that are not the same pixel as correlated ion
			{
			  double temp_implanttime = (implant[back][front].time);
			  //if(front ==0 || back == 0) cout << "temp: " << temp_implanttime << " pixel: " << k << " " << l << endl;
			  double tdiff_neg = (temp_implanttime - implant[l][k].time);
			  
			  //cout << "new loop" << " temp: " << temp_implanttime << " tdiff: " << tdiff_neg << endl;
			  double tdiff = TMath::Abs(tdiff_neg);
			  if((tdiff < tdiffold) && (tdiff > 0))
			    {
			      tdiffold = tdiff;
			      mintime = tdiff;
			      //cout << mintime << endl;
			    }
			}
		    }
		}
	      
	      if(implant[back][front].dt < mintime) mintime = implant[back][front].dt; //check same pixel as correlated ion
	      //cout << mintime << endl;
	      if(implant[back][front].implanted) 
		{
		  //cout << "loop4" << endl;
	  	  double implant_time;
		  implant_time = 0;
		  
		  /* Figure out the implantation time */
		  implant_time = (implant[back][front].time);
		  
		  /* Determine the last decay time in this pixel */
		  double decay_time;
		  decay_time = (decay[back][front].time);
		  
		  if(implant_time > current_time) 
		    {
		      condition = 44;
		    }
		  
		  double deltaT;
		  double deltaT_neg;
		  deltaT_neg = current_time - implant_time;
		  deltaT = TMath::Abs(deltaT_neg); /* in 1ms bins */	  
		  
		  /* Make sure time between back-to-back implants is long; 
		     if not, reject event */
		  //use min time between any ions in correlation field, not just between ions in same pixel
		  //BL 9.21.18
		  //if(((implant[back][front].dt) >= bdecayv.corr.minimplant*bdecayv.clock.scale) || (implant[back][front].dt == 0)) 
		  
		  if(((mintime) >= bdecayv.corr.minimplant*bdecayv.clock.scale) || (implant[back][front].dt == 0)) 
		    {
		      //cout << "loop5" << endl;
		      /* Do not process correlations that extend beyond a fixed time */	    
		      if((deltaT < bdecayv.clock.max) && (deltaT >= 0)) 
			{
			  //cout << "loop6" << endl;
			  decay[back][front].time = bdecay.clock.current;
			  decay[back][front].dt   = deltaT;
			  decay[back][front].dE1  = implant[back][front].dE1; //corrected Pin1
			  decay[back][front].dE2  = implant[back][front].dE2; //corrected Pin2
			  decay[back][front].rawde1 = implant[back][front].rawde1;
			  decay[back][front].rawde2 = implant[back][front].rawde2;
			  decay[back][front].i2pos = implant[back][front].i2pos;
			  decay[back][front].isum = implant[back][front].isum;
			  decay[back][front].imax = implant[back][front].imax;
			  decay[back][front].tof1  = implant[back][front].tof1;
			  decay[back][front].tof2  = implant[back][front].tof2;
			  decay[back][front].rawtof1 = implant[back][front].rawtof1;
			  decay[back][front].rawtof2 = implant[back][front].rawtof2;
			  outval = deltaT; //decaydt;
			  implant[back][front].numcorr++;
			  decay[back][front].dnumcorr /*numcorr*/ = implant[back][front].numcorr;
	        
			  /* Output decay time in 10 ms units on 10-bit spectrum
			     with 16-bit parameter declaration */
			  bdecay.corr.dtimecal = decay[back][front].dt; //*bdecayv.clock.calib;
			  bdecay.corr.dtime    = decay[back][front].time;
			  bdecay.corr.dde1     = (long)decay[back][front].dE1; //corrected Pin1
			  bdecay.corr.dde2     = (long)decay[back][front].dE2; //corrected Pin2
			  bdecay.corr.disum    = (long)decay[back][front].isum;
			  bdecay.corr.dimax    = (long)decay[back][front].imax;
			  bdecay.corr.dtof1     = (long)decay[back][front].tof1;
			  bdecay.corr.dtof2     = (long)decay[back][front].tof2;
			  bdecay.corr.dnumcorr = (int)decay[back][front].dnumcorr;
			  bdecay.corr.di2pos = (long)decay[back][front].i2pos;
			  bdecay.corr.drawtof1 = (long)decay[back][front].rawtof1;
			  bdecay.corr.drawtof2 = (long)decay[back][front].rawtof2;
			  bdecay.corr.drawde1 = (long)decay[back][front].rawde1;
			  bdecay.corr.drawde2 = (long)decay[back][front].rawde2;

			  if(bdecay.corr.dtimecal>0 && outval<0) cout << "oh no... " << bdecay.corr.dtimecal << " " << outval << " " << current_time << " " << implant_time << endl;
			} 
		      else 
			{
			  condition = 48; /* Correlation time too long */
			}
		    } 
		  else 
		    {
		      condition = 52; /* Time between implants too short */
		    }
	  	} 
	      else 
		{
		  condition = 36; /* Back to back decay */
		}
	    } 
	  else 
	    {
	      condition = 8; /* Invalid decay pixel */
	    }
	} 
      else 
	{
	  condition = 72; /* No implant found */
	}
    } 
  else 
    {
      unknown_event = true;
      
      if(condition == 0) 
	{
	  condition = 20; /* Unknown event */
	  returnval += 4096;
      
	  if((bdecayv.hit.fronthi != 0) && (bdecayv.hit.backhi == 0)) 
	    {
	      condition = 24; /* DSSD front, no back */
	    }
	  if((bdecayv.hit.fronthi == 0) && (bdecayv.hit.backhi != 0)) 
	    {
	      condition = 28; /* DSSD back, no front */
	    }
	}
    }
    
  //end main correlator
 
  if(resetcheck != 0) 
    {
      condition = resetcheck; /* Reset array flag */
    }
  
  bdecay.corr.flag = (long) condition;
  
  return outval;
}













