#ifndef TCtr16DISPLAY_H
#define TCtr16DISPLAY_H

class TCtr16RawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "TCtr16RawEvent.h"

#include "TGo4Analysis.h"

#include "TLatex.h"

#include "TGo4RollingGraph.h"

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
 hChipId(0), hWishboneAck(0), hWishboneSource(0), lWishboneText(0),
 hErrorcodes(0), hErrorTimestamp(0),  hMemoryCell(0),hMemoryBlockRowMap(0),
 fDisplayId(0)
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

   /** Histogram of all ADC values for each channel*/
   TH1 *hADCPerChan[Ctr16_CHANNELS];
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


   /** epoch statistic per channel*/
   TH1 *hEpochs[Ctr16_CHANNELS];

     /** timestamp statistic per channel*/
   TH1 *hTimestamps[Ctr16_CHANNELS];

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

   /** ADC baseline from threshold scans for each channel*/
   TH1 *hThresholdBaseline[Ctr16_CHANNELS];

   /** Noise width from threshold scans for each channel*/
   TH1 *hThresholdNoise[Ctr16_BLOCKS];

   /** Set ADC threshold value from threshold scans for each channel*/
   TH1 *hThresholdSetting[Ctr16_BLOCKS];

   /** Mean value from threshold scans for each channel*/
   TH1 * hThresholdMean[Ctr16_BLOCKS];

   /** Set ADC trackkingvalue from threshold scans for each channel*/
   TH1 * hThresholdTracking[Ctr16_BLOCKS];

#ifdef   Ctr16_USE_ROLLINGGRAPH
   /** Noise width from threshold scans for each channel, trending*/
     TGo4RollingGraph *fTrendThresholdNoise[Ctr16_BLOCKS];

     /** Set ADC threshold value from threshold scans for each channel, trending*/
     TGo4RollingGraph* fTrendThresholdSetting[Ctr16_BLOCKS];

     /** Mean value from threshold scans for each channe, trendingl*/
     TGo4RollingGraph* fTrendThresholdMean[Ctr16_BLOCKS];

     /** Set ADC trackkingvalue from threshold scans for each channe, trendingl*/
     TGo4RollingGraph* fTrendThresholdTracking[Ctr16_BLOCKS];
#endif

     /** Feature extracted amplitudes for each channel*/
   TH1 *hFeatureAmplitude[Ctr16_CHANNELS];


   /** Feature extracted amplitudes for each channel, scaled with CSA value*/
   TH1 *hFeatureAmplitudeScaled[Ctr16_CHANNELS];

      /** Feature extracted fine times for each channel*/
   TH1 *hFeatureFineTime[Ctr16_CHANNELS];


   /** CSA amplification factor per channel*/
   TH1 *hCsaAmp[Ctr16_CHANNELS];



   /** statistics  of errorframe codes*/
     TH1* hErrorcodes;

     /** error frame timestamps*/
     TH1* hErrorTimestamp;


     /*Partitioning of ADC values for each memory cell*/
    TH1* hADCValuesPerCell [Ctr16_BLOCKS] [Ctr16_CHANNELROWS][Ctr16_MEMORYCELLS];

    /* Scaler for all memory cells that send ADC data*/
    TH1* hMemoryCell;

    /* Block row overview scaler for that send ADC data*/
    TH2* hMemoryBlockRowMap;

    /* row -cell overview scaler for that send ADC data*/
    TH2* hMemoryRowCellMap[Ctr16_BLOCKS];



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
