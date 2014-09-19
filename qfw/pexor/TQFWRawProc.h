#ifndef TQFWRawROCESSOR_H
#define TQFWRawPROCESSOR_H

/* VUPROM QFW beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */

class TQFWRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>

#include "TQFWRawEvent.h"
#include "TQFWDisplay.h"




class TQFWRawProc: public TGo4EventProcessor
{
protected:

  std::vector<TQFWBoardDisplay*> fBoards;

public:
  TQFWRawProc();
  TQFWRawProc(const char* name);
  virtual ~TQFWRawProc();

  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:

  /** Fill all display histograms here*/
  Bool_t FillDisplays();

  /** access to histogram set for current board id*/
  TQFWBoardDisplay* GetBoardDisplay(Int_t uniqueid);

  /** recreate histograms using the given number of time slice*/
  void InitDisplay(int timeslices, Bool_t replace = kFALSE);

  /** parameter for runtime settings*/
  TQFWRawParam* fPar;

  /** reference to output data*/
  TQFWRawEvent* QFWRawEvent;  //!


private:


  ClassDef(TQFWRawProc,1)
};

#endif //TUNPACKPROCESSOR_H
