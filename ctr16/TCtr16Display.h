#ifndef TCtr16DISPLAY_H
#define TCtr16DISPLAY_H

class TCtr16RawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "TCtr16RawEvent.h"

#include "TGo4Analysis.h"

#include "TLatex.h"

#define Ctr16_MAXSNAPSHOTS 64


#define Ctr16_MAXTRACELONG 512




/********************************
 * Histogram container for each Ctr16ection board
 *  (JAM July 2015)
 *
 * */
class TCtr16BoardDisplay: public  TGo4EventProcessor
{

public:
  TCtr16BoardDisplay() : TGo4EventProcessor(),
 hADCValues(0), hADCDeltaMeanValues (0), hADCNonLinDiff(0), hADCNonLinInt(0), hADCCorrection(0), hADCCValuesCorrected(0),
 hFrameTypes(0), hMsgTypes(0), hDataTypes(0), hChannels(0), hDatawords(0), hMemoryRow(0),
 hChipId(0), hWishboneAck(0), hWishboneSource(0), lWishboneText(0), fDisplayId(0)
  {
    ;
  }
  TCtr16BoardDisplay(Int_t boardid);
  virtual ~TCtr16BoardDisplay();

  /* recreate histograms using the given trace length*/
  virtual void InitDisplay(Int_t tracelength, Int_t numsnapshots, Bool_t replace = kFALSE);

  /** reset all trace histograms here and occ. update something*/
  void ResetDisplay();


  /** sampled signal for each channel*/
  TH1 *hTrace[Ctr16_CHANNELS];

  /** accumulated sampled signals for each channel*/
  TH1 *hTraceSum[Ctr16_CHANNELS];

  /** sampled signal snapshot sequence within this mbs event for each channel  */
  TH1* hTraceSnapshots [Ctr16_CHANNELS] [Ctr16_MAXSNAPSHOTS];

  /** sampled signal snapshot sequence within this mbs event for each channel alternative display  */
  TH2* hTraceSnapshot2d[Ctr16_CHANNELS];

  /** counter for snapshot display index*/
  UInt_t fSnapshotcount[Ctr16_CHANNELS];



   /** Histogram of all ADC Values retrieved*/
   TH1 *hADCValues;

   /** Deviation of ADC Values from mean*/
   TH1 *hADCDeltaMeanValues;

   /** Differential ADC nonlinearity*/
   TH1 *hADCNonLinDiff;

   /** Integral ADC nonlinearity*/
   TH1 *hADCNonLinInt;

   /** ADC correction vector. Derived from hADCNonLinInt during adc calibrate mode*/
   TH1 *hADCCorrection;

   /** Histogram of all ADC values after correction*/
   TH1 *hADCCValuesCorrected;

   /** statistics of frame types*/
   TH1* hFrameTypes;

   /** statistics of message types*/
   TH1* hMsgTypes;

   /** statistics of data types*/
   TH1* hDataTypes;


   /** distribution of channels for message type 3*/
   TH1* hChannels;

   /** size of data for message type */
   TH1* hDatawords;

   /** debug the memory rows used on chip*/
   TH1* hMemoryRow;

   /** check chipid for debug*/
   TH1* hChipId;

   /** timestamp difference of subsequent event messages*/
   TH1 *hDeltaTSMsg[Ctr16_CHANNELS];

   /** epoch difference of subsequent event messages*/
   TH1 *hDeltaEPMsg[Ctr16_CHANNELS];

   /** epoch difference fine of subsequent event messages*/
   TH1 *hDeltaEPMsgFine[Ctr16_CHANNELS];


   /** statistics  of wishbone acknowledge codes*/
   TH1* hWishboneAck;

   /** statistics  of wishbone source*/
   TH1* hWishboneSource;

   /** text dump of last wishbone message*/
   TLatex* lWishboneText;

   /** Mean baseline from threshold scans for each channel*/
   TH1 *hThresholdBaseline[Ctr16_CHANNELS];

   /** Noise width from threshold scans for each channel*/
   TH1 *hThresholdNoise[Ctr16_CHANNELS];

   /** Set threshold value from threshold scans for each channel*/
   TH1 *hThresholdSetting[Ctr16_CHANNELS];

   /** Feature extracted amplitudes for each channel*/
   TH1 *hFeatureAmplitude[Ctr16_CHANNELS];

   /** Feature extracted fine times for each channel*/
   TH1 *hFeatureFineTime[Ctr16_CHANNELS];



   /** statistics  of errorframe codes*/
     TH1* hErrorcodes;

     /** error frame timestamps*/
     TH1* hErrorTimestamp;


     /////////////////////////////

     Int_t GetDevId()
     {
       return fDisplayId;
     }



protected:

  Int_t fDisplayId;


ClassDef(TCtr16BoardDisplay,1)
};


#endif //TCtr16DISPLAY_H
