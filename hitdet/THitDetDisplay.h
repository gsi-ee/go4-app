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


#define HitDet_MAXTRACELONG 256




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

   /** FFT transformed result of hTraceLong*/
   TH1 *hTraceLongFFT;

   /** Previously full filled hTraceLong. Coressponds to contents of hTraceLongFFT*/
   TH1 *hTraceLongPrev;

   /** statistics of message types*/
   TH1* hMsgTypes;

   /** statistics  of wishbone acknowledge codes*/
   TH1* hWishboneAck;

   /** statistics  of wishbone source*/
   TH1* hWishboneSource;

   /** text dump of last wishbone message*/
   TLatex* lWishboneText;
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
