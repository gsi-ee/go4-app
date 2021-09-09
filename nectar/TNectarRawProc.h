#ifndef TNectarRawPROCESSOR_H
#define TNectarRawPROCESSOR_H

/* VUPROM Nectar beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */

class TNectarRawParam;
class TGo4Fitter;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>

#include "TNectarRawEvent.h"
#include "TNectarDisplay.h"




class TNectarRawProc: public TGo4EventProcessor
{


public:
  TNectarRawProc();
  TNectarRawProc(const char* name);
  virtual ~TNectarRawProc();

  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:


  /** extract payload of  mdpp */
  Bool_t UnpackMdpp();

  /** extract payload of  vmmr */
  Bool_t UnpackVmmr();

  /** Additional histogram filling here*/
  Bool_t UpdateDisplays();

  /** access to histogram set for current board id*/
  TMdppDisplay* GetMdppDisplay(UInt_t uniqueid);

  /** access to histogram set for current board id*/
  TVmmrDisplay* GetVmmrDisplay(UInt_t uniqueid);


  /** recreate histograms ; if replace redefine previous histograms of same name*/
  void InitDisplay(Bool_t replace = kFALSE);



  /** subdisplays for each frontend board */
  std::vector<TMdppDisplay*> fMdppDisplays;

  /** subdisplays for each frontend board */
   std::vector<TVmmrDisplay*> fVmmrDisplays;



  /** parameter for runtime settings*/
  TNectarRawParam* fPar;

  /** reference to output data*/
  TNectarRawEvent* fNectarRawEvent;  //!

  /** pointer to currrent mbs subevent*/
  TGo4MbsSubEvent* pSubevt;

  /** pointer cursor to current data in input subevent buffer*/
  Int_t *pData;

  /** total length of current subevent payload in words.*/
  Int_t lWords;

  ClassDef(TNectarRawProc,1)
};

#endif //TUNPACKPROCESSOR_H
