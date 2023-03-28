
#ifndef TCtr16RawParam_H
#define TCtr16RawParam_H

#include "TGo4Parameter.h"
#include "TCtr16RawEvent.h"


class TCtr16RawParam : public TGo4Parameter {
   public:

      TCtr16RawParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);


      Int_t fBoardID[Ctr16_MAXSFP][Ctr16_MAXDEVS]; // mapping of board numbers to pexor chains

      Int_t fNumSnapshots; // number of trace snapshots per mbs event
      Int_t fTraceLength; // length of trace to visualize (16, 32, 64)
      Bool_t fDoCalibrate; // switch on calibrate mode for adc correction
      Bool_t fSlowMotion; // if true than only process one MBS event and stop.
      Int_t fStopAtEvent; // mbs event sequence number to stop at.
      Int_t fVerbosity; // degree of debug printouts. 0 - mostly silent, 1- show data problems, 2 - dump all data words


#ifdef Ctr16_DO_MEMORYSAMPLES
      Int_t fMemorySampleBoard; // id of board for adc memory sample evaluation
      Int_t fMemorySampleStatsLimit; // number of event threshold for writing memory cells ADC values
#endif


      /* initial mapping of board id to sfp/ gosip device cabling*/
      void InitBoardMapping();

      /* copy board configuration from this parameter to static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetConfigBoards();

   private:



   ClassDef(TCtr16RawParam,3)
};

#endif // TCtr16RawParam_H
