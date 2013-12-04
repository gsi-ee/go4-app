// $Id: TQFWRawParam.cxx 557 2010-01-27 15:11:43Z linev $
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

#include "TQFWRawParam.h"
//#include "Riostream.h"
#include "TGo4Log.h"

TQFWRawParam::TQFWRawParam(const char* name) :
   TGo4Parameter(name),
   fSimpleCompensation(kFALSE),
   fMeasureBackground(kFALSE),
   fCorrectBackground(kTRUE)
{
	//ResetOffsetMeasurement();
}


void TQFWRawParam::InitBoardMapping()
{
   // init to non valid here:
   for (int sfp = 0; sfp < PEXOR_MAXSFP; ++sfp)
         {
         for (int dev = 0; dev < PEXOR_MAXDEVS; ++dev)
               {
                  fBoardID[sfp][dev]=-1;
               }
         }

   fBoardID[0][0]=1; // test case: one qfw board at sfp0
}

Bool_t TQFWRawParam::SetConfigBoards()
{
   TQFWRawEvent::fgConfigQFWBoards.clear();
   for (int sfp = 0; sfp < PEXOR_MAXSFP; ++sfp)
            {
            for (int dev = 0; dev < PEXOR_MAXDEVS; ++dev)
                  {
                     Int_t bid=fBoardID[sfp][dev];
                     if(bid<0) continue;
                     TQFWRawEvent::fgConfigQFWBoards.push_back(bid);
                     TGo4Log::Info("TQFWRawParam::SetConfigBoards registers board unique id %u configured at sfp %d dev %d\n",bid,sfp,dev);
                  }
            }
   return kTRUE;
}





//Bool_t TQFWRawParam::AddOffsetMeasurement(int brd, int qfw, int ch, int ts, UInt_t count)
//{
//	if(!CheckQFWBoundaries(brd,qfw,ch,ts))
//		{
//			cout << "**** TQFWRawParam::AddOffsetMeasurement has illegal indices - ("<<brd<<","<<qfw<<","<<ch<<","<<ts<<")" << endl;
//			return kFALSE;
//		}
////	fMeasurementCounts[brd][qfw][ch][ts]++;
////	fQFWOffsetSums[brd][qfw][ch][ts]+=count;
////	fQFWOffsets[brd][qfw][ch][ts]=fQFWOffsetSums[brd][qfw][ch][ts] / fMeasurementCounts[brd][qfw][ch][ts];
//
//return kTRUE;
//}

//void TQFWRawParam::ResetOffsetMeasurement()
//{
//	for (int brd = 0; brd < OPTIC_QFWBOARDS; ++brd)
//		{
//		for (int qfw = 0; qfw < OPTIC_QFWNUM; ++qfw)
//				{
//					for (int ch = 0; ch < OPTIC_QFWCHANS; ++ch)
//					{
//						for (int sl = 0; sl < OPTIC_QFWSLICES; ++sl)
//						{
//							fQFWOffsets[brd][qfw][ch][sl] = 0;
//							fQFWOffsetSums[brd][qfw][ch][sl] = 0;
//							fMeasurementCounts[brd][qfw][ch][sl] = 0;
//						}
//					}
//				}
//		}
//}

//Int_t TQFWRawParam::GetCorrectedQFWValue(int brd, int qfw, int ch, int ts, Int_t count)
//{
//	if(!fCorrectBackground) return count;
//	if(fMeasureBackground) return count; // no correction evaluation during measurement
//	if(!CheckQFWBoundaries(brd,qfw,ch,ts))
//	{
//		cout << "**** TQFWRawParam::GetCorrectedQFWValue has illegal indices - ("<<brd<<","<<qfw<<","<<ch<<","<<ts<<")" << endl;
//		return count;
//	}
//	return (count - fQFWOffsets[brd][qfw][ch][ts]);
//
//}
//
//Bool_t TQFWRawParam::CheckQFWBoundaries(int brd, int qfw, int ch, int ts)
//{
//	if((brd<0) || (qfw<0) || (ch < 0) || (ts < 0)) return kFALSE;
//	if((brd<OPTIC_QFWBOARDS) && (qfw<OPTIC_QFWNUM) && (ch < OPTIC_QFWCHANS) && (ts < OPTIC_QFWSLICES)) return kTRUE;
//	return kFALSE;
//}


Bool_t TQFWRawParam::UpdateFrom(TGo4Parameter *pp)
{
	TQFWRawParam* from = dynamic_cast<TQFWRawParam*> (pp);
   if (from==0) {
      cout << "Wrong parameter object: " << pp->ClassName() << endl;
      return kFALSE;
   }

   cout << "**** TQFWRawParam::UpdateFrom ... " << endl;
   // automatic reset if we newly start the offset calibration:
   if(!fMeasureBackground && from->fMeasureBackground)
   {
	   cout << "TQFWRawParam::UpdateFrom is resetting background measurement!!!"<< endl;
	   //ResetOffsetMeasurement();
   }
   fMeasureBackground=from->fMeasureBackground;
   fCorrectBackground=from->fCorrectBackground;
   fSlowMotionStart=from->fSlowMotionStart;

   return kTRUE;

}
