
#include "TVupromQFWEvent.h"

#include "Riostream.h"

//***********************************************************
TVupromQFWEvent::TVupromQFWEvent() :
   TGo4EventElement()
{
    cout << "**** TVupromQFWEvent: Create instance" << endl;
    fQfwNumTimeSlices=0;
}
//***********************************************************
TVupromQFWEvent::TVupromQFWEvent(const char* name) :
   TGo4EventElement(name)
{
  cout << "**** TVupromQFWEvent: Create instance " << name << endl;
  fQfwNumTimeSlices=0;
}
//***********************************************************
TVupromQFWEvent::~TVupromQFWEvent()
{
  cout << "**** TVupromQFWEvent: Delete instance " << endl;
}

//-----------------------------------------------------------
void  TVupromQFWEvent::Clear(Option_t *t)
{
	fFC_VulomScaler=0;
	fFC_VulomDeadTime=0;
	for(int sc=0; sc<VULOM_SCALERNUM;++sc)
	{
		for(int ch=0; ch<VULOM_SCALERCHANS;++ch)
			{
				fVulomScaler[sc][ch]=0;
			}
	}

	fQfwSetup=0;
	fQfwPreMTime=0;
	fQfwMTime=0;
	//fQfwNumTimeSlices=0; // do not reset number of time slices

	for (int qboard = 0; qboard < VUPROM_QFWBOARDS; ++qboard)
		{
			for (int qfw = 0; qfw < VUPROM_QFWNUM; ++qfw)
			{
				for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
				{
					for (int sl = 0; sl < VUPROM_QFWSLICES; ++sl)
					{
						fQfw[qboard][qfw][ch][sl] = 0;
						fQfwOffsets[qboard][qfw][ch][sl] = 0;
					}// time slice
					fQfwPre[qboard][qfw][ch] = 0;
					fQfwErr[qboard][qfw][ch] = 0;
				}// channel
			}// qfw


}// board

	for (int grid = 0; grid < VUPROM_QFWGRIDS; ++grid)
			{
				 fBeamMeanY[grid]=0.;
				 fBeamRMSX[grid]=0.;
				 fBeamRMSY[grid]=0.;
				 fBeamPreMeanX[grid]=0.;
				 fBeamPreMeanY[grid]=0.;
				 fBeamPreRMSX[grid]=0.;
				 fBeamPreRMSY[grid]=0.;

			}// grid

}
