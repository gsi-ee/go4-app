#ifndef THitDetRawROCESSOR_H
#define THitDetRawPROCESSOR_H

/* VUPROM HitDet beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */

class THitDetRawParam;

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

  /** subdisplays for each frotend board */
  std::vector<THitDetBoardDisplay*> fBoards;


  /** parameter for runtime settings*/
  THitDetRawParam* fPar;

  /** reference to output data*/
  THitDetRawEvent* HitDetRawEvent;  //!



  ClassDef(THitDetRawProc,1)
};

#endif //TUNPACKPROCESSOR_H
