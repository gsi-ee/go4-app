#ifndef TQFWDISPLAY_H
#define TQFWDISPLAY_H

class TQFWRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "TQFWRawEvent.h"
#include "TQFWProfileParam.h"

#include "TGo4Analysis.h"

/*
 * Base Class for all histogram containers ("displays") that are used in qfw analysis
 *  (JAM December 2013)
 *  need to inherit from TGo4EventProcessor since we want to access histogram helper functions
 *
 * */
class TQFWProfileEvent;
class TQFWGrid;
class TQFWCup;

class TQFWLoopDisplay;

class TQFWDisplay: public TGo4EventProcessor
{

public:
  TQFWDisplay() :
      TGo4EventProcessor(), fDisplayId(-1)
  {
    ;
  }

  TQFWDisplay(Int_t deviceid);

  virtual ~TQFWDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* unique ID number of device to display (board or grid)*/
  Int_t GetDevId()
  {
    return fDisplayId;
  }

  void AddLoopDisplay(TQFWLoopDisplay* loop)
  {
    fLoops.push_back(loop);
  }

  void ClearLoopDisplays()
  {
    fLoops.clear();
  }

  Int_t GetNumLoops()
  {
    return fLoops.size();
  }

  /* access to subdisplay set for loopid (if existing)*/
  TQFWLoopDisplay* GetSubDisplay(UInt_t loopid)
  {
    if (loopid < fLoops.size())
      return fLoops[loopid];
    return 0;
  }

  /* Evaluate description of measurement range from qfw setup value*/
//  TString GetSetupString(UChar_t qfwsetup);

protected:

  /* helper function to register variable binsize histogram
   * this is still missing in go4 framework, we put it first here for convencience*/
  TH1* MakeVarbinsTH1(Bool_t replace, char type, const char* fullname, const char* title, Int_t nbinsx, Double_t* xbins,
      const char* xtitle = 0, const char* ytitle = 0);

  Int_t fDisplayId;

  /* this holds array of olptional subdisplays for each time loop*/
  std::vector<TQFWLoopDisplay*> fLoops;

ClassDef(TQFWDisplay,1)
};

/*
 * Intermediate base class for all histogram sets of a single time loop
 *
 */
class TQFWLoopDisplay: public TQFWDisplay
{

public:
  TQFWLoopDisplay() :
      TQFWDisplay(-1), fLoopId(-1), fTimeSlices(-1)
  {
    ;
  }

  TQFWLoopDisplay(Int_t deviceid, Int_t loopid = -1);
  virtual ~TQFWLoopDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* id of time loop*/
  Int_t GetLoopId()
  {
    return fLoopId;
  }

  /* id of time loop*/
  Int_t GetTimeSlices()
  {
    return fTimeSlices;
  }

protected:

  Int_t fLoopId;

  Int_t fTimeSlices;

ClassDef(TQFWLoopDisplay,1)
};

/********************************
 * Histogram container for a single time loop on each board
 *  (JAM December 2013)
 *
 * */
class TQFWBoardLoopDisplay: public TQFWLoopDisplay
{

public:
  TQFWBoardLoopDisplay() :
      TQFWLoopDisplay(), hQFWRaw(0), hQFWRawTrace(0)
  {
    ;
  }

  TQFWBoardLoopDisplay(Int_t boardid, Int_t loopid);
  virtual ~TQFWBoardLoopDisplay()
  {
    ;
  }

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* Display qfw scaler accumulated channels in one histo for this loop
   * Number of bins is equal to 32 chanel x 200 slices
   * PEXOR_QFWCHANS x timeslices (dynamic)
   * */

  TH1* hQFWRaw;

  /* Display qfw scaler trace channels (from current event) in one histo per board/loop
   * Number of bins is equal to 32 chanel x 200 slices */
  TH1* hQFWRawTrace;

ClassDef(TQFWBoardLoopDisplay,1)
};

/********************************
 * Histogram container for each qfw board
 *  (JAM December 2013)
 *
 * */
