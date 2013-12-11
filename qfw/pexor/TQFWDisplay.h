#ifndef TQFWDISPLAY_H
#define TQFWDISPLAY_H


class TQFWRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "TQFWRawEvent.h"
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
  TQFWDisplay(): TGo4EventProcessor(),fDisplayId(-1)
    {;}

  TQFWDisplay(Int_t deviceid);

  virtual ~TQFWDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);

  /* unique ID number of device to display (board or grid)*/
  Int_t GetDevId(){return fDisplayId;}

  void AddLoopDisplay(TQFWLoopDisplay* loop)
  {
      fLoops.push_back(loop);
  }

  void ClearLoopDisplays(){fLoops.clear();}

  Int_t GetNumLoops()
  {
      return fLoops.size();
  }

  /* access to subdisplay set for loopid (if existing)*/
  TQFWLoopDisplay* GetSubDisplay(UInt_t loopid)
  {
    if(loopid<fLoops.size())
      return fLoops[loopid];
    return 0;
  }

   /* Evaluate description of measurement range from qfw setup value*/
   TString GetSetupString(UChar_t qfwsetup);

protected:




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
  TQFWLoopDisplay():TQFWDisplay(-1),fLoopId(-1),fTimeSlices(-1)
    {;}

  TQFWLoopDisplay(Int_t deviceid, Int_t loopid=-1);
  virtual ~TQFWLoopDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);


  /* id of time loop*/
  Int_t GetLoopId(){return fLoopId;}

  /* id of time loop*/
  Int_t GetTimeSlices(){return fTimeSlices;}




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
  TQFWBoardLoopDisplay():TQFWLoopDisplay(), hQFWRaw(0),hQFWRawTrace(0) {;}

  TQFWBoardLoopDisplay(Int_t boardid, Int_t loopid);
  virtual ~TQFWBoardLoopDisplay(){;}

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);


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
  TQFWBoardDisplay(): TQFWDisplay(), hQFWRaw2D(0), hQFWRaw2DTrace(0),
  hQFWRawErr(0),hQFWRawErrTr(0), pPexorQfws(0), pPexorQfwsTrace(0)
  {;}
  TQFWBoardDisplay(Int_t boardid);
  virtual ~TQFWBoardDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);

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
  TQFWGridLoopDisplay():TQFWLoopDisplay(), hBeamXSlice(0), hBeamYSlice(0),
  hBeamXSliceOffs(0), hBeamYSliceOffs(0), hBeamAccXSlice(0), hBeamAccYSlice(0), hBeamMeanCountsX(0),
  hBeamMeanCountsY(0), hBeamRMSCountsX(0), hBeamRMSCountsY(0),
  cBeamXSliceCond(0), cBeamYSliceCond(0), fGridData(0)
  {;}

  TQFWGridLoopDisplay(Int_t grid, Int_t loopid);
  virtual ~TQFWGridLoopDisplay();

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);

  /* TODO: put some information into histogram headers*/
   void AdjustDisplay(TQFWLoop* data);

  /* create link between this display and the event structure for same unique id*/
   void SetGridEvent(TQFWProfileEvent* out);

    /* xy projections of scalers mapped to grid*/
    TH2* hBeamXSlice;
    TH2* hBeamYSlice;

    /* helper histogram showing current offset*/
    TH2* hBeamXSliceOffs;
    TH2* hBeamYSliceOffs;

    TH2* hBeamAccXSlice;
    TH2* hBeamAccYSlice;

    TH1* hBeamMeanCountsX;
       TH1* hBeamMeanCountsY;
       TH1* hBeamRMSCountsX;
       TH1* hBeamRMSCountsY;


        TGo4WinCond* cBeamXSliceCond;
        TGo4WinCond* cBeamYSliceCond;

        /* reference to the grid event object that we display*/
            TQFWGrid* fGridData;

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
  TQFWGridDisplay():TQFWDisplay(),
  hBeamX(0),hBeamY(0), hBeamAccX(0), hBeamAccY(0), pBeamProfiles(0), hBeamMeanXY(0), hBeamRMSX(0), hBeamRMSY(0),
  pBeamRMS(0),fGridData(0)
  {;}

  TQFWGridDisplay(Int_t grid);
  virtual ~TQFWGridDisplay();


  /* access to histogram set for current loop id*/
    TQFWGridLoopDisplay* GetLoopDisplay(Int_t index);

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);


  /* put some information into histogram headers*/
  void AdjustDisplay(TQFWBoard* data);

  /* create link between this display and the event structure for same unique id*/
  void SetGridEvent(TQFWProfileEvent* out);





  /* xy projections of scalers mapped to grid*/
    TH1* hBeamX;
    TH1* hBeamY;


    TH1* hBeamAccX;
    TH1* hBeamAccY;

    TGo4Picture *pBeamProfiles;

    TH2* hBeamMeanXY;
    TH1* hBeamRMSX;
    TH1* hBeamRMSY;

    TGo4Picture *pBeamRMS;

    /* reference to the grid event object that we display*/
    TQFWGrid* fGridData;

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
  TQFWCupLoopDisplay(): TQFWLoopDisplay(),fCupData(0), hCupSlice(0), hCupSliceOffs(0), hAccCupSlice(0){;}
  TQFWCupLoopDisplay(Int_t cupid, Int_t loopid);
  virtual ~TQFWCupLoopDisplay();

  /* create link between this display and the event structure for same unique id*/
   void SetCupEvent(TQFWProfileEvent* out);

  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);

  /* put some information into histogram headers*/
   void AdjustDisplay(TQFWLoop* data);

  /* reference to the grid event object that we display*/
  TQFWCup* fCupData;


  TH2* hCupSlice;

     /* helper histogram showing current offset*/
     TH2* hCupSliceOffs;

     TH2* hAccCupSlice;

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
  TQFWCupDisplay():TQFWDisplay(),hCupScaler(0), hCupAccScaler(0), fCupData(0){;}
  TQFWCupDisplay(Int_t cupid);
  virtual ~TQFWCupDisplay();

  /* create link between this display and the event structure for same unique id*/
  void SetCupEvent(TQFWProfileEvent* out);


  /* recreate histograms using the given number of time slice*/
  virtual void InitDisplay(Int_t timeslices, Bool_t replace=kFALSE);

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
