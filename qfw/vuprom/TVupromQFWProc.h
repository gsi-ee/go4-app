#ifndef TUNPACKPROCESSOR_H
#define TUNPACKPROCESSOR_H

/* VUPROM QFW beam monitor
 * Version 0.9 on 3-Nov-2010 by J.Adamczewski-Musch, GSI
 * V 0.92 25-Jan-2010 with mean/sigma calculation of counts by J.Adamczewski-Musch, GSI
 * */


class TVupromQFWParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include "TVupromQFWEvent.h"


class TVupromQFWProc : public TGo4EventProcessor
{
public:
	TVupromQFWProc();
	TVupromQFWProc(const char* name);
	virtual ~TVupromQFWProc();

	Bool_t BuildEvent(TGo4EventElement* target); // event processing function

protected:

	/* here the geometry of the grids is set*/
	void MapGrids();

	/* use raw qfw data to fill the xy display histograms*/
	void FillGrids(TVupromQFWEvent* out);

	/* recreate histograms using the given number of time slice*/
	void InitDisplay(UInt_t timeslices, Bool_t replace=kFALSE);

	/* parameter for runtime settings*/
	 TVupromQFWParam* fPar;



private:
	/* Display qfw scaler accumulated channels in one histo per qfw
	 * First bins for prescaler, next bins for scaler, last bins for error counts
	 * */
	TH1* hVupromQFW[VUPROM_QFWBOARDS][VUPROM_QFWNUM];

	/* Display qfw scaler trace channels (from current event) in one histo per qfw
	 * First bins for prescaler, next bins for scaler, last bins for error counts
	 * */
	TH1* hVupromQFWTrace[VUPROM_QFWBOARDS][VUPROM_QFWNUM];

	TGo4Picture *pVupromQfws[VUPROM_QFWBOARDS];
	TGo4Picture *pVupromQfwsTrace[VUPROM_QFWBOARDS];

	TH1* hVupromSetup;
	TH1* hVupromPreMtime;
	TH1* hVupromMtime;

	/* xy projections of scalers mapped to grid*/
	TH1* hBeamX[VUPROM_QFWBOARDS];
	TH1* hBeamY[VUPROM_QFWBOARDS];
	TH2* hBeamXSlice[VUPROM_QFWBOARDS];
	TH2* hBeamYSlice[VUPROM_QFWBOARDS];

	/* helper histogram showing current offset*/
	TH2* hBeamXSliceOffs[VUPROM_QFWBOARDS];
	TH2* hBeamYSliceOffs[VUPROM_QFWBOARDS];


	TH1* hBeamPreX[VUPROM_QFWBOARDS];
	TH1* hBeamPreY[VUPROM_QFWBOARDS];
	TH1* hBeamAccX[VUPROM_QFWBOARDS];
	TH1* hBeamAccY[VUPROM_QFWBOARDS];
	TH2* hBeamAccXSlice[VUPROM_QFWBOARDS];
	TH2* hBeamAccYSlice[VUPROM_QFWBOARDS];

	TH1* hBeamPreAccX[VUPROM_QFWBOARDS];
	TH1* hBeamPreAccY[VUPROM_QFWBOARDS];

	TGo4Picture *pBeamProfiles[VUPROM_QFWBOARDS];

	TH2* hBeamMeanXY[VUPROM_QFWGRIDS];
	TH2* hBeamPreMeanXY[VUPROM_QFWGRIDS];
	TH1* hBeamRMSX[VUPROM_QFWGRIDS];
	TH1* hBeamRMSY[VUPROM_QFWGRIDS];
	TH1* hBeamPreRMSX[VUPROM_QFWGRIDS];
	TH1* hBeamPreRMSY[VUPROM_QFWGRIDS];


	TGo4Picture *pBeamRMS[VUPROM_QFWGRIDS];


	TGo4WinCond* cBeamXSliceCond[VUPROM_QFWGRIDS];
	TGo4WinCond* cBeamYSliceCond[VUPROM_QFWGRIDS];
	TH1* hBeamMeanCountsX[VUPROM_QFWGRIDS];
	TH1* hBeamMeanCountsY[VUPROM_QFWGRIDS];
	TH1* hBeamRMSCountsX[VUPROM_QFWGRIDS];
	TH1* hBeamRMSCountsY[VUPROM_QFWGRIDS];

	/* display the vulom scalers:*/
	TH1 *hVulomScaler[VULOM_SCALERNUM];
	TH1 *hVulomDeadTime;
	TH1 *hVulomFC;
	TGo4Picture *pVulomScalers;

	/* bin mapping for beam profile x coordinate*/
	int gBeamX[VUPROM_QFWGRIDS][VUPROM_QFWBOARDS][VUPROM_QFWNUM][VUPROM_QFWCHANS];

	/* bin mapping for beam profile y coordinate*/
	int gBeamY[VUPROM_QFWGRIDS][VUPROM_QFWBOARDS][VUPROM_QFWNUM][VUPROM_QFWCHANS];

	/* number of wires in x direction for grid of index*/
	int gBeamWiresX[VUPROM_QFWGRIDS];

	/* number of wires in y direction for grid of index*/
	int gBeamWiresY[VUPROM_QFWGRIDS];


	ClassDef(TVupromQFWProc,1)
};

#endif //TUNPACKPROCESSOR_H
