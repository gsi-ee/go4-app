// $Id: TVupromQFWParam.cxx 557 2010-01-27 15:11:43Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TVupromQFWParam.h"
#include "Riostream.h"

TVupromQFWParam::TVupromQFWParam(const char* name) :
   TGo4Parameter(name), fMeasureBackground(kFALSE),fCorrectBackground(kTRUE)
{
	ResetOffsetMeasurement();


}


Bool_t TVupromQFWParam::AddOffsetMeasurement(int brd, int qfw, int ch, int ts, UInt_t count)
{
	if(!CheckQFWBoundaries(brd,qfw,ch,ts))
		{
			cout << "**** TVupromQFWParam::AddOffsetMeasurement has illegal indices - ("<<brd<<","<<qfw<<","<<ch<<","<<ts<<")" << endl;
			return kFALSE;
		}
	fMeasurementCounts[brd][qfw][ch][ts]++;
	fQFWOffsetSums[brd][qfw][ch][ts]+=count;
	fQFWOffsets[brd][qfw][ch][ts]=fQFWOffsetSums[brd][qfw][ch][ts] / fMeasurementCounts[brd][qfw][ch][ts];

return kTRUE;
}

void TVupromQFWParam::ResetOffsetMeasurement()
{
	for (int brd = 0; brd < VUPROM_QFWBOARDS; ++brd)
		{
		for (int qfw = 0; qfw < VUPROM_QFWNUM; ++qfw)
				{
					for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
					{
						for (int sl = 0; sl < VUPROM_QFWSLICES; ++sl)
						{
							fQFWOffsets[brd][qfw][ch][sl] = 0;
							fQFWOffsetSums[brd][qfw][ch][sl] = 0;
							fMeasurementCounts[brd][qfw][ch][sl] = 0;
						}
					}
				}
		}
}

Int_t TVupromQFWParam::GetCorrectedQFWValue(int brd, int qfw, int ch, int ts, Int_t count)
{
	if(!fCorrectBackground) return count;
	if(fMeasureBackground) return count; // no correction evaluation during measurement
	if(!CheckQFWBoundaries(brd,qfw,ch,ts))
	{
		cout << "**** TVupromQFWParam::GetCorrectedQFWValue has illegal indices - ("<<brd<<","<<qfw<<","<<ch<<","<<ts<<")" << endl;
		return count;
	}
	return (count - fQFWOffsets[brd][qfw][ch][ts]);

}

Bool_t TVupromQFWParam::CheckQFWBoundaries(int brd, int qfw, int ch, int ts)
{
	if((brd<0) || (qfw<0) || (ch < 0) || (ts < 0)) return kFALSE;
	if((brd<VUPROM_QFWBOARDS) && (qfw<VUPROM_QFWNUM) && (ch < VUPROM_QFWCHANS) && (ts < VUPROM_QFWSLICES)) return kTRUE;
	return kFALSE;
}


Bool_t TVupromQFWParam::UpdateFrom(TGo4Parameter *pp)
{
	TVupromQFWParam* from = dynamic_cast<TVupromQFWParam*> (pp);
   if (from==0) {
      cout << "Wrong parameter object: " << pp->ClassName() << endl;
      return kFALSE;
   }

   cout << "**** TVupromQFWParam::UpdateFrom ... " << endl;
   // automatic reset if we newly start the offset calibration:
   if(!fMeasureBackground && from->fMeasureBackground)
   {
	   cout << "TVupromQFWParam::UpdateFrom is resetting background measurement!!!"<< endl;
	   ResetOffsetMeasurement();
   }
   fMeasureBackground=from->fMeasureBackground;
   fCorrectBackground=from->fCorrectBackground;
   fSlowMotionStart=from->fSlowMotionStart;


// exclude this for the moment, is not handled correctly by go4 gui JAM
//   for (int qfw = 0; qfw < VUPROM_QFWNUM; ++qfw)
//   			{
//   				for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
//   				{
//   					for (int sl = 0; sl < VUPROM_QFWSLICES; ++sl)
//   					{
//   						fQFWOffsets[qfw][ch][sl] = from->fQFWOffsets[qfw][ch][sl];
//   						fQFWOffsetSums[qfw][ch][sl] = from->fQFWOffsetSums[qfw][ch][sl];
//   						fMeasurementCounts[qfw][ch][sl] = from->fMeasurementCounts[qfw][ch][sl];
//   					}
//   				}
//   			}

   return kTRUE;

}
