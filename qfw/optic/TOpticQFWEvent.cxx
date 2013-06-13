
#include "TOpticQFWEvent.h"

#include "TGo4Log.h"

//***********************************************************
TOpticQFWEvent::TOpticQFWEvent() :
   TGo4EventElement()
{
}
//***********************************************************
TOpticQFWEvent::TOpticQFWEvent(const char* name) :
   TGo4EventElement(name)
{
   TGo4Log::Info("TOpticQFWEvent: Create instance %s", name);
}
//***********************************************************
TOpticQFWEvent::~TOpticQFWEvent()
{
}

//-----------------------------------------------------------
void  TOpticQFWEvent::Clear(Option_t *t)
{

	for (int qboard = 0; qboard < OPTIC_QFWBOARDS; ++qboard)
	   for (int qfw = 0; qfw < OPTIC_QFWNUM; ++qfw)
	      fQfwErr[qboard][qfw] = 0;

   for (int qboard = 0; qboard < OPTIC_QFWBOARDS; ++qboard)
      for (int loop = 0; loop < OPTIC_QFWLOOPS; ++loop) {
         fQfwLoopSize[qboard][loop] = 0;
         fQfwLoopTime[qboard][loop] = 0;
         for (int sl = 0; sl < OPTIC_QFWSLICES; ++sl)
    	      for (int ch = 0; ch < OPTIC_QFWCHANS; ++ch)
					fQfw[qboard][loop][sl][ch] = 0;
      }

   fQfwSetup = 0;
   fQfwMTime = 0;

	for (int grid = 0; grid < OPTIC_QFWGRIDS; ++grid) {
	  fBeamMeanY[grid]=0.;
	  fBeamRMSX[grid]=0.;
	  fBeamRMSY[grid]=0.;
	}// grid

}
