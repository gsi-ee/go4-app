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
#include "TGemCSABasicParam.h"

//#include "Riostream.h"
//using namespace std;

//***********************************************************
TGemCSABasicParam::TGemCSABasicParam() : TGo4Parameter("Parameter")
{
  fDoBaselineFits=kFALSE;
  fMapTraces=kFALSE;

}
//***********************************************************
TGemCSABasicParam::TGemCSABasicParam(const char* name) : TGo4Parameter(name)
{
  fDoBaselineFits=kFALSE;
  fMapTraces=kFALSE;

}
//***********************************************************
TGemCSABasicParam::~TGemCSABasicParam()
{
}
//***********************************************************




//----------------------------END OF GO4 SOURCE FILE ---------------------
