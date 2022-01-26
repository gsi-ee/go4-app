#ifndef TQFWProfileProcessor_H
#define TQFWProfileProcessor_H

/* VUPROM QFW beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */


#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>

#include "TQFWProfileEvent.h"
#include "TQFWRawEvent.h"
#include "TQFWDisplay.h"


#include "TQFWProfileParam.h"



class TQFWProfileProc: public TGo4EventProcessor
{
protected:

  TQFWProfileEvent* fOutput; //!

  TQFWProfileParam* fParam;

  std::vector<TQFWGridDisplay*> fGrids;
  std::vector<TQFWCupDisplay*> fCups;

  /** remember output name of statistic file*/
  TString fCountStatfile; //!

public:
  TQFWProfileProc();
  TQFWProfileProc(const char* name);
  virtual ~TQFWProfileProc();



  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:




  /* recreate histograms using the given number of time slice*/
  void InitDisplay(int timeslices, Bool_t replace = kFALSE);

  /** JAM2022 new helper function to evaluate difference profiles between measurement loops*/
  void FillDifferenceHistogram(TH1* result, TH1* minuend, TH1* subtrahend);

private:


  ClassDef(TQFWProfileProc,1)
};

#endif //TUNPACKPROCESSOR_H