class TQFWBoardDisplay: public TQFWDisplay
{

public:
  TQFWBoardDisplay() :
      TQFWDisplay(), hQFWRaw2D(0), hQFWRaw2DTrace(0), hQFWRawErr(0), hQFWRawErrTr(0), pPexorQfws(0), pPexorQfwsTrace(0)
  {
    ;
  }
  TQFWBoardDisplay(Int_t boardid);
  virtual ~TQFWBoardDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* access to histogram set for current loop id*/
  TQFWBoardLoopDisplay* GetLoopDisplay(Int_t index);

  /** Two dimensional accumulated channels in one histo, includes all loops */
  TH2* hQFWRaw2D;

  /** Two dimensional, includes all loops */
  TH2* hQFWRaw2DTrace;

  /** Accumulated histogram of errors per QFW */
  TH1* hQFWRawErr;

  /** Trace histogram of errors per QFW */
  TH1* hQFWRawErrTr;

  TGo4Picture *pPexorQfws;
  TGo4Picture *pPexorQfwsTrace;

ClassDef(TQFWBoardDisplay,1)
};

/********************************
 * Histogram container for a single time loop of each grid
 *  (JAM December 2013)
 *
 * */
class TQFWGridLoopDisplay: public TQFWLoopDisplay
{

public:
  TQFWGridLoopDisplay() :
      TQFWLoopDisplay(), hBeamXSlice(0), hBeamYSlice(0), hBeamXSliceOffs(0), hBeamYSliceOffs(0), hBeamAccXSlice(0),
          hBeamAccYSlice(0),
          hBeamChargeXSlice(0), hBeamChargeYSlice(0), hBeamAccChargeXSlice(0), hBeamAccChargeYSlice(0),
          hBeamCurrentXSlice(0), hBeamCurrentYSlice(0), hBeamAveCurrentXSlice(0), hBeamAveCurrentYSlice(0),
          hBeamLoopX(0), hBeamLoopY(0), hBeamAccLoopX(0), hBeamAccLoopY(0), hPosLoopX(0),
          hPosLoopY(0), hPosAccLoopX(0), hPosAccLoopY(0), hPosQLoopX(0), hPosQLoopY(0), hPosQAccLoopX(0), hPosQAccLoopY(0),
          hPosILoopX(0), hPosILoopY(0), hPosIAveLoopX(0), hPosIAveLoopY(0),
          hBeamMeanCountsX(0), hBeamMeanCountsY(0), hBeamRMSCountsX(0),
          hBeamRMSCountsY(0), cBeamXSliceCond(0), cBeamYSliceCond(0), fGridData(0), fParam(0)
  {
    ;
  }

  TQFWGridLoopDisplay(Int_t grid, Int_t loopid);
  virtual ~TQFWGridLoopDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* TODO: put some information into histogram headers*/
  void AdjustDisplay(TQFWLoop* data);

  /* create link between this display and the event structure for same unique id*/
  void SetGridEvent(TQFWProfileEvent* out);

  /*  create link between this display and setup parameter*/
  void SetProfileParam(TQFWProfileParam* par)
  {
    fParam = par;
  }

  /* xy projections of scalers mapped to grid*/
  TH2* hBeamXSlice;
  TH2* hBeamYSlice;

  /* helper histogram showing current offset (in counts)*/
  TH2* hBeamXSliceOffs;
  TH2* hBeamYSliceOffs;

  TH2* hBeamAccXSlice;
  TH2* hBeamAccYSlice;

  /* xy projections of charge mapped to grid*/
   TH2* hBeamChargeXSlice;
   TH2* hBeamChargeYSlice;
   /* xy projections of charge accumulated mapped to grid*/
   TH2* hBeamAccChargeXSlice;
   TH2* hBeamAccChargeYSlice;

   /* xy projections of current mapped to grid*/
  TH2* hBeamCurrentXSlice;
  TH2* hBeamCurrentYSlice;
  /* xy projections of average current mapped to grid*/
  TH2* hBeamAveCurrentXSlice;
  TH2* hBeamAveCurrentYSlice;


  /* xy projections loopwise profiles*/
  TH1* hBeamLoopX;
  TH1* hBeamLoopY;

