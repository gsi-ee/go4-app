#include "TQFWRawEvent.h"


#include "TGo4Log.h"

/*
 * This class represents one single  timescan loop in a qfw dataset
 *
 */


TQFWLoop::TQFWLoop() :
    TGo4EventElement()
{

}

TQFWLoop::TQFWLoop(const char* name, Short_t ix) :
    TGo4EventElement(name, name, ix)
{

}

TQFWLoop::~TQFWLoop()
{

}

void TQFWLoop::Clear(Option_t *t)
{
  fQfwLoopSize=0;
  fQfwLoopTime=0;
  for (int ch = 0; ch > PEXOR_QFWCHANS; ++ch)
  {
    fQfwTrace[ch].clear();
  }
}

//************************************************************************//

TQFWBoard::TQFWBoard() :
    TGo4CompositeEvent()
{

}
TQFWBoard::TQFWBoard(const char* name, UInt_t unid, Short_t index) :
   TGo4CompositeEvent(name, name, index), fUniqueId(unid)
{
  TGo4Log::Info("TQFWBoard: Create instance %s with unique id: %d, index: %d", name, unid, index);
  TString modname;
  for (int i = 0; i < PEXOR_QFWLOOPS; ++i)
  {
    modname.Form("Board%02d_Loop%02d", fUniqueId, i);
    addEventElement(new TQFWLoop(modname.Data(), i));
  }

}

TQFWBoard::~TQFWBoard()
{

}

void TQFWBoard::Clear(Option_t *t)
{


}

//***********************************************************
TQFWRawEvent::TQFWRawEvent() :
    TGo4CompositeEvent()
{
}
//***********************************************************
TQFWRawEvent::TQFWRawEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
  TGo4Log::Info("TQFWRawEvent: Create instance %s with composite id %d", name, id);
  TString modname;
  UInt_t uniqueid;
  for (unsigned i = 0; i < TQFWRawEvent::fgConfigQFWBoards.size(); ++i)
  {
    uniqueid = TQFWRawEvent::fgConfigQFWBoards[i];
    modname.Form("QFW_Board_%02d", uniqueid);
    addEventElement(new TQFWBoard(modname.Data(), uniqueid, i));
  }

}
//***********************************************************
TQFWRawEvent::~TQFWRawEvent()
{
}



TQFWBoard* TQFWRawEvent::GetBoard(UInt_t id)
{
  TQFWBoard* theBoard=0;
  Short_t numBoards=getNElements();
  for (int i = 0; i < numBoards ; ++i)
   {
    theBoard= (TQFWBoard*) getEventElement(i);
    if(theBoard->GetBoardId()==id) return theBoard;

   }
  return 0;
}



