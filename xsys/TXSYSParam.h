// $Id: TXSYSParam.h 557 2010-01-27 15:11:43Z linev $
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

#ifndef TXSYSParam_H
#define TXSYSParam_H

#include "TGo4Parameter.h"

class TXSYSParam : public TGo4Parameter {
   public:
      TXSYSParam(const char* name = 0);

      Bool_t   fillRaw;  // control filling of raw histograms

   ClassDef(TXSYSParam,1)
};

#endif // TXSYSParam_H
