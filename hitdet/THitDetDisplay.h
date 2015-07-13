#ifndef THitDetDISPLAY_H
#define THitDetDISPLAY_H

class THitDetRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "THitDetRawEvent.h"

#include "TGo4Analysis.h"


#define HitDet_MAXSNAPSHOTS 64



/*
 * Common histogram container ("displays") used in hit detection analysis
 *  (JAM July 2015)
 *  need to inherit from TGo4EventProcessor since we want to access histogram helper functions
 *  consists in vector of sub displays for each board/ HitDetection ASIC, resp.
 * */


//class THitDetDisplay: public TGo4EventProcessor
//{
//
//public:
//  THitDetDisplay() :
//      TGo4EventProcessor(), fDisplayId(-1)
//  {
//    ;
//  }
//
//  THitDetDisplay(Int_t deviceid);
//
//  virtual ~THitDetDisplay();
//
//  /* recreate histograms using the given number of trace length*/
//  virtual void InitDisplay(Int_t tracelength, Bool_t replace = kFALSE);
//
//  /* unique ID number of device to display (board or grid)*/
//  Int_t GetDevId()
//  {
//    return fDisplayId;
//  }
//
//  void AddBoardDisplay(THitDetBoardDisplay* brd)
//  {
//    fBoards.push_back(brd);
//  }
//
//  void ClearBoardDisplays()
//  {
//    std::for_each (fBoards.begin(), fBoards.end(), void delbrd(THitDetBoardDisplay* brd){delete brd;});
//    fBoards.clear();
//  }
//
//  Int_t GetNumBoards()
//  {
//    return fBoards.size();
//  }
//
//  /* access to subdisplay set for loopid (if existing)*/
//  THitDetBoardDisplay* GetBoardDisplay(UInt_t brdid)
//  {
//    if (loopid < fBoards.size())
//      return fBoards[loopid];
//    return 0;
//  }
//
//
//protected:
//
//
//
//  Int_t fDisplayId;
//
//  /* this holds array of subdisplays for each Board*/
//  std::vector<THitDetBoardDisplay*> fBoards;
//
//ClassDef(THitDetDisplay,1)
//};
//

/********************************
 * Histogram container for each HitDetection board
 *  (JAM July 2015)
 *
 * */
class THitDetBoardDisplay: public  TGo4EventProcessor
{

public:
  THitDetBoardDisplay() : TGo4EventProcessor()
  //, hHitDetRaw2D(0), hHitDetRaw2DTrace(0), hHitDetRawErr(0), hHitDetRawErrTr(0), hHitDetOffsets(0), pPexorQfws(0), pPexorQfwsTrace(0)
  {
    ;
  }
  THitDetBoardDisplay(Int_t boardid);
  virtual ~THitDetBoardDisplay();

  /* recreate histograms using the given trace length*/
  virtual void InitDisplay(Int_t tracelength, Int_t numsnapshots, Bool_t replace = kFALSE);

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

   /** statistics of message types*/
   TH1* hMsgTypes;


//
 Int_t GetDevId()
     {
       return fDisplayId;
     }



protected:

  Int_t fDisplayId;

//  TGo4Picture *pPexorQfws;
//  TGo4Picture *pPexorQfwsTrace;

ClassDef(THitDetBoardDisplay,1)
};


#endif //THitDetDISPLAY_H
