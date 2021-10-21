#include "TNectarRawProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "TVirtualFFT.h"
#include "TGo4Analysis.h"
#include "TGo4Log.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"
#include "TGo4WinCond.h"

#include "TNectarRawEvent.h"
#include "TNectarRawParam.h"
#include "TNectarDisplay.h"

#include "TGo4Fitter.h"
#include "TGo4FitDataHistogram.h"
#include "TGo4FitParameter.h"
#include "TGo4FitModelPolynom.h"

#include "TGo4UserException.h"

/** enable this definition to print out event sample data explicitely*/
//#define NECTAR_VERBOSE_PRINT 1
#ifdef NECTAR_VERBOSE_PRINT
#define printdeb( args... )  printf( args );
#else
#define printdeb( args...) ;
#endif

static ULong_t skipped_events = 0;

/* helper macros for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/

#define  NECTAR_EVENT_CHECK_PDATA                                    \
if((pData - pSubevt->GetDataField()) > lWords ) \
{ \
  printf("############ unexpected end of event for subevent size :0x%x, skip event %ld\n", lWords, skipped_events++);\
  GO4_SKIP_EVENT \
  continue; \
}

//***********************************************************
TNectarRawProc::TNectarRawProc() :
    TGo4EventProcessor(), fPar(0), fNectarRawEvent(0), pSubevt(0), pData(0), lWords(0)
{
}

//***********************************************************
// this one is used in standard factory
TNectarRawProc::TNectarRawProc(const char* name) :
    TGo4EventProcessor(name), fPar(0), fNectarRawEvent(0), pSubevt(0), pData(0), lWords(0)
{
  TGo4Log::Info("TNectarRawProc: Create instance %s", name);
  fMdppDisplays.clear();
  fVmmrDisplays.clear();

  SetMakeWithAutosave(kTRUE);
//// init user analysis objects:

  fPar = dynamic_cast<TNectarRawParam*>(MakeParameter("NectarRawParam", "TNectarRawParam", "set_NectarRawParam.C"));
  if (fPar)
    fPar->SetConfigBoards();    // board configuration specified in parameter is used to build subevents and displays

  for (UInt_t i = 0; i < TNectarRawEvent::fgConfigMdppBoards.size(); ++i)
  {
    UInt_t uniqueid = TNectarRawEvent::fgConfigMdppBoards[i];
    Bool_t sixteenchannels = fPar->fMDPP_is16Channels[i];    // prepare displays for configured 16 or 32 channel MDPP
    fMdppDisplays.push_back(new TMdppDisplay(uniqueid, sixteenchannels));
  }

  for (UInt_t i = 0; i < TNectarRawEvent::fgConfigVmmrBoards.size(); ++i)
  {
    UInt_t uniqueid = TNectarRawEvent::fgConfigVmmrBoards[i];
    fVmmrDisplays.push_back(new TVmmrDisplay(uniqueid));
  }

  InitDisplay(false);    // will init all subdisplays, creating histograms etc.

}

//***********************************************************
TNectarRawProc::~TNectarRawProc()
{
  std::cout << "**** TNectarRawProc dtor " << std::endl;

  for (UInt_t i = 0; i < fMdppDisplays.size(); ++i)
  {
    delete fMdppDisplays[i];
  }
  for (UInt_t i = 0; i < fVmmrDisplays.size(); ++i)
  {
    delete fVmmrDisplays[i];
  }
}

TMdppDisplay* TNectarRawProc::GetMdppDisplay(UInt_t uniqueid)
{
  for (UInt_t i = 0; i < fMdppDisplays.size(); ++i)
  {
    TMdppDisplay* theDisplay = fMdppDisplays[i];
    if (uniqueid == theDisplay->GetDevId())
      return theDisplay;
  }
  return 0;
}

TVmmrDisplay* TNectarRawProc::GetVmmrDisplay(UInt_t uniqueid)
{
  for (UInt_t i = 0; i < fVmmrDisplays.size(); ++i)
  {
    TVmmrDisplay* theDisplay = fVmmrDisplays[i];
    if (uniqueid == theDisplay->GetDevId())
      return theDisplay;
  }
  return 0;
}

void TNectarRawProc::InitDisplay(Bool_t replace)
{
  std::cout << "**** TNectarRawProc: Init Display with replace= " << replace << std::endl;

  for (UInt_t i = 0; i < fMdppDisplays.size(); ++i)
  {
    fMdppDisplays[i]->InitDisplay(0, replace);
  }
  for (UInt_t i = 0; i < fVmmrDisplays.size(); ++i)
  {
    fVmmrDisplays[i]->InitDisplay(0, replace);
  }

}

//-----------------------------------------------------------
// event function
Bool_t TNectarRawProc::BuildEvent(TGo4EventElement* target)
{
// called by framework from TNectarRawEvent to fill it
  fNectarRawEvent = (TNectarRawEvent*) target;
  fNectarRawEvent->SetValid(kFALSE);    // not store
  TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
  if (source == 0)
  {
    std::cout << "AnlProc: no input event !" << std::endl;
    return kFALSE;
  }
  UShort_t triggertype = source->GetTrigger();

  if (triggertype > 11)
  {
    // frontend offset trigger can be one of these, we let it through to unpacking loop
    //cout << "**** TNectarRawProc: Skip trigger event" << endl;
    GO4_SKIP_EVENT_MESSAGE("**** TNectarRawProc: Skip event of trigger type 0x%x", triggertype);
    //return kFALSE; // this would let the second step execute!
  }

// first we fill the TNectarRawEvent with data from MBS source
// we have up to two subevents, crate 1 and 2
// Note that one has to loop over all subevents and select them by
// crate number:   pSubevt->GetSubcrate(),
// procid:         pSubevt->GetProcid(),
// and/or control: pSubevt->GetControl()
// here we use only crate number

//  take general event number from mbs event header. Note that subsystem sequence may differ:
  fNectarRawEvent->fSequenceNumber = source->GetCount();

  source->ResetIterator();
  while ((pSubevt = source->NextSubEvent()) != 0)
  {    // loop over subevents

    // JAM here one can exclude data from other subsystem by these id numbers
    // see mbs setup.usf how this is defined!
    if (pSubevt->GetSubcrate() != 0)
      continue;
    if (pSubevt->GetControl() != 9)
      continue;
//  if (pSubevt->GetProcid() != 1)
//    continue;

    pData = pSubevt->GetDataField();
    lWords = pSubevt->GetIntLen();

    if ((UInt_t) *pData == 0xbad00bad)
    {
      GO4_SKIP_EVENT_MESSAGE("**** TNectarRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*pData));
    }

    // loop over single subevent data:
    while (pData - pSubevt->GetDataField() < lWords)
    {

      // first header word indicates board type:
      // 0xA - VMMR, 0xB - MDPP
      // also module id and firmware version is given
      Int_t header = *pData++;
      Bool_t isVMMR = ((header >> 28) & 0xF) == 0xA;
      Bool_t isMDPP = ((header >> 28) & 0xF) == 0xB;
      //  JAM todo: use following heaeder info somewhere?
      //Int_t module_nr = (header >> 16) & 0xF;
      //Int_t firmware = header & 0xFFFF;

      if (isVMMR)
      {
        if (!UnpackVmmr())
        {
          GO4_SKIP_EVENT_MESSAGE("Data error: unpacking of VMMR data failed! -  skip event  NEVER COME HERE!");
          // at the moment, any error in UnpackMdpp will throw exception, so you should never see this line
        }
      }

      else if (isMDPP)
      {
        if (!UnpackMdpp())
        {
          GO4_SKIP_EVENT_MESSAGE("Data error: unpacking of MDPP data failed! -  skip event  NEVER COME HERE!");
          // at the moment, any error in UnpackMdpp will throw exception, so you should never see this line
        }
      }
      else
      {
        GO4_SKIP_EVENT_MESSAGE("Data error: header is neither VMMR nor MDPP - skip event!");
        return kFALSE;
      }

    }    // while pData - pSubevt->GetDataField() <lWords

  }    // while subevents

//

  UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
  fNectarRawEvent->SetValid(kTRUE);    // to store

  if (fPar->fSlowMotion)
  {
    Int_t evnum = source->GetCount();
    GO4_STOP_ANALYSIS_MESSAGE(
        "Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
  }

  return kTRUE;
}

Bool_t TNectarRawProc::UnpackMdpp()
{
// mdpp header should follow
  Int_t MDPP_head = *pData++;
  Bool_t isHeader = ((MDPP_head >> 28) & 0xF) == 0x4;
  if (!isHeader)
    GO4_SKIP_EVENT_MESSAGE("Data error: invalid mdpp header 0x%x -  skip event!", MDPP_head);
  Int_t sublen = MDPP_head & 0x3FF;    // number of following data words,including EOE
  UInt_t modid = (MDPP_head >> 16) & 0xFF;    //module id
  Int_t resolution = (MDPP_head >> 10) & 0x3F;    //3 bit TDC_resolution 0x604 , 3bit ADC_resolution 0x6046

// JAm 13-09-21: evaluate if MDPP has 16 or 32 channel format
  Bool_t mddpHas16Channels = kFALSE;
  Int_t ix = fPar->GetMDPPArrayindex(modid);
  if (ix >= 0)
    mddpHas16Channels = fPar->fMDPP_is16Channels[ix];

  TMdppBoard* theBoard = fNectarRawEvent->GetMdppBoard(modid);
  if (theBoard == 0)
  {
    GO4_STOP_ANALYSIS_MESSAGE(
        "Configuration error: MDPP module id %d does not exist as subevent. Please check TNectarRawParam setup", modid);
    return kFALSE;
  }
  theBoard->SetResolution(resolution);

  TMdppDisplay* boardDisplay = GetMdppDisplay(modid);
  if (boardDisplay == 0)
  {
    GO4_STOP_ANALYSIS_MESSAGE("Configuration error: MDPP module id %d does not exist as histogram display set!", modid);
    return kFALSE;
  }
  boardDisplay->ResetDisplay();

  boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_HEADER);    // always account header if found

// loop over following data words:
  Int_t* pdatastart = pData;    // remember begin of MDPP payload data section
  while ((pData - pdatastart) < sublen)
  {
    Int_t word = *pData++;
    Bool_t isTimestamp = ((word >> 28) & 0xF) == 0x2;
    Bool_t isData = ((word >> 28) & 0xF) == 0x1;
    Bool_t isEndmark = ((word >> 30) & 0x3) == 0x3;
    Bool_t isDummy = ((word >> 30) & 0x3) == 0x0;
    if (isTimestamp)
    {
      UShort_t ts = word & 0xFFFF;
      theBoard->fExtendedTimeStamp = ts;
      boardDisplay->hExtTimeStamp->Fill(ts);
      boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_TIMESTAMP);
    }
    else if (isData)
    {
      if (mddpHas16Channels)
      {
        // we expect MDPP 16 channel variant from configuration
        UShort_t maxchannel=16;
        UShort_t channel = (word >> 16) & 0x1F;
        UShort_t value = word & 0xFFFF;
        printdeb("UnpackMdpp 16 has data word 0x%x, channel=%d, value=0x%x\n", word, channel, value)
        Bool_t isTDC = channel > 15 ? kTRUE : kFALSE;
        if (isTDC)
        {
          channel -= 16;
          if (channel >= maxchannel)
          {
            printf("Warning: MDPP 16 TDC channel info %d out of bounds, maxchannel=%d\n", channel, maxchannel);
            continue;    // maybe better stop here ?
          }
          // put data to output event structure (tree file, next analysis steps...)
          theBoard->AddTdcMessage(new TMdppTdcData(value), channel);
          // now do histogramming:
          boardDisplay->hRawTDC[channel]->Fill(value);
          boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_TDC);
          boardDisplay->hTDC_ChannelScaler->Fill(channel);
        }
        else
        {
          // ADC:
          if (channel >= maxchannel)
          {
            printf("Warning: MDPP ADC channel info %d out of bounds, maxchannel=%d\n", channel, maxchannel);
            continue;    // maybe better stop here ?
          }

          theBoard->AddAdcMessage(new TMdppAdcData(value), channel);
          boardDisplay->hRawADC[channel]->Fill(value);
          boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_ADC);
          boardDisplay->hADC_ChannelScaler->Fill(channel);
        }

      }    //if(mddpHas16Channels)
      else
      {
        // we expect MDPP 32 channel variant from configuration
        UShort_t channel = (word >> 16) & 0x7F;
        UShort_t value = word & 0xFFFF;
        printdeb("UnpackMdpp 32 has data word 0x%x, channel=%d, value=0x%x\n", word, channel, value)
        Bool_t isTDC = channel > 31 ? kTRUE : kFALSE;
        if (isTDC)
        {
          // check here for extra trigger time difference information at highest channels:
          if (channel > 63)
          {
            UShort_t trigchan = channel - 64;
            if (trigchan >= MDPP_EXTDTCHANNELS)
            {
              printf("Warning: MDPP 32 external trigger input %d out of bounds, maxinputs=%d\n", trigchan, MDPP_EXTDTCHANNELS);
              continue;    // maybe better stop here ?
            }
            theBoard->AddExtDTMessage(new TMdppTdcData(value), trigchan);
            boardDisplay->hExtTrigTime[trigchan]->Fill(value);
          }

          else
          {
            // regular TDC
            channel -= 32;
            if (channel >= MDPP_CHANNELS)
            {
              printf("Warning: MDPP 32 TDC channel info %d out of bounds, maxchannel=%d\n", channel, MDPP_CHANNELS);
              continue;    // maybe better stop here ?
            }
            // put data to output event structure (tree file, next analysis steps...)
            theBoard->AddTdcMessage(new TMdppTdcData(value), channel);
            // now do histogramming:
            boardDisplay->hRawTDC[channel]->Fill(value);
            boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_TDC);
            boardDisplay->hTDC_ChannelScaler->Fill(channel);
          }
        }
        else
        {
          // ADC
          if (channel >= MDPP_CHANNELS)
          {
            printf("Warning: MDPP ADC channel info %d out of bounds, maxchannel=%d\n", channel, MDPP_CHANNELS);
            continue;    // maybe better stop here ?
          }
          theBoard->AddAdcMessage(new TMdppAdcData(value), channel);
          boardDisplay->hRawADC[channel]->Fill(value);
          boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_ADC);
          boardDisplay->hADC_ChannelScaler->Fill(channel);
        }

      }    //if(mddpHas16Channels)
    }
    else if (isEndmark)
    {
      Int_t count = (word & 0x3FFFFFFF);
      theBoard->fEventCounter = count;
      boardDisplay->hMsgTypes->Fill(TMdppMsg::MSG_EOE);

      // now check against global counter of all subsystems:
      if (fNectarRawEvent->fSequenceNumber < 0)
      {
        fNectarRawEvent->fSequenceNumber = count;    // init if not yet set by other board
        printdeb("Warning: MDPP board %d setting sequence number to %d \n", modid, count);

        // TODO: maybe we could put MBS header counter to fSequenceNumber, but this may differ by special trigger events
      }
      else
      {
        if (fNectarRawEvent->fSequenceNumber != count)    // check against global counter
        {
          printdeb("Warning: MDPP board %d eventcounter %d is not matching common counter %d \n", modid, count,
              fNectarRawEvent->fSequenceNumber);
          // JAM - optionally we may stop here in such case?
          //GO4_STOP_ANALYSIS_MESSAGE("Warning: MDPP board %d eventcounter %d is not matching common counter %d \n", modid, count, fNectarRawEvent->fSequenceNumber);
        }
      }

    }
    else if (isDummy)
    {
      // do nothing, just proceed.
    }

    else
    {
      GO4_STOP_ANALYSIS_MESSAGE("Data error: unknown format in MDPP word 0x%x from module id %d-  stopped!", word,
          modid);
      return kFALSE;    // redundant, above will do exception
    }
    // following check will avoid that sublen of frontend is wrong..
    NECTAR_EVENT_CHECK_PDATA
  }    //  while ((pData - pdatastart) < sublen)

  return kTRUE;
}

Bool_t TNectarRawProc::UnpackVmmr()
{
  Int_t VMMR_head = *pData++;
  Bool_t isHeader = ((VMMR_head >> 28) & 0xF) == 0x4;
  if (!isHeader)
    GO4_SKIP_EVENT_MESSAGE("Data error: invalid vmmr header 0x%x -  skip event!", VMMR_head);
  Int_t sublen = VMMR_head & 0xFFF;    // number of following data words,including EOE
  Int_t modid = (VMMR_head >> 16) & 0xFF;    //module id

  TVmmrBoard* theBoard = fNectarRawEvent->GetVmmrBoard(modid);
  if (theBoard == 0)
  {
    GO4_STOP_ANALYSIS_MESSAGE(
        "Configuration error: VMMR module id %d does not exist as subevent. Please check TNectarRawParam setup", modid);
    return kFALSE;
  }

  TVmmrDisplay* boardDisplay = GetVmmrDisplay(modid);
  if (boardDisplay == 0)
  {
    GO4_STOP_ANALYSIS_MESSAGE("Configuration error: VMMR module id %d does not exist as histogram display set!", modid);
    return kFALSE;
  }
  boardDisplay->ResetDisplay();

  boardDisplay->hMsgTypes->Fill(TVmmrMsg::MSG_HEADER);    // always account header if found
// loop over following data words:
  Int_t* pdatastart = pData;    // remember begin of MDPP payload data section
  while ((pData - pdatastart) < sublen)
  {
    Int_t word = *pData++;
    Bool_t isTimestamp = ((word >> 28) & 0xF) == 0x2;
    Bool_t isAdc = ((word >> 28) & 0xF) == 0x1;
    //Bool_t isDeltaT = ((word >> 28) & 0xF) == 0x3;  // old definition  JAM
    Bool_t isEndmark = ((word >> 30) & 0x3) == 0x3;
    Bool_t isDummy = ((word >> 30) & 0x3) == 0x0;
    Bool_t isDeltaT=kFALSE;
    Bool_t isExtendedTS=kFALSE;


    if(isTimestamp)
    {
      isExtendedTS= ((word >> 23) & 0x1) == 0x1;
      isDeltaT = !isExtendedTS;
    }

    if (isExtendedTS)
    {
      UShort_t ts = word & 0xFFFF;
      theBoard->fExtendedTimeStamp = ts;
      boardDisplay->hExtTimeStamp->Fill(ts);
      boardDisplay->hMsgTypes->Fill(TVmmrMsg::MSG_TIMESTAMP);
    }
    else if (isAdc)
    {

      UChar_t slave = (word >> 24) & 0xF;
      UShort_t channel = (word >> 12) & 0xFFF;
      UShort_t value = word & 0xFFF;

      // first put extracted data to output event structure:
      TVmmrSlave* theSubBoard = theBoard->GetSlave(slave);
      if (theSubBoard == 0)
      {
        GO4_STOP_ANALYSIS_MESSAGE("NEVER  COME HERE: Could not access Vmmr subevent for FE slave %d", slave);
        //  JAM -maybe paranoid, but you never know...
      }
      theSubBoard->AddAdcMessage(new TVmmrAdcData(value, channel));

      // then fill histograms of corresponding container:
      boardDisplay->hMsgTypes->Fill(TVmmrMsg::MSG_ADC);

      TVmmrSlaveDisplay* slaveDisplay = boardDisplay->GetSlaveDisplay(slave);
      if (slaveDisplay == 0)
      {
        GO4_STOP_ANALYSIS_MESSAGE("NEVER  COME HERE: Could not access Vmmr display container for  FE slave %d", slave);

      }
      if (channel < VMMR_CHANNELS)
      {
        slaveDisplay->hRawADC[channel]->Fill(value);
      }
      else
      {
        // possible range of FE subaddress may be bigger than number of channels to plot:
        printf("VMMR %d slave %d warning: supress FE subadress %d for histogramming! please check setup.\n", modid,
            slave, channel);
      }

    }
    else if (isDeltaT)
    {
      UChar_t slave = (word >> 24) & 0xF;
      UShort_t value = word & 0xFFFF;

      // first put extracted data to output event structure:
      TVmmrSlave* theSubBoard = theBoard->GetSlave(slave);
      if (theSubBoard == 0)
      {
        GO4_STOP_ANALYSIS_MESSAGE("NEVER  COME HERE: Could not access Vmmr subevent for FE slave %d", slave);
        //  JAM -maybe paranoid, but you never know...
      }
      theSubBoard->fDeltaTGate = value;

      // then fill histograms of corresponding container:
      boardDisplay->hMsgTypes->Fill(TVmmrMsg::MSG_DELTA_T);

      TVmmrSlaveDisplay* slaveDisplay = boardDisplay->GetSlaveDisplay(slave);
      if (slaveDisplay == 0)
      {
        GO4_STOP_ANALYSIS_MESSAGE("NEVER  COME HERE: Could not access Vmmr display container for  FE slave %d", slave);

      }
      slaveDisplay->hDeltaTime->Fill(value);

    }
    else if (isEndmark)
    {
      Int_t count = (word & 0x3FFFFFFF);
      theBoard->fEventCounter = count;
      boardDisplay->hMsgTypes->Fill(TVmmrMsg::MSG_EOE);

      // now check against global counter of all subsystems:
      if (fNectarRawEvent->fSequenceNumber < 0)
      {
        fNectarRawEvent->fSequenceNumber = count;    // init if not yet set by other board
        printdeb("Warning: VMMR board %d setting sequence number to %d \n", modid, count);
        // TODO: maybe we could put MBS header counter to fSequenceNumber, but this may differ by special trigger events
      }
      else
      {
        if (fNectarRawEvent->fSequenceNumber != count)    // check against global counter
        {
          printdeb("Warning: VMMR board %d eventcounter %d is not matching common counter %d \n", modid, count,
              fNectarRawEvent->fSequenceNumber);
          // JAM - optionally we may stop here in such case?
          //GO4_STOP_ANALYSIS_MESSAGE("Warning: VMMR board %d eventcounter %d is not matching common counter %d \n", modid, count, fNectarRawEvent->fSequenceNumber);
        }
      }

    }
    else if (isDummy)
    {
      // do nothing, just proceed.
    }
    else
    {
      GO4_STOP_ANALYSIS_MESSAGE("Data error: unknown format in VMMR word 0x%x from module id %d-  stopped!", word,
          modid);
      return kFALSE;    // redundant, above will do exception
    }

    // following check will avoid that sublen of frontend is wrong..
    NECTAR_EVENT_CHECK_PDATA
  }    // while ((pData - pdatastart) < sublen)
  return kTRUE;
}

Bool_t TNectarRawProc::UpdateDisplays()
{

// place for some advanced analysis from output event data here, if not in the second go4 analysis step...

  // access now collected data of mdpp boards to evaluate delta t histograms:
  for (UInt_t i = 0; i < TNectarRawEvent::fgConfigMdppBoards.size(); ++i)
  {
    UInt_t uniqueid = TNectarRawEvent::fgConfigMdppBoards[i];

    TMdppBoard* theMDPP = fNectarRawEvent->GetMdppBoard(uniqueid);
    if (theMDPP == 0)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
          "Configuration error from UpdateDisplays: MDPP module id %d does not exist as subevent. Please check TNectarRawParam setup",
          uniqueid);
      return kFALSE;
    }

//////////////////// JAM check if there is any data:
#ifdef  NECTAR_VERBOSE_PRINT
    for(UChar_t c=0; c<MDPP_CHANNELS; ++c)
    {
      UInt_t maxmessages=theMDPP->NumTdcMessages(c);
      printdeb("Channel %d has %d maxmessages\n", c, maxmessages);
      for(UInt_t i=0; i<maxmessages; ++i)
      {
        TMdppTdcData* msg= theMDPP->GetTdcMessage(c, i);
        if(msg)
        {
          printdeb(" - data in channel %d (i=%d)  is %d\n", c, i, msg->fData);
        }
      }
    }

#endif

    TMdppDisplay* boardDisplay = GetMdppDisplay(uniqueid);
    if (boardDisplay == 0)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
          "Configuration error from UpdateDisplays: MDPP module id %d does not exist as histogram display set!",
          uniqueid);
      return kFALSE;
    }
    Int_t refchannel = fPar->fMDPP_ReferenceChannel[i];
    if (refchannel < 0 || refchannel >= MDPP_CHANNELS)
      refchannel = 0;

    printdeb("UpdateDisplays - refchannel %d for board %d, index %d\n", refchannel, uniqueid, i);

    // first get all reference time messages of this event:
    UInt_t maxrefs = theMDPP->NumTdcMessages(refchannel);
    printdeb("UpdateDisplays - maxrefs=%d\n", maxrefs);

    if (maxrefs == 0)
      continue;
    Int_t reftime[maxrefs];
    for (UInt_t r = 0; r < maxrefs; ++r)
    {
      TMdppTdcData* refdata = theMDPP->GetTdcMessage(refchannel, r);
      if (refdata)
        reftime[r] = refdata->fData;

      printdeb(" - reftime[%d]=%d\n", r, reftime[r]);
    }
    for (UChar_t c = 0; c < MDPP_CHANNELS; ++c)
    {
      UInt_t maxmessages = theMDPP->NumTdcMessages(c);
      printdeb("Channel %d has %d maxmessages\n", c, maxmessages);
      for (UInt_t i = 0; i < maxmessages; ++i)
      {
        TMdppTdcData* msg = theMDPP->GetTdcMessage(c, i);
        if (msg)
        {
          UInt_t r = i;    // always use reference time message of same buffer index
          if (r && r >= maxrefs)
            r = maxrefs - 1;    // (...if existing, otherwise use most recent one)
          Int_t deltaT = msg->fData - reftime[r];
          printdeb(" - deltaT channel %d (i=%d=  is %d\n", c, i, deltaT);

          boardDisplay->hDeltaTDC[c]->Fill(deltaT);
        }
      }
    }
  }
    
    //////////////////////////////////////////////
    for (UInt_t i = 0; i < TNectarRawEvent::fgConfigVmmrBoards.size(); ++i)
  {
    UInt_t uniqueid = TNectarRawEvent::fgConfigVmmrBoards[i];

    TVmmrBoard* theVmmr = fNectarRawEvent->GetVmmrBoard(uniqueid);
    if (theVmmr== 0)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
          "Configuration error from UpdateDisplays: VMMR module id %d does not exist as subevent. Please check TNectarRawParam setup",
          uniqueid);
      return kFALSE;
    }
    
    TVmmrDisplay* boardDisplay = GetVmmrDisplay(uniqueid);
    if (boardDisplay == 0)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
          "Configuration error from UpdateDisplays: VMMR module id %d does not exist as histogram display set!",
          uniqueid);
      return kFALSE;
    }
    // just fil for slave number 1:
    int slid=1;
    TVmmrSlave* theslave=theVmmr->GetSlave(slid);
     if (theslave== 0)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
          "Configuration error from UpdateDisplays: VMMR slave id %d does not exist as subevent. Please check TNectarRawParam setup",
          slid);
      return kFALSE;
    }
    
    TVmmrSlaveDisplay* sldisplay= boardDisplay->GetSlaveDisplay(slid);
    if (sldisplay == 0)
    {
      GO4_STOP_ANALYSIS_MESSAGE(
          "Configuration error from UpdateDisplays: VMMR slave id %d does not exist as histogram display set!",
          slid);
      return kFALSE;
    }
    
    // now get channel data from event structure for channels 1 and 2:
    
     UInt_t maxvmmrmessages = theslave->NumAdcMessages();
      printdeb("slave %d has %d maxmessages\n", slid,  maxvmmrmessages);
       UShort_t ch1[10];
       UShort_t ch2[10];
       int ix1=0, ix2=0;
      for (UInt_t i = 0; i < maxvmmrmessages; ++i)
      {
          TVmmrAdcData* dat = theslave->GetAdcMessage(i);
           if(dat->fFrontEndSubaddress==1)
           {
                ch1[ix1++]=dat->fData;
                if(ix1>+10) break;
            }
           if(dat->fFrontEndSubaddress==2)
           {
                ch2[ix2++]=dat->fData;
                 if(ix2>+10) break;
            }    
    
      }
      int maxi=ix1;
      if(ix2<ix1)maxi=ix2;      
      for(int i=0; i<maxi;i++)
      {
            sldisplay->hRawADC1vs2->Fill(ch1[i],ch2[i]);     
      }
    
    }   
  
  
  return kTRUE;
}

