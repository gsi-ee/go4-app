#ifndef THitDetRawPROCESSOR_H
#define THitDetRawPROCESSOR_H

/* VUPROM HitDet beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */

class THitDetRawParam;
class TGo4Fitter;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>

#include "THitDetRawEvent.h"
#include "THitDetDisplay.h"




class THitDetRawProc: public TGo4EventProcessor
{


public:
  THitDetRawProc();
  THitDetRawProc(const char* name);
  virtual ~THitDetRawProc();

  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:

  /** Additional histogram filling here*/
  Bool_t UpdateDisplays();

  /** access to histogram set for current board id*/
  THitDetBoardDisplay* GetBoardDisplay(Int_t uniqueid);

  /** recreate histograms using the given number of time slice*/
  void InitDisplay(Int_t timeslices, Int_t numsnapshots, Bool_t replace = kFALSE);

  /** provide FFT of complete trace long */
  void DoFFT( THitDetBoardDisplay* disp);

  /** transform hamming or other filter over array of lenght N*/
  void DoFilter(Double_t* array, Int_t N);

  /** calculate corrected adc value from raw entry, using correction vector histogram of display*/
  Double_t CorrectedADCVal(Short_t raw,  THitDetBoardDisplay* boardDisplay);


  /** Apply sinus fit to the full trace long*/
  void DoSinusFit( THitDetBoardDisplay* boardDisplay);

  /** subdisplays for each frotend board */
  std::vector<THitDetBoardDisplay*> fBoards;


  /** parameter for runtime settings*/
  THitDetRawParam* fPar;

  /** reference to output data*/
  THitDetRawEvent* HitDetRawEvent;  //!

  /** remember most recent message for delta T evaluation*/
  THitDetMsgEvent fLastMessages[HitDet_CHANNELS];


  TGo4Fitter* fSinusFitter;

  ClassDef(THitDetRawProc,1)
};

#endif //TUNPACKPROCESSOR_H
