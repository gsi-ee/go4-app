//-------------------------------------------------------------
//        Go4 Release Package v3.03-05 (build 30305)
//                      05-June-2008
//---------------------------------------------------------------
//   The GSI Online Offline Object Oriented (Go4) Project
//   Experiment Data Processing at EE department, GSI
//---------------------------------------------------------------
//
//Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
//                    Planckstr. 1, 64291 Darmstadt, Germany
//Contact:            http://go4.gsi.de
//----------------------------------------------------------------
//This software can be used under the license agreements as stated
//in Go4License.txt file which is part of the distribution.
//----------------------------------------------------------------
#ifndef SPAR_H
#define SPAR_H

#define __ARRAYSIZE__ 100
#define __MEVX__ 5
#define __MEVY__ 10


// maximum number of sfp readout chains used:
#define PEXOR_MAXSFP 4
// maximum number of devices per sfpchain:
#define PEXOR_MAXDEVS 10

#define PEXOR_APFEL_GRIDS 2

/* default number of wires for first init:*/
#define PEXOR_APFEL_WIRES 16


#include "TGo4Parameter.h"

class TFeb3BasicParam : public TGo4Parameter {
   public:
      TFeb3BasicParam();
      TFeb3BasicParam(const char* name);
      virtual ~TFeb3BasicParam();

      void InitMapping();
      void FindBoardIndices(Int_t boardid, Int_t &ch, Int_t &slave);
      Bool_t fDoGridMapping; // switch beam profile grid mapping on or off

      Int_t fBoardID[PEXOR_MAXSFP][PEXOR_MAXDEVS]; // mapping of board numbers to pexor chains

      Int_t fNumGrids; // number of grid devices
      Int_t fGridDeviceID[PEXOR_APFEL_GRIDS]; // unique beam profile grid id
      Int_t fGridBoardID_X[PEXOR_APFEL_GRIDS][PEXOR_APFEL_WIRES]; // febex board id for [grid,wireX]
      Int_t fGridBoardID_Y[PEXOR_APFEL_GRIDS][PEXOR_APFEL_WIRES]; // febex board id for [grid,wireY]
      Int_t fGridChannel_X[PEXOR_APFEL_GRIDS][PEXOR_APFEL_WIRES];  // febex channel on board for [grid,wireX]
      Int_t fGridChannel_Y[PEXOR_APFEL_GRIDS][PEXOR_APFEL_WIRES];  // febex channel on board for [grid,wireY]






   ClassDef(TFeb3BasicParam,1)
};

#endif //SPAR_H

//----------------------------END OF GO4 SOURCE FILE ---------------------