  /* xy projections loopwise profiles accum*/
  TH1* hBeamAccLoopX;
  TH1* hBeamAccLoopY;

  /* xy counts projections loopwise mapped to grid absolute positions (mm)*/
  TH1* hPosLoopX;
  TH1* hPosLoopY;

  /* xy counts projections loopwise accumulatedmapped to grid absolute positions (mm)*/
  TH1* hPosAccLoopX;
  TH1* hPosAccLoopY;

  /* xy charge projections loopwise mapped to grid absolute positions (mm)*/
  TH1* hPosQLoopX;
  TH1* hPosQLoopY;

  /* xy charge projections loopwise accumulatedmapped to grid absolute positions (mm)*/
  TH1* hPosQAccLoopX;
  TH1* hPosQAccLoopY;

  /* xy current projections loopwise mapped to grid absolute positions (mm)*/
  TH1* hPosILoopX;
  TH1* hPosILoopY;

  /* xy current projections loopwise accumulated average, mapped to grid absolute positions (mm)*/
  TH1* hPosIAveLoopX;
  TH1* hPosIAveLoopY;

  TH1* hBeamMeanCountsX;
  TH1* hBeamMeanCountsY;
  TH1* hBeamRMSCountsX;
  TH1* hBeamRMSCountsY;

  TGo4WinCond* cBeamXSliceCond;
  TGo4WinCond* cBeamYSliceCond;

  /* reference to the grid event object that we display*/
  TQFWGrid* fGridData;

  /* reference to setup parameters*/
  TQFWProfileParam* fParam;

ClassDef(TQFWGridLoopDisplay,1)
};

/********************************
 * Histogram container for each grid
 *  (JAM December 2013)
 *
 * */
class TQFWGridDisplay: public TQFWDisplay
{

public:
  TQFWGridDisplay() :
      TQFWDisplay(), hBeamX(0), hBeamY(0), hBeamAccX(0), hBeamAccY(0), hPosX(0), hPosY(0), hPosAccX(0), hPosAccY(0),
      hPosQ_X(0), hPosQ_Y(0),hPosQAcc_X(0),hPosQAcc_Y(0), hPosI_X(0), hPosI_Y(0), hPosIAve_X(0), hPosIAve_Y(0),
      pBeamProfiles(0), pPosProfiles(0), pChargeProfiles(0), pCurrentProfiles(0),
      hBeamMeanXY(0), hBeamRMSX(0), hBeamRMSY(0), hPosMeanXY(0), hPosRMSX(0), hPosRMSY(0),
      pBeamRMS(0), pPosRMS(0), fGridData(0), fParam(0)
  {
    ;
  }

  TQFWGridDisplay(Int_t grid);
  virtual ~TQFWGridDisplay();

  /* access to histogram set for current loop id*/
  TQFWGridLoopDisplay* GetLoopDisplay(Int_t index);

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* put some information into histogram headers*/
  void AdjustDisplay(TQFWBoard* data);

  /* create link between this display and the event structure for same unique id*/
  void SetGridEvent(TQFWProfileEvent* out);

  /*  create link between this display with subdisplays and setup parameter*/
  void SetProfileParam(TQFWProfileParam* par);

  /* xy projections of scalers mapped to grid*/
  TH1* hBeamX;
  TH1* hBeamY;

  TH1* hBeamAccX;
  TH1* hBeamAccY;

  /* xy projections of scalers mapped to grid absolute positions (mm)*/
  TH1* hPosX;
  TH1* hPosY;

  TH1* hPosAccX;
  TH1* hPosAccY;

  /* xy charge projections mapped to grid absolute positions (mm)*/
  TH1* hPosQ_X;
  TH1* hPosQ_Y;

  /* xy charge projections accumulated mapped to grid absolute positions (mm)*/
  TH1* hPosQAcc_X;
  TH1* hPosQAcc_Y;

  /* xy current projections mapped to grid absolute positions (mm)*/
  TH1* hPosI_X;
  TH1* hPosI_Y;

