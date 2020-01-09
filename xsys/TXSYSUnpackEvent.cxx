
#include "TXSYSUnpackEvent.h"

//***********************************************************
TXSYSUnpackEvent::TXSYSUnpackEvent() :
   TGo4EventElement()
{
}
//***********************************************************
TXSYSUnpackEvent::TXSYSUnpackEvent(const char* name) :
   TGo4EventElement(name)
{
}
//***********************************************************
TXSYSUnpackEvent::~TXSYSUnpackEvent()
{
}
//***********************************************************

//-----------------------------------------------------------
void  TXSYSUnpackEvent::Clear(Option_t *t)
{
   void* destfield;
   //cout << "+++ event clear" << endl;
   fLen=0;
   fChannel=-1;
   fFlag=-1;
   destfield = (void*) &fSpectrum[0];
   memset(destfield, 0, sizeof(fSpectrum));
   destfield = (void*) &fSpecID[0];
   memset(destfield, 0, sizeof(fSpecID));
}
