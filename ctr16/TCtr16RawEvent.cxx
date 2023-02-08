#include "TCtr16RawEvent.h"

#include "TGo4Log.h"

//************************************************************************//

TCtr16Board::TCtr16Board() :
    TGo4EventElement(), fCurrentTraceEvent(0), fTracedataIndex(0), fTracesize12bit(0), fTracesize32bit (0),
    fToBeContinued(kFALSE), fUniqueId(0)
{

}
TCtr16Board::TCtr16Board(const char* name, UInt_t unid, Short_t index) :
    TGo4EventElement(name, name, index),fCurrentTraceEvent(0), fTracedataIndex(0), fTracesize12bit(0),
    fTracesize32bit (0), fToBeContinued(kFALSE), fUniqueId(unid)
{
  TGo4Log::Info("TCtr16Board: Create instance %s with unique id: %d, index: %d", name, unid, index);
  for (Int_t ch = 0; ch < Ctr16_CHANNELS; ++ch)
  {
    fMessages[ch].clear();
  }
}

TCtr16Board::~TCtr16Board()
{

}

void TCtr16Board::AddMessage(TCtr16Msg* msg, UChar_t channel)
{
  //printf("TCtr16Board::AddMessage %d\n",channel);
  if (channel < Ctr16_CHANNELS)
  {
    fMessages[channel].push_back(msg);
  }
}

void TCtr16Board::Clear(Option_t *t)
{
  //printf("TCtr16Board::Clear");
  for (Int_t ch = 0; ch < Ctr16_CHANNELS; ++ch)
  {

    for (unsigned i = 0; i < fMessages[ch].size(); ++i)
    {
      delete fMessages[ch].at(i);
    }
    fMessages[ch].clear();
  }

}

//***********************************************************

std::vector<UInt_t> TCtr16RawEvent::fgConfigCtr16Boards;

TCtr16RawEvent::TCtr16RawEvent() :
    TGo4CompositeEvent(), fSequenceNumber(-1), fVULOMStatus(0), fDataCount(0)
{
}
//***********************************************************
TCtr16RawEvent::TCtr16RawEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id), fSequenceNumber(-1), fVULOMStatus(0), fDataCount(0)
{
  TGo4Log::Info("TCtr16RawEvent: Create instance %s with composite id %d", name, id);
  TString modname;
  UInt_t uniqueid;
  for (unsigned i = 0; i < TCtr16RawEvent::fgConfigCtr16Boards.size(); ++i)
  {
    uniqueid = TCtr16RawEvent::fgConfigCtr16Boards[i];
    modname.Form("Ctr16_Board_%02d", uniqueid);
    addEventElement(new TCtr16Board(modname.Data(), uniqueid, i));
  }

}
//***********************************************************
TCtr16RawEvent::~TCtr16RawEvent()
{
}

TCtr16Board* TCtr16RawEvent::GetBoard(UInt_t id)
{
  TCtr16Board* theBoard = 0;
  Short_t numBoards = getNElements();
  for (int i = 0; i < numBoards; ++i)
  {
    theBoard = (TCtr16Board*) getEventElement(i);
    if (theBoard->GetBoardId() == id)
      return theBoard;

  }
  return 0;
}

void TCtr16RawEvent::Clear(Option_t *t)
{
  //TGo4Log::Info("TCtr16RawEvent: Clear ");
  TGo4CompositeEvent::Clear();
  fSequenceNumber = -1;
  fVULOMStatus = 0;
  fDataCount = 0;
}
