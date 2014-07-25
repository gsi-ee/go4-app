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

  for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
  {
    fQfwTrace[ch].clear();
    //TGo4Log::Info("TQFWLoop: Clear for ch: %d", ch);
  }
}


     Double_t TQFWLoop::GetCoulombPerCount()
     {
       Double_t CperCount=0;
       switch (fQfwSetup)
        {
          case 0:
            CperCount = -0.25 * 1e-12;
            break;
          case 1:
            CperCount = -2.5 * 1e-12;
            break;
          case 2:
            CperCount = 0.25 * 1e-12;
            break;
          case 3:
            CperCount = 2.5 * 1e-12;
            break;

          case 0x10:
            CperCount = -0.25 * 1e-12;
            break;
          case 0x11:
            CperCount = -2.5 * 1e-12;
            break;
          case 0x12:
            CperCount = 0.25 * 1e-12;
            break;
          case 0x13:
            CperCount = 2.5 * 1e-12;
            break;
          default:
            break;

        };

       return CperCount;
     }

     Double_t TQFWLoop::GetMicroSecsPerTimeSlice()
     {
        return (fQfwLoopTime * 20 / 1000);
     }

     TString TQFWLoop::GetSetupString()
     {
       return TQFWBoard::GetSetupString(fQfwSetup);
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
  // initialize frontend offsets:
  for (int c = 0; c < PEXOR_QFWCHANS; ++c)
     {
        fQfwOffsets[c]=0;
     }
}

TQFWBoard::~TQFWBoard()
{

}

void TQFWBoard::Clear(Option_t *t)
{
  //TGo4Log::Info("TQFWBoard: Clear ");
  TGo4CompositeEvent::Clear();
  fQfwSetup=0;

  for (int q = 0; q < PEXOR_QFWNUM; ++q)
   {
      fQfwErr[q]=0;
   }
}

TString TQFWBoard::GetSetupString()
{
  return TQFWBoard::GetSetupString(fQfwSetup);
}


TString TQFWBoard::GetSetupString(UChar_t qfwset)
  {
    /* evaluate measurement setup*/
    TString setup;
    switch (qfwset)
    {
      case 0:
        setup.Form("(-) [ 2.5pF & 0.25pC]");
        break;

      case 1:
        setup.Form("(-) [25.0pF & 2.50pC]");
        break;

      case 2:
        setup.Form("(+) [ 2.5pF & 0.25pC]");
        break;

      case 3:
        setup.Form("(+) [25.0pF & 2.50pC]");
        break;

      case 0x10:
        setup.Form("1000uA (-) [ 2.5pF & 0.25pC]");
        break;

      case 0x11:
        setup.Form("1000uA (-) [25.0pF & 2.50pC]");
        break;

      case 0x12:
        setup.Form("1000uA (+) [ 2.5pF & 0.25pC]");
        break;

      case 0x13:
        setup.Form("1000uA (+) [25.0pF & 2.50pC]");
        break;

      default:
        setup.Form("unknown setup %d", qfwset);
        break;
    };

    return setup;
  }





//***********************************************************

std::vector<UInt_t> TQFWRawEvent::fgConfigQFWBoards;


TQFWRawEvent::TQFWRawEvent() :
    TGo4CompositeEvent(), fSequenceNumber(-1)
{
}
//***********************************************************
TQFWRawEvent::TQFWRawEvent(const char* name, Short_t id) :
    TGo4CompositeEvent(name, name, id),fSequenceNumber(-1)
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

void TQFWRawEvent::Clear(Option_t *t)
{
  //TGo4Log::Info("TQFWRawEvent: Clear ");
    TGo4CompositeEvent::Clear();
    fSequenceNumber=-1;
}
