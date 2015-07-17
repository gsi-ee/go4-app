#include "THitDetRawEvent.h"

#include "TGo4Log.h"

//************************************************************************//

THitDetBoard::THitDetBoard() :
    TGo4EventElement(), fUniqueId(0)
{

}
THitDetBoard::THitDetBoard(const char* name, UInt_t unid, Short_t index) :
    TGo4EventElement(name, name, index), fUniqueId(unid)
{
  TGo4Log::Info("THitDetBoard: Create instance %s with unique id: %d, index: %d", name, unid, index);
  for (Int_t ch = 0; ch < HitDet_CHANNELS; ++ch)
  {
    fMessages[ch].clear();
  }
}

THitDetBoard::~THitDetBoard()
{

}

void THitDetBoard::AddMessage(THitDetMsg* msg, UChar_t channel)
{
  //printf("THitDetBoard::AddMessage %d\n",channel);
  if (channel < HitDet_CHANNELS)
  {
    fMessages[channel].push_back(msg);
  }
}

void THitDetBoard::Clear(Option_t *t)
{
  //printf("THitDetBoard::Clear");
  for (Int_t ch = 0; ch < HitDet_CHANNELS; ++ch)
  {

    for (unsigned i = 0; i < fMessages[ch].size(); ++i)
    {
      delete fMessages[ch].at(i);
    }
    fMessages[ch].clear();
  }

}

//***********************************************************

std::vector<UInt_t> THitDetRawEvent::fgConfigHitDetBoards;

THitDetRawEvent::THitDetRawEvent() :
    TGo4CompositeEvent(), fSequenceNumber(-1), fVULOMStatus(0), fDataCount(0)
{
}
//***********************************************************
THitDetRawEvent::THitDetRawEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id), fSequenceNumber(-1), fVULOMStatus(0), fDataCount(0)
{
  TGo4Log::Info("THitDetRawEvent: Create instance %s with composite id %d", name, id);
  TString modname;
  UInt_t uniqueid;
  for (unsigned i = 0; i < THitDetRawEvent::fgConfigHitDetBoards.size(); ++i)
  {
    uniqueid = THitDetRawEvent::fgConfigHitDetBoards[i];
    modname.Form("HitDet_Board_%02d", uniqueid);
    addEventElement(new THitDetBoard(modname.Data(), uniqueid, i));
  }

}
//***********************************************************
THitDetRawEvent::~THitDetRawEvent()
{
}

THitDetBoard* THitDetRawEvent::GetBoard(UInt_t id)
{
  THitDetBoard* theBoard = 0;
  Short_t numBoards = getNElements();
  for (int i = 0; i < numBoards; ++i)
  {
    theBoard = (THitDetBoard*) getEventElement(i);
    if (theBoard->GetBoardId() == id)
      return theBoard;

  }
  return 0;
}

void THitDetRawEvent::Clear(Option_t *t)
{
  //TGo4Log::Info("THitDetRawEvent: Clear ");
  TGo4CompositeEvent::Clear();
  fSequenceNumber = -1;
  fVULOMStatus = 0;
  fDataCount = 0;
}
