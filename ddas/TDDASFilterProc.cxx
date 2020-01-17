//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TDDASFilterProc.h"

#include "TClass.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TMath.h"

#include "TGo4Log.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"


#include "TDDASRawEvent.h"
#include "TDDASFilterParameter.h"

 TRandom3 TDDASFilterProc::fRandom3;


//***********************************************************
TDDASFilterProc::TDDASFilterProc() :
    TGo4EventProcessor(), fPar(0)
{
}
//***********************************************************
TDDASFilterProc::TDDASFilterProc(const char* name) :
    TGo4EventProcessor(name), fPar(0)
{
  TGo4Log::Info("TDDASFilterProc: Create %s", name);

  //// init user analysis objects:
  fPar = dynamic_cast<TDDASFilterParameter*>(MakeParameter("DDASFilterParameter", "TDDASFilterParameter", "set_DDASFilterParameter.C"));


  fBdecay.Reset();
  fBdecayVars.Initialize();
  fBdecayVars.ReadDSSD();

  fBdecayVars.ReadOther();
  fBdecayVars.ReadSuN();



  ///////////////////////////////////////////////////////////////////////////////77
  // now define some monitoring histograms
  TString obname;
  TString obtitle;
  TString foldername;
  TString binlabel;

  obname.Form("Raw/EnergyMap");
  obtitle.Form("Channel vs ADC energy overview");

  hEnergyMap= MakeTH2('I', obname.Data(), obtitle.Data(), GO4_DDAS_CHANNELS, 0, GO4_DDAS_CHANNELS, GO4_DDAS_ADCS, 0, GO4_DDAS_ADCS
        ,"channel", "ADC id", "energy");

  for(Int_t adc=0; adc<GO4_DDAS_ADCS; ++adc)
     {



    for(Int_t ch=0; ch<GO4_DDAS_CHANNELS; ++ch)
            {
              obname.Form("Raw/ADC%d/Time/Time_%d_%d", adc, adc,ch);
              obtitle.Form("Raw Channel Time ADC %d channel %d", adc,ch );
              hTime[adc][ch] = MakeTH1('D', obname.Data(), obtitle.Data(), GO4_DDAS_TIMERANGE, 0, GO4_DDAS_TIMERANGE, "time", "counts");

              obname.Form("Raw/ADC%d/Energy/Energy_%d_%d",adc,adc,ch);
              obtitle.Form("Raw Channel Energy ADC %d channel %d ", adc, ch);
              hEnergy[adc][ch] = MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "energy", "counts");

               obname.Form("Raw/ADC%d/Traces/Trace_%d_%d", adc, adc, ch);
               obtitle.Form("Raw Trace ADC %d Channel %d", adc, ch);
               hTrace[adc][ch]= MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_TRACELEN, 0, GO4_DDAS_TRACELEN, "time (bins)", "adc counts");
            }

     }


  for(Int_t strip=0; strip<GO4_DDAS_DSSD_STRIPS; ++strip)
       {
          obname.Form("Mapped/DSSDFront/High/DSSD_Front_Hi_%d", strip);
          obtitle.Form("DSSD Front hi gain strip %d", strip);
          hDSSD_Front_Higain_Energy[strip]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "channels", "energy");

          obname.Form("Mapped/DSSDFront/Low/DSSD_Front_Lo_%d", strip);
          obtitle.Form("DSSD Front lo gain strip %d", strip);
          hDSSD_Front_Logain_Energy[strip]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "channels", "energy");


          obname.Form("Mapped/DSSDBack/High/DSSD_Back_Hi_%d", strip);
          obtitle.Form("DSSD Back hi gain strip %d", strip);
          hDSSD_Back_Higain_Energy[strip]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "channels", "energy");

          obname.Form("Mapped/DSSDBack/Low/DSSD_Back_Lo_%d", strip);
          obtitle.Form("DSSD Back lo gain strip %d", strip);
          hDSSD_Back_Logain_Energy[strip]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "channels", "energy");
       }

  for(Int_t npmt=0; npmt<GO4_DDAS_GAMMA_NPMTS; ++npmt)
         {
            obname.Form("Mapped/PMT/Intensity/SuN_Intensity_%d", npmt);
            obtitle.Form("Gamma pmt intensity npmt %d", npmt);
            hGamma_SuN_Intens[npmt]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "channels", "intensity");

            obname.Form("Mapped/PMT/Energy/SuN_Energy_%d", npmt);
            obtitle.Form("Gamma pmt energy npmt %d", npmt);
            hGamma_SuN_Energy[npmt]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_ERANGE, 0, GO4_DDAS_ERANGE, "channels", "energy");

            obname.Form("Mapped/PMT/Time/SuN_Time_%d", npmt);
            obtitle.Form("Gamma pmt time npmt %d", npmt);
            hGamma_SuN_Time[npmt]=MakeTH1('I', obname.Data(), obtitle.Data(), GO4_DDAS_TIMERANGE, 0, GO4_DDAS_TIMERANGE, "channels", "time");

         }


}
//***********************************************************
TDDASFilterProc::~TDDASFilterProc()
{

}
//***********************************************************


