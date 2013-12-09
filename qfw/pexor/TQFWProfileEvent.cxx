#include "TQFWProfileEvent.h"
#include "TQFWProfileParam.h"

#include "TGo4Log.h"
#include "TGo4UserException.h"




TQFWGrid::TQFWGrid(const char* name, UInt_t unid, Short_t index) :
    TGo4EventElement(name, name, index),fUniqueId(unid)
{
  TGo4Log::Info("TQFWGrid: Create instance %s with unique id: %d, index: %d", name, unid, index);
  ClearMappings();
  Clear();
}

TQFWGrid::~TQFWGrid()
{

}

void TQFWGrid::Clear(Option_t *t)
{
  fBeamMeanX = 0;
  fBeamMeanY = 0;
  fBeamRMSX = 0;
  fBeamRMSY = 0;
#ifdef QFW_STORECURRENTS
  fXCurrent.clear();
  fYCurrent.clear();
#endif
}



void TQFWGrid::AddXMapping(UInt_t board, Int_t channel)
{
  fXChannels.push_back(TQFWChannelMap(board, channel));
}

void TQFWGrid::AddYMapping(UInt_t board, Int_t channel)
{
  fYChannels.push_back(TQFWChannelMap(board, channel));
}

void TQFWGrid::ClearMappings()
{
  fXChannels.clear();
  fYChannels.clear();
}

/**************************************************************************/

TQFWCup::TQFWCup(const char* name, UInt_t unid, Short_t index) :
    TGo4EventElement(name, name, index) , fUniqueId(unid)
{
  TGo4Log::Info("TQFWCup: Create instance %s with unique id: %d, index: %d", name, unid, index);
  ClearMappings();
   Clear();
}

TQFWCup::~TQFWCup()
{

}

void TQFWCup::Clear(Option_t *t)
{
#ifdef QFW_STORECURRENTS
  fCurrent.clear();
#endif
}

void TQFWCup::AddMapping(UInt_t board, Int_t channel)
{
  fChannels.push_back(TQFWChannelMap(board, channel));
}


void TQFWCup::ClearMappings()
{
  fChannels.clear();
}





//***********************************************************

TQFWProfileParam* TQFWProfileEvent::fParameter=0;

TQFWProfileEvent::TQFWProfileEvent() :
    TGo4CompositeEvent()
{
}


TQFWProfileEvent::TQFWProfileEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id)
{
  TGo4Log::Info("TQFWProfileEvent: Create instance %s with composite id %d", name, id);
  SetupGrids();
  SetupCups();
}


void TQFWProfileEvent::SetupGrids()
{
  if(TQFWProfileEvent::fParameter==0)
    {
      TGo4Log::Warn("TQFWProfileEvent: setup grid subevents fails , no parameter assigned!");
      return; // better throw event exception here!
    }
  TString modname;
  UInt_t uniqueid;
  for (int i = 0; i < TQFWProfileEvent::fParameter->fNumGrids; ++i)
  {
    uniqueid = TQFWProfileEvent::fParameter->fGridDeviceID[i];
    modname.Form("QFW_Grid_%02d", uniqueid);
    TQFWGrid* grid = new TQFWGrid(modname.Data(), uniqueid, i);
    for (int wx = 0; wx < PEXOR_QFW_WIRES; ++wx)
    {
      Int_t board = TQFWProfileEvent::fParameter->fGridBoardID_X[i][wx];
      if (board < 0)
        continue;
      Int_t channel = TQFWProfileEvent::fParameter->fGridChannel_X[i][wx];
      //if(channel<0) // we keep negative channel numbers to mask out broken wires later
      grid->AddXMapping(board, channel);
    }
    for (int wy = 0; wy < PEXOR_QFW_WIRES; ++wy)
    {
      Int_t board = TQFWProfileEvent::fParameter->fGridBoardID_Y[i][wy];
      if (board < 0)
        continue;
      Int_t channel = TQFWProfileEvent::fParameter->fGridChannel_Y[i][wy];
      //if(channel<0) // we keep negative channel numbers to mask out broken wires later
      grid->AddYMapping(board, channel);
    }
    addEventElement(grid);
  }

}

void TQFWProfileEvent::SetupCups()
{
  if (TQFWProfileEvent::fParameter == 0)
  {
    TGo4Log::Warn("TQFWProfileEvent: setup cup subevents fails, no parameter assigned!");
    return;    // better throw event exception here!
  }

  TString modname;
  UInt_t uniqueid;
  for (int i = 0; i < TQFWProfileEvent::fParameter->fNumCups; ++i)
  {
    uniqueid = TQFWProfileEvent::fParameter->fCupDeviceID[i];
    modname.Form("QFW_Cup_%02d", uniqueid);
    TQFWCup* cup = new TQFWCup(modname.Data(), uniqueid, i);
    for (int seg = 0; seg < PEXOR_QFW_CUPSEGMENTS; ++seg)
    {
      Int_t board = TQFWProfileEvent::fParameter->fCupBoardID[i][seg];
      if (board < 0)
        continue;
      Int_t channel = TQFWProfileEvent::fParameter->fCupChannel[i][seg];
      //if(channel<0) // we keep negative channel numbers to mask out broken wires later
      cup->AddMapping(board, channel);
    }
    addEventElement(cup);
  }

}




TQFWProfileEvent::~TQFWProfileEvent()
{

}

//-----------------------------------------------------------
void TQFWProfileEvent::Clear(Option_t *t)
{

}




TQFWGrid* TQFWProfileEvent::GetGrid(UInt_t uniqueid)
{
  TQFWGrid* theGrid=0;
   Short_t numSubEvents=getNElements();
   for (int i = 0; i < numSubEvents ; ++i)
    {
     theGrid= dynamic_cast<TQFWGrid*> (getEventElement(i));
     if(theGrid==0) continue;
     if(theGrid->GetGridId()==uniqueid) return theGrid;
    }
   return 0;
}

TQFWCup* TQFWProfileEvent::GetCup(UInt_t uniqueid)
{
  TQFWCup* theCup=0;
    Short_t numSubEvents=getNElements();
    for (int i = 0; i < numSubEvents ; ++i)
     {
      theCup= dynamic_cast<TQFWCup*> (getEventElement(i));
      if(theCup==0) continue;
      if(theCup->GetCupId()==uniqueid) return theCup;
     }
    return 0;
}

