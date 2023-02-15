#ifndef TCtr16RawPROCESSOR_H
#define TCtr16RawPROCESSOR_H

/* VUPROM Ctr16 beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */

class TCtr16RawParam;
class TGo4Fitter;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>

#include "TCtr16RawEvent.h"
#include "TCtr16Display.h"




class TCtr16RawProc: public TGo4EventProcessor
{


public:
  TCtr16RawProc();
  TCtr16RawProc(const char* name);
  virtual ~TCtr16RawProc();

  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:

  /** Additional histogram filling here*/
  Bool_t UpdateDisplays();

  /** access to histogram set for current board id*/
  TCtr16BoardDisplay* GetBoardDisplay(Int_t uniqueid);

  /** recreate histograms using the given number of time slice*/
  void InitDisplay(Int_t timeslices, Int_t numsnapshots, Bool_t replace = kFALSE);


  /** get next data word from subevent payload.
   * Optionally align it to word boundaries.
   * Return values: 1: message exceeded, 2: subevent exceeded, 0:OK
   * */
   Int_t NextDataWord();

  /** Change alignment of subevent payload words*/
  void SwitchDataAlignment();

  /* Get trace data from transient event message.*
   * Return values: 1: message exceeded, 2: subevent exceeded, 0:OK
   * */
  Int_t ExtractTrace(TCtr16Board* board, TCtr16BoardDisplay* disp);

  /* Scan through most recent trace data of board and build event message.
   * Also fill trace histograms and snapshots*/
  void FinalizeTrace(TCtr16Board* board, TCtr16BoardDisplay* disp);

  /** Evaluate time differences between current event ev and previous event of that channel*/
  void UpdateDeltaTimes(TCtr16Board* board, TCtr16BoardDisplay* disp, TCtr16MsgEvent* ev, UChar_t chan);

  /** calculate corrected adc value from raw entry, using correction vector histogram of display*/
  Double_t CorrectedADCVal(Short_t raw,  TCtr16BoardDisplay* boardDisplay);



  /** subdisplays for each frotend board */
  std::vector<TCtr16BoardDisplay*> fBoards;


  /** parameter for runtime settings*/
  TCtr16RawParam* fPar;

  /** the current input subevent */
  TGo4MbsSubEvent *fPsubevt;

  /** reference to output data*/
  TCtr16RawEvent* Ctr16RawEvent;  //!

//  /** remember most recent message for delta T evaluation*/
  TCtr16MsgEvent fLastMessages[Ctr16_CHANNELS];

 /** unpacker working pointer*/
  Int_t *fPdata; //!

  /** begin of payload working pointer*/
  Int_t* fPdatastart; //!

  /** length of subevent in 32bit words*/
  Int_t fLwords; //!

  /** begin of message working pointer*/
  Int_t* fPdatastartMsg; //!

  /** message size in 32bit words*/
  Int_t fMsize; //!

  /** Copy of current working data
   * For message alignment */
  Int_t fWorkData; //!

  /** Required bitshift to align payload to 32bit*/
  Int_t fWorkShift; //!


  ClassDef(TCtr16RawProc,1)
};

#endif //TUNPACKPROCESSOR_H
