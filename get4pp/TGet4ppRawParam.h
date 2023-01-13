
#ifndef TGet4ppRawParam_H
#define TGet4ppRawParam_H

#include "TGo4Parameter.h"
#include "TGet4ppRawEvent.h"

#define Get4pp_FILTERCOEFFS 5


class TGet4ppRawParam : public TGo4Parameter {
   public:

    enum FilterTypes
    {
      FIL_NONE = 0,
      FIL_VONHANN = 1,
      FIL_HAMMING = 2,
      FIL_BLACKMAN = 3,
      FIL_NUTALL = 4,
      FIL_BLACKNUTALL = 5,
      FIL_BLACKHARRIS = 6,
      FIL_SRSFLATTOP = 7,
      FIL_USER = 8
    };


      TGet4ppRawParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Int_t fBoardID[Get4pp_MAXBOARDS]; // mapping of board numbers to (slot?) indices


      Int_t fVerbosity; // degree of debug printouts. 0 - mostly silent, 1- show data problems, 2 - dump all data words
      Bool_t fSlowMotion; // if true than only process one MBS event and stop.

      Bool_t fUseSoftwareCalibration; // if true, evaluate delta times with software calibration of fine time bins
      Bool_t fResetCalibration; // set true to begin new software calibration procedure
      Int_t  fCalibrationCounts; // minimum required statistics for each channel (leading/trailing) to complete calibration

#ifdef Get4pp_DOFINETIMSAMPLES
      Int_t fFineTimeSampleBoard; // id of board for fine time evaluation
      Int_t fFineTimeStatsLimit; // number of event threshold for writing fine times
#endif
      /* initial mapping of board id to sfp/ gosip device cabling*/
      void InitBoardMapping();

      /* copy board configuration from this parameter to static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetConfigBoards();

   private:



   ClassDef(TGet4ppRawParam,1)
};

#endif // TGet4ppRawParam_H
