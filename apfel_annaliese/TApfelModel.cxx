// $Id: TModelTemplate.cxx 478 2009-10-29 12:26:09Z linev $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

/* template for user model class
   The only function, which should be redefined by used - UserFunction()
   In given example constructor provides a general interface to create
   several parameters. Amplitude parameters can be add, if necessary.
   Any kind of other constructors with predefine parameters set and names can be invented
   Here user function is Gaussian
*/


#include "TApfelModel.h"