  /* xy current projections accumulated average, mapped to grid absolute positions (mm)*/
  TH1* hPosIAve_X;
  TH1* hPosIAve_Y;

  TGo4Picture *pBeamProfiles;
  TGo4Picture *pPosProfiles;
  TGo4Picture *pChargeProfiles;
  TGo4Picture *pCurrentProfiles;


  TH2* hBeamMeanXY;
  TH1* hBeamRMSX;
  TH1* hBeamRMSY;

  TH2* hPosMeanXY;
  TH1* hPosRMSX;
  TH1* hPosRMSY;

  TGo4Picture *pBeamRMS;
  TGo4Picture *pPosRMS;

  /* reference to the grid event object that we display*/
  TQFWGrid* fGridData;

  /* reference to setup parameters*/
  TQFWProfileParam* fParam;

ClassDef(TQFWGridDisplay,1)
};

/********************************
 * Histogram container for a single time loop of a Faraday Cup
 *  (JAM December 2013)
 *
 * */
class TQFWCupLoopDisplay: public TQFWLoopDisplay
{

public:
  TQFWCupLoopDisplay() :
      TQFWLoopDisplay(), fCupData(0), hCupSlice(0), hCupSliceOffs(0), hAccCupSlice(0),
      hCupChargeSlice(0), hCupAccChargeSlice(0),hCupCurrentSlice(0), hCupAveCurrentSlice(0), hCupLoopScaler(0),
          hCupAccLoopScaler(0), hCupLoopCharge(0), hCupAccLoopCharge(0), hCupLoopCurrent(0), hCupAveLoopCurrent(0)
  {
    ;
  }
  TQFWCupLoopDisplay(Int_t cupid, Int_t loopid);
  virtual ~TQFWCupLoopDisplay();

  /* create link between this display and the event structure for same unique id*/
  void SetCupEvent(TQFWProfileEvent* out);

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* put some information into histogram headers*/
  void AdjustDisplay(TQFWLoop* data);

  /* reference to the grid event object that we display*/
  TQFWCup* fCupData;

  TH2* hCupSlice;

  /* helper histogram showing current offset*/
  TH2* hCupSliceOffs;

  TH2* hAccCupSlice;

  TH2* hCupChargeSlice;
  TH2* hCupAccChargeSlice;

  TH2* hCupCurrentSlice;
  TH2* hCupAveCurrentSlice;


  TH1* hCupLoopScaler;
  TH1* hCupAccLoopScaler;

  TH1* hCupLoopCharge;
  TH1* hCupAccLoopCharge;

  TH1* hCupLoopCurrent;
  TH1* hCupAveLoopCurrent;

  /* keep histograms of current ratios of all cup segments against first (reference) segment
   * this can be used as indicator for beam position in case of segmented capacitor monitor*/
  std::vector<TH1*> hSegmentRatio;

  /* keep histograms of current ratios of all cup segments against first (reference) segment
   * this can be used as indicator for beam position in case of segmented capacitor monitor*/
  std::vector<TH1*> hAccSegmentRatio;

ClassDef(TQFWCupLoopDisplay,1)
};

/********************************
 * Histogram container for each Faraday Cup
 *  (JAM December 2013)
 *
 * */
class TQFWCupDisplay: public TQFWDisplay
{

public:
  TQFWCupDisplay() :
      TQFWDisplay(), hCupScaler(0), hCupAccScaler(0), fCupData(0)
  {
    ;
  }
  TQFWCupDisplay(Int_t cupid);
  virtual ~TQFWCupDisplay();

  /* create link between this display and the event structure for same unique id*/
  void SetCupEvent(TQFWProfileEvent* out);

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace = kFALSE);

  /* put some information into histogram headers*/
  void AdjustDisplay(TQFWBoard* data);

  TQFWCupLoopDisplay* GetLoopDisplay(Int_t index);

  /* each bin is one cup segment*/
  TH1* hCupScaler;
  TH1* hCupAccScaler;

  /* reference to the grid event object that we display*/
  TQFWCup* fCupData;

ClassDef(TQFWCupDisplay,1)
};

#endif //TQFWDISPLAY_H
