// $Id: TAwagsSisUnpackEvent.cxx 2627 2019-10-01 08:02:45Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TAwagsSisBasicEvent.h"

void TAwagsSisBasicEvent::Clear(Option_t *t)
{
  if(!IsValid()) return;
  fSignalTrace.clear();
  for (int i = 0; i < MAX_SFP; i++)
  {
    for (int j = 0; j < MAX_SLAVE; j++)
    {
      for (int k = 0; k < N_CHA; k++)
      {
#ifdef AWAGS_STORE_TRACES
        fSpillTrace[i][j][k].clear();
#endif
        fChargeTrend[i][j][k].clear();
      }
    }
  }
}
