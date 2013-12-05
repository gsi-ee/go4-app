
#ifndef TQFWProfileParam_H
#define TQFWProfileParam_H

#include "TGo4Parameter.h"

#include "TQFWRawEvent.h"
#include "TQFWProfileEvent.h"






class TQFWProfileParam : public TGo4Parameter {
   public:

      TQFWProfileParam();

      TQFWProfileParam(const char* name);

      virtual Bool_t UpdateFrom(TGo4Parameter *);

      Bool_t   fMeasureBackground;  // switches background measurement on/off
      Bool_t   fCorrectBackground;  // apply background correction to qfw on/off

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


      Double_t fQFWOffsetsX[PEXOR_QFW_GRIDS][PEXOR_QFWLOOPS][PEXOR_QFW_WIRES]; // average offset values (grid,loop,wire)
      Int_t fQFWOffsetXSums[PEXOR_QFW_GRIDS][PEXOR_QFWLOOPS][PEXOR_QFW_WIRES]; // accumulated offset values (grid,loop,wire)
      Int_t fMeasurementCountsX[PEXOR_QFW_GRIDS][PEXOR_QFWLOOPS][PEXOR_QFW_WIRES]; // counts offset values (grid,wire,loop)

      Double_t fQFWOffsetsY[PEXOR_QFW_GRIDS][PEXOR_QFWLOOPS][PEXOR_QFW_WIRES]; // average offset values (grid,wire,loop)
      Int_t fQFWOffsetYSums[PEXOR_QFW_GRIDS][PEXOR_QFWLOOPS][PEXOR_QFW_WIRES]; // accumulated offset values (grid,loop,wire)
      Int_t fMeasurementCountsY[PEXOR_QFW_GRIDS][PEXOR_QFWLOOPS][PEXOR_QFW_WIRES]; // counts offset values (grid,wire,loop)






      /* initial mapping of board/channels to grid/cup display*/
      void InitProfileMapping();

      /* reference this parameter with the grid/cup setup at static event members
       * before initialization. Usually this is done in UpdateFrom*/
      Bool_t SetEventConfig();

  /* new measurement point for x direction wire*/
      Bool_t  AddXOffsetMeasurement(Int_t grid, Int_t loop, Int_t wire, UInt_t value);

  /* new measurement point for x direction wire*/
      Bool_t AddYOffsetMeasurement(Int_t grid, Int_t loop, Int_t wire, UInt_t value);

//      /* Reset values of offset measurement to zero.*/
  void ResetOffsetMeasurement();

  /* Evaluate offset correction from the previously measured data*/
  Double_t GetCorrectedXValue(Int_t grid, Int_t loop, Int_t wire, UInt_t count);

  /* Evaluate offset correction from the previously measured data*/
  Double_t GetCorrectedYValue(Int_t grid, Int_t loop, Int_t wire, UInt_t count);

  Bool_t CheckGridBoundaries(Int_t grid, Int_t loop, Int_t wire);

   private:



   ClassDef(TQFWProfileParam,1)
};

#endif // TQFWProfileParam_H
