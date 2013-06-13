#ifndef TQFWEVENT_H
#define TQFWEVENT_H

#include "TGo4EventElement.h"

// total number of boards in analysis
#define OPTIC_QFWBOARDS 1

// number of loops
#define OPTIC_QFWLOOPS  3

// maximum number of slices in loop
#define OPTIC_QFWSLICES 200

// number of channels in one board
#define OPTIC_QFWCHANS  32
// number of QFW in one board
#define OPTIC_QFWNUM 8



#define OPTIC_QFWGRIDS 2



#define DEADTIME_BINS 4096
#define FC_BINS 4096

class TOpticQFWEvent : public TGo4EventElement {
   public:
      TOpticQFWEvent();
      TOpticQFWEvent(const char* name);
      virtual ~TOpticQFWEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");


      /* the actual number of time slices used*/
      Int_t fQfwLoopSize[OPTIC_QFWBOARDS][OPTIC_QFWLOOPS];

      /* the actual time of each loop */
      Int_t fQfwLoopTime[OPTIC_QFWBOARDS][OPTIC_QFWLOOPS];

	   /* vuprom QFW scaler values*/
	   UInt_t fQfw[OPTIC_QFWBOARDS][OPTIC_QFWLOOPS][OPTIC_QFWSLICES][OPTIC_QFWCHANS];

	   /* vuprom QFW error count values*/
	   UInt_t fQfwErr[OPTIC_QFWBOARDS][OPTIC_QFWNUM];


	   //  ======================== OLD DATA =============================

	   /* setup number (only one byte used!)*/
	   UChar_t fQfwSetup;

	   /* qfw scaler time range (20ns/unit)*/
	   UInt_t fQfwMTime;


	   /* mean beam position from current x profile*/
	   Double_t fBeamMeanX[OPTIC_QFWGRIDS];

	   /* mean beam position from current y profile*/
	   Double_t fBeamMeanY[OPTIC_QFWGRIDS];

	   /* beam rms width from current x profile*/
	   Double_t fBeamRMSX[OPTIC_QFWGRIDS];

	   /* beam rms width from current y profile*/
	   Double_t fBeamRMSY[OPTIC_QFWGRIDS];

	   ClassDef(TOpticQFWEvent,1)
};
#endif //TEVENT_H



