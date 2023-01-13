#ifndef TGet4ppRawPROCESSOR_H
#define TGet4ppRawPROCESSOR_H

/* VUPROM Get4pp beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */

class TGet4ppRawParam;
//class TGo4Fitter;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>

#include "TGet4ppRawEvent.h"
#include "TGet4ppDisplay.h"




class TGet4ppRawProc: public TGo4EventProcessor
{


public:
  TGet4ppRawProc();
  TGet4ppRawProc(const char* name);
  virtual ~TGet4ppRawProc();

  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:

  /** Additional histogram filling here*/
  Bool_t UpdateDisplays();

  /** access to histogram set for current board id*/
  TGet4ppBoardDisplay* GetBoardDisplay(Int_t uniqueid);

  /** recreate histograms using the given number of time slice*/
  void InitDisplay(Bool_t replace = kFALSE);

  /** Evaluate histograms for fine time software calibration*/
  Bool_t DoCalibrations();

  /** Clear fine time calibration histograms for all known board displays*/
  void ResetCalibrations();

  /** subdisplays for each frotend board */
  std::vector<TGet4ppBoardDisplay*> fBoards;

  /** parameter for runtime settings*/
  TGet4ppRawParam* fPar;

  /** reference to output data*/
  TGet4ppRawEvent* Get4ppRawEvent;  //!

#ifdef Get4pp_DOFINETIMSAMPLES
/* local event counter for writing out fine time binds decision (simple approach)*/
  Int_t fEventCounter;
#endif

  /** flag to indicate that we have a valid fine time software calibration available*/
  Bool_t fCalibrationReady;

  /** flag to check old tdc calibration from ASF file and use this by default*/
  Bool_t fUseOldCalibration;

  /** channel statistics counter for calibration procedure*/
  Int_t fCalibrationCounter;

  ClassDef(TGet4ppRawProc,1)
};

#endif //TUNPACKPROCESSOR_H
