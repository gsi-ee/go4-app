#ifndef TGet4ppDISPLAY_H
#define TGet4ppDISPLAY_H

class TGet4ppRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "TGet4ppRawEvent.h"

#include "TGo4Analysis.h"

#include "TLatex.h"




/********************************
 * Histogram container for each Get4pp board
 *  (JAM April 2020)
 *
 * */
class TGet4ppBoardDisplay: public  TGo4EventProcessor
{

public:
  TGet4ppBoardDisplay() : TGo4EventProcessor(),
  hChipId(0), hMsgTypes(0), hChannels(0), hSyncFlags(0), hWishboneAck(0), hWishboneSource(0), lWishboneText(0), fDisplayId(0)
  {
    ;
  }
  TGet4ppBoardDisplay(Int_t boardid);
  virtual ~TGet4ppBoardDisplay();

  /* recreate histograms using the given trace length*/
  virtual void InitDisplay(Bool_t replace = kFALSE);

  /** reset all trace histograms here and occ. update something
   * long direct ADC trace is only cleared if argument is true*/
  void ResetDisplay();

  /** reset all tdc calibration histograms*/
  void ResetCalibration();

  /** Test for existing calibration: returns true if all channels/edges has already calibration from ASF */
  Bool_t CheckCalibration();

  /** evaluate tdc calibration histograms from recent statistics*/
   Bool_t DoCalibration();

   /** check chipid for debug*/
   TH1* hChipId;

  /** statistics of message types*/
   TH1* hMsgTypes;

   /** distribution of channels for message type 0*/
   TH1* hChannels;

   /** flags forsync and epoch sync*/
   TH1* hSyncFlags;

    /** leading/trailing edges statistics per channel*/
   TH1* hEdges[Get4pp_CHANNELS];

   /** lost leading/trailing edges statistics per channel*/
   TH1* hLostEdges[Get4pp_CHANNELS];

   /** TDC data vs error event statistics per channel*/
   TH1* hEventTypes[Get4pp_CHANNELS];

  /** histogram of all coarse times per TDC channel, for leading/trailing edges*/
  TH1 *hCoarseTime[Get4pp_CHANNELS][2];

  /** histogram of all fine times per TDC channel, for leading/trailing edges*/
  TH1 *hFineTime[Get4pp_CHANNELS][2];

  /** histogram of all epoch times per TDC channel, for leading/trailing edges*/
  TH1 *hEpochs[Get4pp_CHANNELS][2];

  /** histogram of full timestamp per TDC channel, for leading/trailing edges*/
  TH1 *hFullTime[Get4pp_CHANNELS][2];

  /** histogram of timestamp per TDC channel in seconds unit, for leading/trailing edges*/
  TH1 *hTimeInSeconds[Get4pp_CHANNELS][2];

  /** histogram of time over threshold per TDC channel (diff time of subsequent leading/trailing edge)*/
  TH1 *hToTinSeconds [Get4pp_CHANNELS];

  /** Scalers for error event codes, per channel. Errors without specific channels are put into channel 0*/
  TH1 *hErrorCodes [Get4pp_CHANNELS];

  /** DLL phase value of error events, per channel. Errors without specific channels are put into channel 0*/
  TH1 *hErrorDLLPhase [Get4pp_CHANNELS];

   /** statistics  of wishbone acknowledge codes*/
   TH1* hWishboneAck;

   /** statistics  of wishbone source*/
   TH1* hWishboneSource;

   /** text dump of last wishbone message*/
   TLatex* lWishboneText;


   ////JAM 20-10-2022 new histograms for delta t below:

   /** leading edge delta time between channels, fine time units. Note that not all indices for pointers are used */
    TH1 *hDeltaTime[Get4pp_CHANNELS][Get4pp_CHANNELS];

    /** leading edge delta time between channels,unit seconds. Note that not all indices for pointers are used */
    TH1 *hDeltaTimeInSeconds[Get4pp_CHANNELS][Get4pp_CHANNELS];


    // JAM 11-01-2023: now add software calibration for fine time

    /** histogram for accumulated fine time sum, used software calibration per TDC channel, for leading/trailing edges*/
     TH1 *hFineTimeSum[Get4pp_CHANNELS][2];

    /** histogram for fine time software calibration per TDC channel, for leading/trailing edges*/
    TH1 *hFineCalibration[Get4pp_CHANNELS][2];


    /** leading edge calibrated delta time between channels, fine time units. Note that not all indices for pointers are used */
    TH1 *hDeltaTimeCalibrated[Get4pp_CHANNELS][Get4pp_CHANNELS];

        /** leading edge calibrated delta time between channels,unit seconds. Note that not all indices for pointers are used */
    TH1 *hDeltaTimeCalibratedInSeconds[Get4pp_CHANNELS][Get4pp_CHANNELS];


  
   //
 Int_t GetDevId()
     {
       return fDisplayId;
     }

 Double_t GetLastLeadingTime(UChar_t channel)
 {
	 if(channel <Get4pp_CHANNELS)
		 return fLastLeadingEdgeTime[channel];
	 else
		 return -1.0;
 }

 Bool_t SetLastLeadingTime(UChar_t channel, Double_t val)
 {
	 if(channel >=Get4pp_CHANNELS) return kFALSE;
	 fLastLeadingEdgeTime[channel]=val;
	 return kTRUE;
 }

	Bool_t LastEdgeWasLeading(UChar_t channel) {
		if (channel < Get4pp_CHANNELS)
			return fHasLeadingEdge[channel];
		else
			return kFALSE;
	}

	Bool_t SetLastEdgeLeading(UChar_t channel,	Bool_t on)
	{
		 if(channel >=Get4pp_CHANNELS) return kFALSE;
		 fHasLeadingEdge[channel]=on;
		 return kTRUE;
	}



protected:

  Int_t fDisplayId;

  /** last leadinge edge time of this channel for ToT caluclations*/
  Double_t fLastLeadingEdgeTime[Get4pp_CHANNELS];

  /** flag that there has been leading edge in this channel before trailing*/
  Bool_t fHasLeadingEdge[Get4pp_CHANNELS];



ClassDef(TGet4ppBoardDisplay,1)
};


#endif //TGet4ppDISPLAY_H
