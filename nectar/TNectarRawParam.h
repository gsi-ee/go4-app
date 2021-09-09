
#ifndef TNectarRawParam_H
#define TNectarRawParam_H

#include "TGo4Parameter.h"
#include "TNectarRawEvent.h"



class TNectarRawParam : public TGo4Parameter {
   public:

      TNectarRawParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Int_t fVMMR_BoardID[VMMR_MAXBOARDS]; // mapping of VMMR board numbers to (slot?) indices
      Int_t fMDPP_BoardID[MDPP_MAXBOARDS]; // mapping of MDPP board numbers to (slot?) indices

      Int_t fMDPP_ReferenceChannel[VMMR_MAXBOARDS]; // TDC reference channel for DeltaT of each MDPP board

      Bool_t fSlowMotion; // if true than only process one MBS event and stop.

      /* initial mapping of board id to sfp/ gosip device cabling*/
      void InitBoardMapping();

      /* copy board configuration from this parameter to static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetConfigBoards();

   private:



   ClassDef(TNectarRawParam,3)
};

#endif // TNectarRawParam_H
