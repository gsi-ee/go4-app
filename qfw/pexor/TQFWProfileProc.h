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

public:
  TQFWProfileProc();
  TQFWProfileProc(const char* name);
  virtual ~TQFWProfileProc();

  Bool_t BuildEvent(TGo4EventElement* target);    // event processing function

protected:


   /* use raw qfw data to fill the xy display histograms*/
 //  void FillGrids();



  /* here the geometry of the grids is set*/
  void MapGrids();

  /* use raw qfw data to fill the xy display histograms*/
//  void FillGrids(TQFWRawEvent* out);

  /* recreate histograms using the given number of time slice*/
  void InitDisplay(int timeslices, Bool_t replace = kFALSE);

  /* parameter for runtime settings*/
//  TQFWProfileParam* fPar;

private:

//  TGo4Picture *pPexorQfws[OPTIC_QFWBOARDS];
//  TGo4Picture *pPexorQfwsTrace[OPTIC_QFWBOARDS];

  /* xy projections of scalers mapped to grid*/
//  TH1* hBeamX[OPTIC_QFWBOARDS];
//  TH1* hBeamY[OPTIC_QFWBOARDS];
//  TH2* hBeamXSlice[OPTIC_QFWBOARDS];
//  TH2* hBeamYSlice[OPTIC_QFWBOARDS];
//
//  /* helper histogram showing current offset*/
//  TH2* hBeamXSliceOffs[OPTIC_QFWBOARDS];
//  TH2* hBeamYSliceOffs[OPTIC_QFWBOARDS];
//
//  TH1* hBeamAccX[OPTIC_QFWBOARDS];
//  TH1* hBeamAccY[OPTIC_QFWBOARDS];
//  TH2* hBeamAccXSlice[OPTIC_QFWBOARDS];
//  TH2* hBeamAccYSlice[OPTIC_QFWBOARDS];
//
//  TGo4Picture *pBeamProfiles[OPTIC_QFWBOARDS];
//
//  TH2* hBeamMeanXY[OPTIC_QFWGRIDS];
//  TH1* hBeamRMSX[OPTIC_QFWGRIDS];
//  TH1* hBeamRMSY[OPTIC_QFWGRIDS];
//
//  TGo4Picture *pBeamRMS[OPTIC_QFWGRIDS];
//
//  TGo4WinCond* cBeamXSliceCond[OPTIC_QFWGRIDS];
//  TGo4WinCond* cBeamYSliceCond[OPTIC_QFWGRIDS];
//  TH1* hBeamMeanCountsX[OPTIC_QFWGRIDS];
//  TH1* hBeamMeanCountsY[OPTIC_QFWGRIDS];
//  TH1* hBeamRMSCountsX[OPTIC_QFWGRIDS];
//  TH1* hBeamRMSCountsY[OPTIC_QFWGRIDS];
//
//  /* bin mapping for beam profile x coordinate*/
//  int gBeamX[OPTIC_QFWGRIDS][OPTIC_QFWBOARDS][OPTIC_QFWNUM][OPTIC_QFWCHANS];
//
//  /* bin mapping for beam profile y coordinate*/
//  int gBeamY[OPTIC_QFWGRIDS][OPTIC_QFWBOARDS][OPTIC_QFWNUM][OPTIC_QFWCHANS];
//
//  /* number of wires in x direction for grid of index*/
//  int gBeamWiresX[OPTIC_QFWGRIDS];
//
//  /* number of wires in y direction for grid of index*/
//  int gBeamWiresY[OPTIC_QFWGRIDS];

  ClassDef(TQFWProfileProc,1)
};

#endif //TUNPACKPROCESSOR_H
