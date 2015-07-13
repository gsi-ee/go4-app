
#ifndef THitDetRawParam_H
#define THitDetRawParam_H

#include "TGo4Parameter.h"
#include "THitDetRawEvent.h"

class THitDetRawParam : public TGo4Parameter {
   public:
      THitDetRawParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Int_t fBoardID[HitDet_MAXBOARDS]; // mapping of board numbers to (slot?) indices

      Int_t fNumSnapshots; // number of trace snapshots per mbs event
      Int_t fTraceLength; // length of trace to visualize (8, 16, 32)




      /* initial mapping of board id to sfp/ gosip device cabling*/
      void InitBoardMapping();

      /* copy board configuration from this parameter to static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetConfigBoards();

   private:



   ClassDef(THitDetRawParam,1)
};

#endif // THitDetRawParam_H
