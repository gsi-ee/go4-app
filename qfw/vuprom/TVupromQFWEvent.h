#ifndef TQFWEVENT_H
#define TQFWEVENT_H

#include "TGo4EventElement.h"

#define VUPROM_QFWBOARDS 2
#define VUPROM_QFWGRIDS 2

#define VUPROM_QFWNUM 8
#define VUPROM_QFWCHANS 4

#define VUPROM_QFWSLICES 200
/* this is now maximum number of slices*/


#define VULOM_SCALERNUM 5
#define VULOM_SCALERCHANS 16

#define DEADTIME_BINS 4096
#define FC_BINS 4096

class TVupromQFWEvent : public TGo4EventElement {
   public:
      TVupromQFWEvent();
      TVupromQFWEvent(const char* name);
      virtual ~TVupromQFWEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");

	/* vulom scaler values*/
	UInt_t fVulomScaler[VULOM_SCALERNUM][VULOM_SCALERCHANS];
	
	
	UInt_t fFC_VulomScaler;
	
	UInt_t fFC_VulomDeadTime;

	/* vuprom QFW scaler values*/
	UInt_t fQfw[VUPROM_QFWBOARDS][VUPROM_QFWNUM][VUPROM_QFWCHANS][VUPROM_QFWSLICES];

	/* Measured vuprom QFW scaler offsets. for correction */
	UInt_t fQfwOffsets[VUPROM_QFWBOARDS][VUPROM_QFWNUM][VUPROM_QFWCHANS][VUPROM_QFWSLICES];

	/* vuprom QFW prescaler values*/
	UInt_t fQfwPre[VUPROM_QFWBOARDS][VUPROM_QFWNUM][VUPROM_QFWCHANS];

	/* vuprom QFW error count values*/
	UInt_t fQfwErr[VUPROM_QFWBOARDS][VUPROM_QFWNUM][VUPROM_QFWCHANS];

	/* setup number (only one byte used!)*/
	UChar_t fQfwSetup;

	/* qfw prescaler time range (20ns/unit)*/
	UInt_t fQfwPreMTime;

	/* qfw scaler time range (20ns/unit)*/
	UInt_t fQfwMTime;

	/* the actual number of time slices used*/
	UInt_t fQfwNumTimeSlices;


	/* mean beam position from current x profile*/
	Double_t fBeamMeanX[VUPROM_QFWGRIDS];

	/* mean beam position from current y profile*/
	Double_t fBeamMeanY[VUPROM_QFWGRIDS];

	/* beam rms width from current x profile*/
	Double_t fBeamRMSX[VUPROM_QFWGRIDS];

	/* beam rms width from current y profile*/
	Double_t fBeamRMSY[VUPROM_QFWGRIDS];

	/* mean beam position from current x profile prescaler*/
	Double_t fBeamPreMeanX[VUPROM_QFWGRIDS];

	/* mean beam position from current y profile prescaler*/
	Double_t fBeamPreMeanY[VUPROM_QFWGRIDS];

	/* beam rms width from current x profile prescaler*/
	Double_t fBeamPreRMSX[VUPROM_QFWGRIDS];

	/* beam rms width from current y profile prescaler*/
	Double_t fBeamPreRMSY[VUPROM_QFWGRIDS];


   ClassDef(TVupromQFWEvent,3)
};
#endif //TEVENT_H



