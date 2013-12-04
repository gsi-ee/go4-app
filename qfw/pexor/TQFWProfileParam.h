
#ifndef TQFWProfileParam_H
#define TQFWProfileParam_H

#include "TGo4Parameter.h"

#include "TQFWRawEvent.h"
#include "TQFWProfileEvent.h"






class TQFWProfileParam : public TGo4Parameter {
   public:
      TQFWProfileParam(const char* name = 0);

      virtual Bool_t UpdateFrom(TGo4Parameter *);


      // configuration objects are here

      Int_t fNumCups; // number of cup devices
      Int_t fCupDeviceID[PEXOR_QFW_CUPS]; // unique faraday cup id
      Int_t fCupBoardID[PEXOR_QFW_CUPS][PEXOR_QFW_CUPSEGMENTS]; // qfw board id for [cup,segment]
      Int_t fCupChannel[PEXOR_QFW_CUPS][PEXOR_QFW_CUPSEGMENTS]; // qfw channel on board for [cup,segment]

      Int_t fNumGrids; // number of grid devices
      Int_t fGridDeviceID[PEXOR_QFW_GRIDS]; // unique beam profile grid id
      Int_t fGridBoardID_X[PEXOR_QFW_GRIDS][PEXOR_QFW_WIRES]; // qfw board id for [grid,wireX]
      Int_t fGridBoardID_Y[PEXOR_QFW_GRIDS][PEXOR_QFW_WIRES]; // qfw board id for [grid,wireY]
      Int_t fGridChannel_X[PEXOR_QFW_GRIDS][PEXOR_QFW_WIRES];  // qfw channel on board for [grid,wireX]
      Int_t fGridChannel_Y[PEXOR_QFW_GRIDS][PEXOR_QFW_WIRES];  // qfw channel on board for [grid,wireY]

      /* initial mapping of board/channels to grid/cup display*/
      void InitProfileMapping();

      /* reference this parameter with the grid/cup setup at static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetEventConfig();

   private:



   ClassDef(TQFWProfileParam,1)
};

#endif // TQFWProfileParam_H
