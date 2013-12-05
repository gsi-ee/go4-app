
#ifndef TQFWRawParam_H
#define TQFWRawParam_H

#include "TGo4Parameter.h"
#include "TQFWRawEvent.h"

class TQFWRawParam : public TGo4Parameter {
   public:
      TQFWRawParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Int_t fBoardID[PEXOR_MAXSFP][PEXOR_MAXDEVS]; // mapping of board numbers to pexor chains

      Bool_t   fSimpleCompensation; // if true, simple compensation with average per channel/series value will be done


	   UInt_t fSlowMotionStart; // event number in file to begin slow control mode (0=off)


      /* initial mapping of board id to sfp/ gosip device cabling*/
      void InitBoardMapping();

      /* copy board configuration from this parameter to static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetConfigBoards();

   private:



   ClassDef(TQFWRawParam,1)
};

#endif // TQFWRawParam_H