//-----------------------------------------------------------

Bool_t TDDASFilterProc::BuildEvent(TGo4EventElement* dest)
{
  fInput = dynamic_cast<TDDASRawEvent*>(GetInputEvent());
  fOutput = dynamic_cast<TDDASRawEvent*>(dest);

  if ((fInput == 0) || (fOutput == 0))
  {
    TGo4Log::Error("DDASFilterProc: events are not specified!");
    return kFALSE;
  }
  fOutput->SetValid(kFALSE); // supress everything that will not go through filter
  // fill poutevt here:
  DdasToEvent();

  return kTRUE;
}

void TDDASFilterProc::DdasToEvent()
{
  std::vector<ddaschannel*>& channellist = fInput->fDDASEvent.GetData();
  std::vector<ddaschannel*>::iterator channellist_it;



  double starttime=0, endtime=0;
  double oldtime = 0;



  fBdecay.Reset();
  fBdecayVars.hit.Initialize();





  int eventsize = channellist.size();

  fBdecay.ddasdiagnostics.cmult = eventsize;

  channellist_it = channellist.begin();

  for (int i = 1; i <= 18; i++)
  {
    fBdecay.ddasdiagnostics.adchit[i] = 0;
  }

  for (channellist_it = channellist.begin(); channellist_it < channellist.end(); channellist_it++)
  {
    int crateid = (*channellist_it)->crateid;
    int slotid = (*channellist_it)->slotid;
    int channum = (*channellist_it)->chanid;

    /* The time of an event will be taken as the time of the first
     channel in the event. */

    if (channellist_it == channellist.begin())
    {
      /* Calculate time difference between events. */
      double tdiffevent = ((*channellist_it)->GetTime()) - starttime;
      tdiffevent = tdiffevent / 1000.; /* Microseconds time difference. */
      fBdecay.ddasdiagnostics.tdiffevent = tdiffevent;

      starttime = (*channellist_it)->GetTime();

      fBdecay.clock.current = ((*channellist_it)->GetTime()) / 1000000.;
      // cout << "fBdecay.clock.current: " << fBdecay.clock.current << endl;

      if (fBdecay.clock.current > oldtime)
      {
        oldtime = fBdecay.clock.current;
      }
      else
      {
        cout << "times out of order" << (*channellist_it)->slotid << " " << (*channellist_it)->chanid << " "
            << setprecision(12) << (*channellist_it)->GetTime() << " " << setprecision(12) << oldtime << endl;
        oldtime = starttime;
      }
    }

    /* Unpack data from crate 3 */
    if (crateid == 3)
    {
      unsigned int adcnumber = slotid - 1;

      fBdecay.ddasdiagnostics.adchit[adcnumber] += pow(2., (double) (*channellist_it)->chanid);
      fBdecay.adc[adcnumber].channel[channum] = ((*channellist_it)->GetEnergy());
      fBdecay.time[adcnumber].timefull[channum] = ((*channellist_it)->GetTime());

      if ((adcnumber < GO4_DDAS_ADCS) && (channum < GO4_DDAS_CHANNELS))
        {
          hEnergyMap->Fill(channum, adcnumber, (*channellist_it)->GetEnergy());
          hEnergy[adcnumber][channum]->Fill((*channellist_it)->GetEnergy());
          hTime[adcnumber][channum]->Fill((*channellist_it)->GetTime() - starttime);
        }
      else
      {
          std::cout<<"DdasToEvent WARNING: indices from data out of array range: adc:"<<adcnumber<<" , channel:"<<channum << std::endl;
      }


      //check for trace and exract
      if ((*channellist_it)->tracelength != 0)
      {
        fBdecay.adc[adcnumber].chantrace[channum].trace = (*channellist_it)->GetTrace();

        // JAM here copy trace into histogram bins:
        for(int i=0; i<(*channellist_it)->tracelength; ++i)
        {
          if(i < hTrace[adcnumber][channum]->GetNbinsX())
            hTrace[adcnumber][channum]->SetBinContent(i+1,fBdecay.adc[adcnumber].chantrace[channum].trace[i]);
        }

      }
    }

    /* Map the channel, calibrate and theshold check. */
    MapChannels(crateid, slotid, channum, fBdecay, fBdecayVars);

    /** do some raw histogramming of strips here:*/

    // some histogrammming of mapped things JAM2020:
    for(int strip=0; strip<GO4_DDAS_DSSD_STRIPS; ++strip)
               {
                // always suppress empty data:
                  if(fBdecay.front.hienergy[strip]>-1)
                    hDSSD_Front_Higain_Energy[strip]->Fill(fBdecay.front.hienergy[strip]);

                  if(fBdecay.front.loenergy[strip]>-1)
                     hDSSD_Front_Logain_Energy[strip]->Fill(fBdecay.front.loenergy[strip]);

                  if(fBdecay.back.hienergy[strip]>-1)
                    hDSSD_Back_Higain_Energy[strip]->Fill(fBdecay.back.hienergy[strip]);

                  if(fBdecay.back.loenergy[strip]>-1)
                    hDSSD_Back_Logain_Energy[strip]->Fill(fBdecay.back.loenergy[strip]);
               } // strip

    for(int npmt=0; npmt<GO4_DDAS_GAMMA_NPMTS; ++npmt)
             {
                // always suppress empty data:
                if(fBdecay.sun.rpmt[npmt]!=0)
                  hGamma_SuN_Intens[npmt]->Fill(fBdecay.sun.rpmt[npmt]);
                if(fBdecay.sun.gmpmt[npmt]!=0)
                  hGamma_SuN_Energy[npmt]->Fill(fBdecay.sun.gmpmt[npmt]);
                if(fBdecay.sun.tpmt[npmt]!=0)
                    hGamma_SuN_Time[npmt]->Fill(fBdecay.sun.tpmt[npmt] - starttime);
             }


    // TODO- further histograms for // Pin01, Pin02, I2pos, TAC,  XFP, Pin01 - I2S, Pin02 - I2N
    //Pin01 - XFP, Pin02 - XFP, Veto

    endtime = (*channellist_it)->GetTime();

  } /* For entire channellist... */

  /* Channels within an event have been unpacked into appropriate
   fBdecay parameters and hit flags have been set.  Maximum channels
   have already been established for the Si detectors in MapChannels. */

  //only fill output if the dssd got hit (implant OR decay):
  // JAM2020: may override this condition with fDoFiltering=false to get everything
  // may also define higher thresholds for filtering here in go4 parameter
  if(!fPar->fDoFiltering ||
       ((fBdecay.front.imax >  fPar->fDSSD_Front_Imax)&&(fBdecay.back.imax > fPar->fDSSD_Back_Imax)) ||
       ((fBdecay.front.dmax > fPar->fDSSD_Front_Dmax)&&(fBdecay.back.dmax > fPar->fDSSD_Back_Dmax)))
     {

       // JAM 2020 - we do not fill output tree directly, but copy this input event into our output event:
      fOutput->fDDASEvent=fInput->fDDASEvent;
      fOutput->SetValid(kTRUE); // only valid events are stored into tree
     }

}



