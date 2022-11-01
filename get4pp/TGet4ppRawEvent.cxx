#include "TGet4ppRawEvent.h"

#include "TGo4Log.h"

//************************************************************************//

TGet4ppBoard::TGet4ppBoard() :
    TGo4EventElement(), fUniqueId(0)
{

}
TGet4ppBoard::TGet4ppBoard(const char* name, UInt_t unid, Short_t index) :
    TGo4EventElement(name, name, index), fUniqueId(unid)
{
  TGo4Log::Info("TGet4ppBoard: Create instance %s with unique id: %d, index: %d", name, unid, index);
  for (Int_t ch = 0; ch < Get4pp_CHANNELS; ++ch)
  {
    fMessages[ch].clear();
  }
}

TGet4ppBoard::~TGet4ppBoard()
{

}

void TGet4ppBoard::AddMessage(TGet4ppMsg* msg, UChar_t channel)
{
  //printf("TGet4ppBoard::AddMessage %d\n",channel);
  if (channel < Get4pp_CHANNELS)
  {
    fMessages[channel].push_back(msg);
  }
}


UInt_t TGet4ppBoard::NumMessages(UChar_t channel)
{
  if (channel >= Get4pp_CHANNELS)
    return 0;
  return fMessages[channel].size();
}

TGet4ppMsg* TGet4ppBoard::GetMessage(UChar_t channel, UInt_t i)
{
  if (channel >= Get4pp_CHANNELS)
    return 0;
  if (i >= fMessages[channel].size())
    return 0;
  return fMessages[channel].at(i);
}



void TGet4ppBoard::Clear(Option_t *t)
{
  //printf("TGet4ppBoard::Clear");
  for (Int_t ch = 0; ch < Get4pp_CHANNELS; ++ch)
  {

    for (unsigned i = 0; i < fMessages[ch].size(); ++i)
    {
      delete fMessages[ch].at(i);
    }
    fMessages[ch].clear();
  }

}

//***********************************************************

std::vector<UInt_t> TGet4ppRawEvent::fgConfigGet4ppBoards;

TGet4ppRawEvent::TGet4ppRawEvent() :
    TGo4CompositeEvent(),
    fSequenceNumber(-1), fVULOMStatus(0), fDataCount(0)
#ifdef Get4pp_DOFINETIMSAMPLES
    ,fTapConfig(0), fDelayConfig(0), fShiftChannel(0), fShiftDelay(0)
#endif
{
}
//***********************************************************
TGet4ppRawEvent::TGet4ppRawEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id), //#
    fSequenceNumber(-1), fVULOMStatus(0), fDataCount(0)
#ifdef Get4pp_DOFINETIMSAMPLES
    , fTapConfig(0), fDelayConfig(0), fShiftChannel(0), fShiftDelay(0)
#endif
{
  TGo4Log::Info("TGet4ppRawEvent: Create instance %s with composite id %d", name, id);
  TString modname;
  UInt_t uniqueid;
  for (unsigned i = 0; i < TGet4ppRawEvent::fgConfigGet4ppBoards.size(); ++i)
  {
    uniqueid = TGet4ppRawEvent::fgConfigGet4ppBoards[i];
    modname.Form("Get4pp_Board_%02d", uniqueid);
    addEventElement(new TGet4ppBoard(modname.Data(), uniqueid, i));
  }
}
//***********************************************************
TGet4ppRawEvent::~TGet4ppRawEvent()
{
}

TGet4ppBoard* TGet4ppRawEvent::GetBoard(UInt_t id)
{
  TGet4ppBoard* theBoard = 0;
  Short_t numBoards = getNElements();
  for (int i = 0; i < numBoards; ++i)
  {
    theBoard = (TGet4ppBoard*) getEventElement(i);
    if (theBoard->GetBoardId() == id)
      return theBoard;

  }
  return 0;
}

void TGet4ppRawEvent::Clear(Option_t *t)
{
  //TGo4Log::Info("TGet4ppRawEvent: Clear ");
  TGo4CompositeEvent::Clear();
  fSequenceNumber = -1;
  fVULOMStatus = 0;
  fDataCount = 0;
#ifdef Get4pp_DOFINETIMSAMPLES
  // clear fine time sample elements here
  SetValid(kFALSE);
  fTapConfig=0;
  fDelayConfig=0;
  fShiftChannel=0;
  fShiftDelay=0;
  //fLmdFileName="nofile"; // we keep most recent filename
  for(int i=0; i<Get4pp_CHANNELS; ++i)
  {
    for(int j=0; j<Get4pp_FINERANGE; ++j)
    {
      fFineTimeBinLeading[i][j]=-1;
      fFineTimeBinTrailing[i][j]=-1;
    }
    for(int k=0; k<Get4pp_CHANNELS; ++k)
    {
      fDeltaTimeLeadingMean[i][k]=0;
      fDeltaTimeLeadingSigma[i][k]=0;
    }
  }
#endif


}
