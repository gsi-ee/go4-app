
#ifndef TOpticQFWParam_H
#define TOpticQFWParam_H

#include "TGo4Parameter.h"
#include "TOpticQFWEvent.h"

class TOpticQFWParam : public TGo4Parameter {
   public:
      TOpticQFWParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);



      Bool_t   fMeasureBackground;  // switches background measurement on/off
      Bool_t   fCorrectBackground;  // apply background correction to qfw on/off

      Int_t fQFWOffsets[OPTIC_QFWBOARDS][OPTIC_QFWNUM][OPTIC_QFWCHANS][OPTIC_QFWSLICES]; // average offset values (qfw,chan,tslice)

      Int_t fQFWOffsetSums[OPTIC_QFWBOARDS][OPTIC_QFWNUM][OPTIC_QFWCHANS][OPTIC_QFWSLICES]; // accumulated offset values (qfw,chan,tslice)


      UInt_t fMeasurementCounts[OPTIC_QFWBOARDS][OPTIC_QFWNUM][OPTIC_QFWCHANS][OPTIC_QFWSLICES]; // number of measurements for (qfw,chan,tslice)

	UInt_t fSlowMotionStart; // event number in file to begin slow control mode (0=off)

      /* add new measured offset counts for next event. The QFWOffsets for specified coordinate
       * will be evaluated from this as an average of all calls of this method. */
      Bool_t AddOffsetMeasurement(int brd, int qfw, int ch, int ts, UInt_t count);

      /* Reset values of offset measurement to zero.*/
      void ResetOffsetMeasurement();

      /* Evaluate offset correction from the previously measured data*/
      Int_t GetCorrectedQFWValue(int brd, int qfw, int ch, int ts, Int_t count);

      /* are we inside allowed array range?*/
      Bool_t CheckQFWBoundaries(int brd, int qfw, int ch, int ts);

   private:



   ClassDef(TOpticQFWParam,1)
};

#endif // TOpticQFWParam_H
