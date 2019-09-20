#ifndef THitDetDISPLAY_H
#define THitDetDISPLAY_H

class THitDetRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "THitDetRawEvent.h"

#include "TGo4Analysis.h"

#include "TLatex.h"

#define HitDet_MAXSNAPSHOTS 64


#define HitDet_MAXTRACELONG 512




/********************************
 * Histogram container for each HitDetection board
 *  (JAM July 2015)
 *
 * */
class THitDetBoardDisplay: public  TGo4EventProcessor
{

public:
  THitDetBoardDisplay() : TGo4EventProcessor(),
  hTraceLong(0), hTraceLongSum(0), hTraceLongFFT(0), hMsgTypes(0), fDisplayId(0)
  {
    ;
  }
  THitDetBoardDisplay(Int_t boardid);
  virtual ~THitDetBoardDisplay();

  /* recreate histograms using the given trace length*/
  virtual void InitDisplay(Int_t tracelength, Int_t numsnapshots, Bool_t replace = kFALSE);

  /** reset all trace histograms here and occ. update something
   * long direct ADC trace is only cleared if argument is true*/
  void ResetDisplay(Bool_t cleartracelong=kFALSE);

// JAM put histograms etc here


  /** sampled signal for each channel*/
  TH1 *hTrace[HitDet_CHANNELS];

  /** accumulated sampled signals for each channel*/
  TH1 *hTraceSum[HitDet_CHANNELS];

  /** sampled signal snapshot sequence within this mbs event for each channel  */
  TH1* hTraceSnapshots [HitDet_CHANNELS] [HitDet_MAXSNAPSHOTS];

  /** sampled signal snapshot sequence within this mbs event for each channel alternative display  */
  TH2* hTraceSnapshot2d[HitDet_CHANNELS];

  /** for direct ADC readout: subsequent traces stitched together in one display, no channel info here!*/
   TH1 *hTraceLong;

   /** for direct ADC readout: accumulated subsequent traces stitched together in one display, no channel info here!*/
   TH1 *hTraceLongSum;

   /** FFT transformed result of hTraceLongPrev*/
   TH1 *hTraceLongFFT;

   /** Previously full filled hTraceLong. Coressponds to contents of hTraceLongFFT*/
   TH1 *hTraceLongPrev;

   /** FFT transformed result of conditino window inside hTraceLong*/
   TH1 *hTracePartFFT;


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

   /** for direct ADC readout: subsequent traces stitched together and corrected for inl*/
   TH1 *hTraceLongCorrected;

   /** for direct ADC readout: subsequent traces stitched together and corrected for inl*/
   TH1 *hTraceLongPrevCorrected;

   /** FFT transformed result of condition window inside hTraceLongPrevCorrected*/
   TH1 *hTracePartCorrectedFFT;

   /** for direct ADC readout: accumulated subsequent traces stitched together and corrected for inl*/
   TH1 *hTraceLongSumCorrected;




   /** statistics of message types*/
   TH1* hMsgTypes;

   /** distribution of channels for message type 3*/
   TH1* hChannels;

   /** size of data for message type */
   TH1* hDatawords;

   /** debug the memory rows used on chip*/
   TH1* hMemoryRow;

   /** check chipid for debug*/
   TH1* hChipId;

   /** timestamp difference of subsequent event messages*/
   TH1 *hDeltaTSMsg[HitDet_CHANNELS];

   /** epoch difference of subsequent event messages*/
   TH1 *hDeltaEPMsg[HitDet_CHANNELS];

   /** epoch difference fine of subsequent event messages*/
   TH1 *hDeltaEPMsgFine[HitDet_CHANNELS];


   /** statistics  of wishbone acknowledge codes*/
   TH1* hWishboneAck;

   /** statistics  of wishbone source*/
   TH1* hWishboneSource;

   /** text dump of last wishbone message*/
   TLatex* lWishboneText;

   /** window condition to select region for part fft*/
   TGo4WinCond* cWindowFFT;

   //
 Int_t GetDevId()
     {
       return fDisplayId;
     }



protected:

  Int_t fDisplayId;


ClassDef(THitDetBoardDisplay,1)
};


#endif //THitDetDISPLAY_H