void TDDASFilterProc::MapChannels(int crateid, int slotid, int channum, betadecay &bdecay, betadecayvariables &bdecayv)
{
  //depending on what is hit, fill parameters
  //calibrate and threshold check

  double starttime=0;
  starttime = bdecay.clock.current;

  double currenttime=0;

  //determine id of the channel
  int id;

  if(crateid == 3)
    {
      id = (slotid-2)*16 + channum;
    }
  else if (crateid == 1)
    {
      id = (crateid *10)*16 + (slotid-2)*16 + channum;
    }
  else
    {
      id = -1;
    }

  if( !(id>=0 && id<112)) cout << "****** problems with id value ****** " << crateid<< " " << slotid << " "
                   << channum << endl;


  //DSSD front hi gain
  if( (id >= 48) && (id < 64))
    {
      int strip = id - 48;
      //cout << " dssd front hi " << bdecayv.fthresh.hithresh[strip] << " "
      //     << bdecayv.fthresh.duld[strip] << endl;
      bdecay.front.himultiset[strip] = bdecay.front.himultiset[strip] + 1;
      bdecay.front.hienergy[strip] = bdecay.adc[slotid-1].channel[channum];
      bdecay.front.hiecal[strip] = bdecayv.front.hislope[strip] * (bdecay.front.hienergy[strip] - bdecayv.front.hioffset[strip]);
      bdecay.front.hitime[strip] = bdecay.time[slotid-1].timefull[channum];

      currenttime = bdecay.time[slotid-1].timefull[channum];

      bdecay.front.hieventtdc[strip] = (currenttime - starttime) + 3000.;


      if( (bdecay.front.hiecal[strip] >= bdecayv.fthresh.hithresh[strip]) && (bdecay.front.hiecal[strip] <= bdecayv.fthresh.duld[strip]))
    {
      bdecayv.hit.fronthi = 1;
      bdecay.front.dmult++;                                 // increment multiplicity
      bdecay.front.dsum += bdecay.front.hiecal[strip];       // increment energy sum

      if(bdecay.front.hiecal[strip] > bdecay.front.dmax)
        {
          bdecay.front.dmax = bdecay.front.hiecal[strip];
          bdecay.front.dmaxch = strip;
          bdecay.front.dmaxtime = bdecay.front.hitime[strip];
        }

    }

      //cout << " fd " <<strip << " " << bdecay.front.hiecal[strip] << " " << bdecay.front.dmax <<  endl;


    }


  //DSSD front lo gain
  if( (id >= 80) && (id < 96))
    {
      int strip = id-80;
      bdecay.front.lomultiset[strip] = bdecay.front.lomultiset[strip] + 1;
      bdecay.front.loenergy[strip] = bdecay.adc[slotid-1].channel[channum];
      bdecay.front.loecal[strip] = bdecayv.front.loslope[strip] * bdecay.front.loenergy[strip];
      bdecay.front.lotime[strip] = bdecay.time[slotid-1].timefull[channum];

      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.front.loeventtdc[strip] = (currenttime - starttime) + 3000.;


      if(bdecay.front.loecal[strip] > 0)
    {
      bdecayv.hit.frontloall = 1;
    }

      if( (bdecay.front.loecal[strip] >= bdecayv.fthresh.lothresh[strip]) && (bdecay.front.loecal[strip] <= bdecayv.fthresh.loduld[strip]) )
    {
      bdecayv.hit.frontlo = 1;
      bdecay.front.imult++;  // increment multiplicity
      bdecay.front.isum += bdecay.front.loecal[strip];  // increment energy sum

      if(bdecay.front.loecal[strip] > bdecay.front.imax)
        {
          bdecay.front.imax = bdecay.front.loecal[strip];
          bdecay.front.imaxch = strip;
          bdecay.front.imaxtime = bdecay.front.lotime[strip];
        }

    }

      //cout << " fi " <<strip << " " << bdecay.front.loecal[strip] << " " << bdecay.front.imax << endl;

    }

  //DSSD back hi gain
  if( (id >= 64) && (id < 80))
    {
      int strip = (id-64);
      //cout << " dssd back hi " << bdecayv.bthresh.hithresh[strip] << endl;
      bdecay.back.himultiset[strip] = bdecay.back.himultiset[strip] + 1;
      bdecay.back.hienergy[strip] = bdecay.adc[slotid-1].channel[channum];
      bdecay.back.hiecal[strip] = bdecayv.back.hislope[strip] * (bdecay.back.hienergy[strip] - bdecayv.back.hioffset[strip]);
      bdecay.back.hitime[strip] = bdecay.time[slotid-1].timefull[channum];

      currenttime =  bdecay.time[slotid-1].timefull[channum];
      bdecay.back.hieventtdc[strip] = (currenttime - starttime) + 3000.;



      if( (bdecay.back.hiecal[strip] >= bdecayv.bthresh.hithresh[strip]) && (bdecay.back.hiecal[strip] <= bdecayv.bthresh.duld[strip]))
    {
      bdecayv.hit.backhi = 1;
      bdecay.back.dmult++;                                 // increment multiplicity
      bdecay.back.dsum += bdecay.back.hiecal[strip];       // increment energy sum
      //dsumch += (strip)*bdecay.back.hiecal;    // increment weighted sum

      if(bdecay.back.hiecal[strip] > bdecay.back.dmax)
        {
          bdecay.back.dmax = bdecay.back.hiecal[strip];
          bdecay.back.dmaxch = strip;
          bdecay.back.dmaxtime = bdecay.back.hitime[strip];
        }

    }
      //cout << " bd " <<strip << " " << bdecay.back.hiecal[strip] << " " << bdecay.back.dmax <<  endl;

    }

  //DSSD back lo gain
  if( (id >= 96) && (id < 112))
    {
      int strip = id - 96;
      bdecay.back.lomultiset[strip] = bdecay.back.lomultiset[strip] + 1;
      bdecay.back.loenergy[strip] = bdecay.adc[slotid-1].channel[channum];
      bdecay.back.loecal[strip] = bdecayv.back.loslope[strip] * bdecay.back.loenergy[strip];
      bdecay.back.lotime[strip] = bdecay.time[slotid-1].timefull[channum];

      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.back.loeventtdc[strip] = (currenttime - starttime) + 3000.;




      if(bdecay.back.loecal[strip] > 0)
    {
      bdecayv.hit.backloall = 1;
    }

      if( (bdecay.back.loecal[strip] >= bdecayv.bthresh.lothresh[strip]) && (bdecay.back.loecal[strip] <= bdecayv.bthresh.loduld[strip]) )
    {
      bdecayv.hit.backlo = 1;
      bdecay.back.imult++;                                 // increment multiplicity
      bdecay.back.isum += bdecay.back.loecal[strip];       // increment energy sum

      if(bdecay.back.loecal[strip] > bdecay.back.imax)
        {
          bdecay.back.imax = bdecay.back.loecal[strip];
          bdecay.back.imaxch = strip;
          bdecay.back.imaxtime = bdecay.back.lotime[strip];
        }

    }
      //cout << " bi " <<strip << " " << bdecay.back.loecal[strip] << " " << bdecay.back.imax << endl;

    }


  // Gamma array
  if( ((id >= 0) && (id < 12)) || ((id >= 16) && (id < 28)) )
    { /*for eff cal files */

      int npmt;
      if( (id >= 16) && (id < 28) ) npmt = 12+ (id-16);
      else npmt = id;

      bdecay.sun.rpmt[npmt] = (bdecay.adc[slotid-1].channel[channum]) + (fRandom3.Rndm());
      bdecay.sun.tpmt[npmt] = bdecay.time[slotid-1].timefull[channum];
      bdecay.sun.gmpmt[npmt] =bdecayv.SuN.pslope[npmt] * bdecay.sun.rpmt[npmt] + bdecayv.SuN.pintercept[npmt];


      if(bdecay.sun.rpmt[npmt] > bdecayv.SuN.thresh[npmt])
    {
      bdecayv.hit.sun = 1;
    }

    }


  // Pin01
  if( id == 32 )
    {
      bdecay.pin01.multiset = bdecay.pin01.multiset + 1;
      bdecay.pin01.energy = bdecay.adc[slotid-1].channel[channum];
      bdecay.pin01.ecal = (bdecayv.pin01.slope*bdecay.pin01.energy + bdecayv.pin01.intercept);

      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.pin01.time = bdecay.time[slotid-1].timefull[channum];
      bdecay.pin01.eventtdc = (currenttime - starttime) + 3000.;

      if((bdecay.pin01.ecal > bdecayv.pin01.thresh) && (bdecay.pin01.ecal < 30000))
    {
      bdecayv.hit.pin01 = 1;
    }
      else if(bdecay.pin01.ecal>20)
    {
      bdecayv.hit.pin01 = 2;
    }
      else
    {
      bdecayv.hit.pin01 = -1;
    }
    }
  // Pin02
  if( id == 33 )
    {
      bdecay.pin02.multiset = bdecay.pin02.multiset + 1;
      bdecay.pin02.energy = bdecay.adc[slotid-1].channel[channum];
      bdecay.pin02.ecal = (bdecayv.pin02.slope*bdecay.pin02.energy + bdecayv.pin02.intercept);

      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.pin02.time = bdecay.time[slotid-1].timefull[channum];
      bdecay.pin02.eventtdc = (currenttime - starttime) + 3000.;

      if((bdecay.pin02.ecal > bdecayv.pin02.thresh) && (bdecay.pin02.ecal <30000) )
    {
      bdecayv.hit.pin02 = 1;
    }
      else if(bdecay.pin02.ecal>20)
    {
      bdecayv.hit.pin02 = 2;
    }
      else
    {
      bdecayv.hit.pin02 = -1;
    }
    }


  // I2pos TAC
  if( id == 38)
    {
      bdecay.tac.i2posmultiset = bdecay.tac.i2posmultiset + 1;
      bdecay.tac.i2pos = bdecay.adc[slotid-1].channel[channum]+3000.;
      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.tac.i2postdc = (currenttime - starttime) + 3000.;
      bdecay.tac.i2pos_cfdtime = bdecay.time[slotid-1].timecfd[channum];
      bdecay.tac.i2postime = bdecay.time[slotid-1].timefull[channum];
    }

  //Pin01 - XFP
  if(id == 39)
    {
      bdecay.tac.pin01i2nmultiset = bdecay.tac.pin01i2nmultiset + 1;
      bdecay.tac.pin01i2n = bdecay.adc[slotid-1].channel[channum];
      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.tac.i2ntdc = (currenttime - starttime) + 3000.;
    }

  //Pin01 - I2S
  if(id == 40)
    {
      bdecay.tac.pin01i2smultiset = bdecay.tac. pin01i2smultiset + 1;
      bdecay.tac.pin01i2s = bdecay.adc[slotid-1].channel[channum];
      currenttime = bdecay.time[slotid-1].timefull[channum];
      bdecay.tac.i2stdc = (currenttime - starttime) + 3000;
    }

  //Pin02 - I2N
  if(id == 41)
    {
      bdecay.tac.pin02i2nmultiset = bdecay.tac.pin02i2nmultiset + 1;
      bdecay.tac.pin02i2n = bdecay.adc[slotid-1].channel[channum];
      bdecay.tac.pin02i2ntime = bdecay.time[slotid-1].timefull[channum];
    }

  //Pin01 - XFP
  if(id == 35)
    {
      bdecay.tac.pin01xfpmultiset = bdecay.tac.pin01xfpmultiset + 1;
      bdecay.tac.pin01xfp = bdecay.adc[slotid-1].channel[channum];
      bdecay.tac.pin01xfptime = bdecay.time[slotid-1].timefull[channum];
    }

  //Pin02 - XFP
  if(id == 36)
    {
      bdecay.tac.pin02xfpmultiset = bdecay.tac.pin02xfpmultiset + 1;
      bdecay.tac.pin02xfp = bdecay.adc[slotid-1].channel[channum];
      bdecay.tac.pin02xfptime = bdecay.time[slotid-1].timefull[channum];
    }

  // Veto
  if( id == 34 )
    {
      bdecay.veto.multiset = bdecay.veto.multiset + 1;
      bdecay.veto.energy = bdecay.adc[slotid-1].channel[channum];
      bdecay.veto.time = bdecay.time[slotid-1].timefull[channum];

      /* // beta decays in the veto */
      /*   if( (bdecay.veto.energy > bdecayv.veto.thresh) && (bdecay.veto.energy < bdecayv.veto.uld) ){ */
      /*     bdecayv.hit.veto = 1; */
      /*   } */
      /*   else if( (bdecay.veto.energy> bdecayv.veto.uld) && (bdecay.veto.energy<bdecayv.veto.uld2) ){ // light charged particles */
      /*     bdecayv.hit.veto = 2; */
      /*   } */
      /*   else{ */
      /*     bdecayv.hit.pin02 = -1; */
      /*   } */
      /* } */
      if(bdecay.veto.energy > 0)
    {
      bdecayv.hit.veto = 1;
    }
    }
}



