#include "TCtr16RawProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TGo4Analysis.h"
#include "TGo4Log.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"
#include "TGo4WinCond.h"

#include "TCtr16RawEvent.h"
#include "TCtr16RawParam.h"
#include "TCtr16Display.h"



static unsigned long skipped_events = 0;
static unsigned long skipped_frames = 0;

/* helper macros for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/

#define  Ctr16EVENT_CHECK_PDATA                                    \
if((fPdata - psubevt->GetDataField()) > fLwords ) \
{ \
  if(skipped_events>100){\
  GO4_STOP_ANALYSIS_MESSAGE("############ unexpected end of event for subevent size :0x%x, stop after %ld skipped events!\n", fLwords, skipped_events++); \
  }else{ \
  GO4_SKIP_EVENT_MESSAGE("############ unexpected end of event for subevent size :0x%x, skip event %ld\n", fLwords, skipped_events++);} \
}

#define  Ctr16RAW_CHECK_PDATA                                    \
if((fPdata - fPdatastart) > Ctr16RawEvent->fDataCount ) \
{ \
  if(skipped_events>100){ \
    GO4_STOP_ANALYSIS_MESSAGE("############ unexpected end of payload for datacount:0x%x, stop after %ld skipped events!\n", Ctr16RawEvent->fDataCount, skipped_events++);\
  }else{ \
    GO4_SKIP_EVENT_MESSAGE("############ unexpected end of payload for datacount:0x%x, skip event %ld\n", Ctr16RawEvent->fDataCount, skipped_events++);}\
}

#define  Ctr16MSG_CHECK_PDATA                                    \
if((fPdata - fPdatastartMsg) > fMsize ) \
{ \
  if(skipped_events>100){\
    GO4_STOP_ANALYSIS_MESSAGE("############ fPdata offset 0x%x exceeds message size:0x%x, stop after %ld skipped events\n", (unsigned int)(fPdata - fPdatastartMsg), fMsize, skipped_events++);\
  }else{ \
    GO4_SKIP_EVENT_MESSAGE("############ fPdata offset 0x%x exceeds message size:0x%x, skip event %ld\n", (unsigned int)(fPdata - fPdatastartMsg), fMsize, skipped_events++);}\
}

// this one is to discard last message that may be cut off by vulom daq:
#define  Ctr16EVENTLASTMSG_CHECK_PDATA                                    \
if((fPdata - psubevt->GetDataField()) >= fLwords ) \
 { \
  skipmessage=kTRUE; \
  break; \
}

/*printf("############ fPdata offset 0x%x exceeds  subevent size :0x%x, skip message %ld\n", (unsigned int) (fPdata - psubevt->GetDataField()), fLwords, skipped_msgs++);\*/

#define Ctr16Dump( args... ) \
if (fPar->fVerbosity>1) printf( args);

#define Ctr16Warn( args... ) \
if(fPar->fVerbosity>0) printf( args);

//***********************************************************
TCtr16RawProc::TCtr16RawProc() :
    TGo4EventProcessor(), fPar(0), Ctr16RawEvent(0), fPdata(0), fPdatastart(0), fLwords(0), fPdatastartMsg(0),
        fMsize(0), fWorkData(0), fWorkShift(0)
{
}

//***********************************************************
// this one is used in standard factory
TCtr16RawProc::TCtr16RawProc(const char *name) :
    TGo4EventProcessor(name), fPar(0), Ctr16RawEvent(0), fPdata(0), fPdatastart(0), fLwords(0), fPdatastartMsg(0),
        fMsize(0), fWorkData(0), fWorkShift(0)
{
  TGo4Log::Info("TCtr16RawProc: Create instance %s", name);
  fBoards.clear();
  SetMakeWithAutosave(kTRUE);
//// init user analysis objects:

  fPar = dynamic_cast<TCtr16RawParam*>(MakeParameter("Ctr16RawParam", "TCtr16RawParam", "set_Ctr16RawParam.C"));
  if (fPar)
    fPar->SetConfigBoards();

  for (unsigned i = 0; i < TCtr16RawEvent::fgConfigCtr16Boards.size(); ++i)
  {
    UInt_t uniqueid = TCtr16RawEvent::fgConfigCtr16Boards[i];
    fBoards.push_back(new TCtr16BoardDisplay(uniqueid));
  }

  InitDisplay(fPar->fTraceLength, fPar->fNumSnapshots, false);

}

//***********************************************************
TCtr16RawProc::~TCtr16RawProc()
{
  TGo4Log::Info("TCtr16RawProc: Delete instance");
  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    delete fBoards[i];
  }

}

/* access to histogram set for current board id*/
TCtr16BoardDisplay* TCtr16RawProc::GetBoardDisplay(Int_t uniqueid)
{
  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    TCtr16BoardDisplay *theDisplay = fBoards[i];
    if (uniqueid == theDisplay->GetDevId())
      return theDisplay;
  }
  return 0;
}

void TCtr16RawProc::InitDisplay(int tracelength, Int_t numsnapshots, Bool_t replace)
{
  cout << "**** TCtr16RawProc: Init Display for " << tracelength << " trace bins, snapshots=" << numsnapshots << endl;

  for (unsigned i = 0; i < fBoards.size(); ++i)
  {
    fBoards[i]->InitDisplay(tracelength, numsnapshots, replace);
  }

}

//-----------------------------------------------------------
// event function
Bool_t TCtr16RawProc::BuildEvent(TGo4EventElement *target)
{
// called by framework from TCtr16RawEvent to fill it
  Ctr16RawEvent = (TCtr16RawEvent*) target;
  Ctr16RawEvent->SetValid(kFALSE);    // not store
  TGo4MbsEvent *source = (TGo4MbsEvent*) GetInputEvent();
  if (source == 0)
  {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  UShort_t triggertype = source->GetTrigger();

  if (triggertype > 11)
  {
    // frontend offset trigger can be one of these, we let it through to unpacking loop
    //cout << "**** TCtr16RawProc: Skip trigger event" << endl;
    GO4_SKIP_EVENT_MESSAGE("**** TCtr16RawProc: Skip event of trigger type 0x%x", triggertype);
    //return kFALSE; // this would let the second step execute!
  }

  static Int_t numsnapshots = fPar->fNumSnapshots;
  static Int_t tracelength = fPar->fTraceLength;

  // since we fill histograms already in BuildEvent, need to check if we must rescale histogram displays:
  if ((fPar->fNumSnapshots != numsnapshots) || (fPar->fTraceLength != tracelength))
  {
    numsnapshots = fPar->fNumSnapshots;
    tracelength = fPar->fTraceLength;
    InitDisplay(tracelength, numsnapshots, kTRUE);

  }

// first we fill the TCtr16RawEvent with data from MBS source
// we have up to two subevents, crate 1 and 2
// Note that one has to loop over all subevents and select them by
// crate number:   psubevt->GetSubcrate(),
// procid:         psubevt->GetProcid(),
// and/or control: psubevt->GetControl()
// here we use only crate number

  source->ResetIterator();
  TGo4MbsSubEvent *psubevt(0);
  while ((psubevt = source->NextSubEvent()) != 0)
  {    // loop over subevents
    fPdata = psubevt->GetDataField();
    fLwords = psubevt->GetIntLen();

    if ((unsigned) *fPdata == 0xbad00bad)
    {
      GO4_SKIP_EVENT_MESSAGE("**** TCtr16RawProc: Found BAD mbs event (marked 0x%x), skip it.", (*fPdata));
    }
    Bool_t skipmessage = kFALSE;
    // loop over single subevent data:
    while (fPdata - psubevt->GetDataField() < fLwords)
    {
      // vulom status word:
      Ctr16RawEvent->fVULOMStatus = *fPdata++;
      //event trigger counter:
      Ctr16RawEvent->fSequenceNumber = *fPdata++;
      // data length
      Ctr16RawEvent->fDataCount = *fPdata++;
      if (Ctr16RawEvent->fDataCount > (fLwords - 3))
      {
        GO4_SKIP_EVENT_MESSAGE(
            "**** TCtr16RawProc: Mismatch with subevent len %d and data count 0x%8x - vulom status:0x%x seqnum:0x%x \n",
            fLwords, Ctr16RawEvent->fDataCount, Ctr16RawEvent->fVULOMStatus, Ctr16RawEvent->fSequenceNumber);
        // avoid that we run optional second step on invalid raw event!
      }

      fPdatastart = fPdata;    // remember begin of asic payload data section
      fPdata++;    // skip first  word?

      // now fetch boardwise subcomponents for output data and histograming:
      Int_t slix = 0;    // JAM here we later could evaluate a board identifier mapped to a slot/sfp number contained in subevent
      UInt_t brdid = fPar->fBoardID[slix];    // get hardware identifier from "DAQ link index" number
      TCtr16Board *theBoard = Ctr16RawEvent->GetBoard(brdid);
      if (theBoard == 0)
      {
        GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as subevent, slot index:%d", brdid,
            slix);

        return kFALSE;
      }
      TCtr16BoardDisplay *boardDisplay = GetBoardDisplay(brdid);
      if (boardDisplay == 0)
      {
        GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
        return kFALSE;
      }
      boardDisplay->ResetDisplay();

      // evaluate Ctr16ection ASIC messages in the payload:

      while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)
      {

        // first vulom wrapper containing total message length:
        Int_t vulombytecount = *fPdata++;

        if (((vulombytecount >> 28) & 0xF) == 0x4)    // check if we are still on track with expected vulom header
        {
          Int_t chipid = (vulombytecount >> 16) & 0xFF;
          boardDisplay->hChipId->Fill(chipid);

          fPdatastartMsg = fPdata;    // remember start of message for checking
          fMsize = (vulombytecount & 0x3F) / sizeof(Int_t);    // message size in 32bit words
          fWorkData = *fPdata;    //init work buffer as aligned
          fWorkShift = 0;
          while ((fPdata - fPdatastartMsg) < fMsize)
          {
            //inside message loop, we use aligned data words:
            // evaluate message type from header:
            Int_t header = fWorkData;
            // not we do not increment fPdata here, do this inside msg types
            Int_t frametype = ((header >> 30) & 0x3);
            boardDisplay->hFrameTypes->Fill(frametype);
            //printf("MMMMMMMM message type %d \n",mtype);
            switch (frametype)
            {

              case TCtr16Msg::Frame_Data:

                {
                  // first get epoch number
                  UInt_t epoch = header & 0xFFFFFF;
                  NextDataWord();    //fPdata++;
                  // scan payload for events:
                  TCtr16Msg::DataType evtype = (TCtr16Msg::DataType) ((fWorkData >> 30) & 0x3);
                  UChar_t fullchannel = ((fWorkData >> 26) & 0xF);
                  UChar_t row = ((fWorkData >> 24) & 0x3);
                  boardDisplay->hDataTypes->Fill(evtype);
                  boardDisplay->hMemoryRow->Fill(row);
                  switch (evtype)
                  {
                    case TCtr16Msg::Data_Transient:
                      {
                        if (theBoard->fCurrentTraceEvent == 0)
                          theBoard->fCurrentTraceEvent = new TCtr16MsgTransient(fullchannel);
                        TCtr16MsgTransient *tmsg = theBoard->fCurrentTraceEvent;
                        tmsg->SetEpoch(epoch);
                        tmsg->SetRow(row);
                        UShort_t ts = fWorkData & 0xFFF;
                        tmsg->SetTimeStamp(ts);
                        theBoard->fTracesize12bit = ((fWorkData >> 16) & 0xF);
                        theBoard->fTracesize32bit = ((1 + theBoard->fTracesize12bit) * 3) / 8;    // account header word again in evdata

                        // account partially filled last data word here: ? taken from hitdetection code of 2017  JAM23
                        if ((Float_t) (1 + theBoard->fTracesize12bit) * 3.0 / 8.0 > (Float_t) theBoard->fTracesize32bit)
                          theBoard->fTracesize32bit++;

                        Ctr16Dump("Data_Transient - channel:%d size12:%d size32:%d\n", fullchannel,
                            theBoard->fTracesize12bit, theBoard->fTracesize32bit);
                        if ((fPdata - fPdatastart) + theBoard->fTracesize32bit > fLwords)
                          GO4_SKIP_EVENT_MESSAGE(
                              "Transient Event header error: 12 bit size %d does not fit into mbs subevent buffer of restlen %ld words",
                              theBoard->fTracesize32bit, fLwords - (fPdata - fPdatastart));

                        boardDisplay->hDatawords->Fill(theBoard->fTracesize12bit);
                        boardDisplay->hChannels->Fill(fullchannel);
                        ExtractTrace(theBoard, boardDisplay);
                      }
                      break;

                    case TCtr16Msg::Data_Feature:
                      {
                        TCtr16MsgFeature *fmsg = new TCtr16MsgFeature(fullchannel);
                        fmsg->SetEpoch(epoch);
                        fmsg->SetRow(row);
                        UShort_t ts = (fWorkData >> 5) & 0xFFF;
                        fmsg->SetTimeStamp(ts);
                        UChar_t finetime = (fWorkData & 0x3F);
                        fmsg->SetFineTime(finetime);
                        NextDataWord();
                        UShort_t ampl = (fWorkData >> 16) & 0xFFFF;
                        fmsg->SetAmplitude(ampl);
                        // feature histograms here:
                        boardDisplay->hFeatureAmplitude[fullchannel]->Fill(ampl);
                        boardDisplay->hFeatureFineTime[fullchannel]->Fill(finetime);
                        UpdateDeltaTimes(theBoard, boardDisplay, fmsg, fullchannel); // evaluate delta T between messages also here
                        theBoard->AddMessage(fmsg, fullchannel);
                        SwitchDataAlignment();// next message will begin in lower 16 bit part of this data word
                        NextDataWord();
                        // TODO: check if another feature event would fit into rest of vulom container


                      }
                      break;

                    default:
                      printf("############ found unknown data type 0x%x, skip data fra,e %ld\n", evtype,
                          skipped_frames++);
                      fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:

                      break;
                  }

                  // repeat loop over next event from here

                }
                break;

              case TCtr16Msg::Frame_Continuation:
                {
                  if (!theBoard->fToBeContinued)
                  {
                    printf("!!!! Unexpected continuation frame! Skip event %ld\n", skipped_events++);
                    GO4_SKIP_EVENT
                    ;
                  }
                  if (theBoard->fCurrentTraceEvent == 0)
                  {
                    printf("!!!! Continuation frame without previous transient message! Skip event %ld\n",
                        skipped_events++);
                    GO4_SKIP_EVENT
                    ;
                  }

                  // check if epochs match with already accounted:
                  UInt_t epoch = header & 0xFFFFFF;
                  if (epoch != theBoard->fCurrentTraceEvent->GetEpoch())
                  {
                    printf("!!!! Continuation frame with epoch mismatch:  previous 0x%x current 0x%x ! Skip event %ld\n",
                        epoch, theBoard->fCurrentTraceEvent->GetEpoch(), skipped_events++);
                  }
                  NextDataWord();
                  ExtractTrace(theBoard, boardDisplay);
                }
                break;

              case TCtr16Msg::Frame_Error:
                {
                  UInt_t epoch = header & 0xFFFFFF;
                  NextDataWord();
                  while(fPdata-fPdatastartMsg < fMsize)
                    {
                      UChar_t code =(fWorkData>>24) & 0xFF;
                      UShort_t ts = (fWorkData>>8) & 0xFFF;
                      printf("ERROR FRAME: code:0x%x epoch 0x%x timestamp:0x%x \n", code, epoch, ts);
                      // TODO: create message and fill histograms
                      fWorkShift+=8;
                      NextDataWord();
                    }
                }
                break;

              case TCtr16Msg::Frame_Wishbone:
                // wishbone response (error message)
                {
                  UChar_t wishhead = (header >> 24) & 0xFF;
                  UChar_t ctrlhead = (header >> 16) & 0xFF;
                  //printf("MSG_WishboneEvent header=0x%x\n",wishhead);
                  TCtr16MsgWishbone *theMsg = new TCtr16MsgWishbone(wishhead);
                  boardDisplay->hWishboneAck->Fill(theMsg->GetAckCode());
                  // first evaluate what kind of data we have:
                  Bool_t isThresholdMessage = kFALSE;
                  Bool_t isSlowControlType = kFALSE;
                  TCtr16MsgWishbone::ControlType ctype = TCtr16MsgWishbone::Ctrl_None;
                  if (theMsg->GetAckCode() == TCtr16MsgWishbone::ACK_SlowControl)
                  {
                    ctype = (TCtr16MsgWishbone::ControlType) ctrlhead;
                    if (ctype == TCtr16MsgWishbone::Ctrl_Start)
                    {
                      // here check for lenght if we maybe have a threshold message
                      if (fMsize <= 3)
                      {
                        isThresholdMessage = kTRUE;
                        boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Threshold);
                      }
                      else
                      {
                        boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Start);
                      }
                    }
                    else
                    {
                      boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Init);
                    }
                    isSlowControlType = kTRUE;
                  }
                  else
                  {
                    boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Wishbone);
                  }
                  // end data type evaluation

                  if (isThresholdMessage)
                  {
                    // use threshold message instead of wishbone container:
                    delete theMsg;
                    UChar_t chan = (fWorkData >> 8) & 0xF;    // fWorkData is still at begin of message header
                    TCtr16MsgThreshold *msg = new TCtr16MsgThreshold(chan);
                    UShort_t meanbase = ((fWorkData & 0xF) << 8);
                    NextDataWord();    //fPdata++;
                    meanbase |= (fWorkData >> 24) & 0xFF;
                    UShort_t noisewidth = (fWorkData >> 8) & 0xFFF;
                    UShort_t thresh = ((fWorkData & 0xF) << 8);
                    NextDataWord();    //fPdata++;
                    thresh |= (fWorkData >> 24) & 0xFF;
                    msg->SetBaseline(meanbase);
                    msg->SetNoiseWidth(noisewidth);
                    msg->SetThreshold(thresh);
                    // histograms of threshold values for each channel
                    boardDisplay->hThresholdBaseline[chan]->Fill(meanbase);
                    boardDisplay->hThresholdNoise[chan]->Fill(noisewidth);
                    boardDisplay->hThresholdSetting[chan]->Fill(thresh);
                    theBoard->AddMessage(msg, chan);
                  }
                  else
                  {
                    // treat data as wishbone message:
                    theMsg->SetControlMessageType(ctype);
                    if (!isSlowControlType)
                    {
                      // regular wishbone message:
                      Short_t address = (header >> 8) & 0xFFFF;
                      theMsg->SetAddress(address);
                      // here optional rest of data:

                      UChar_t val = fWorkData && 0xFF;    // fWorkData is still at begin of message header
                      theMsg->AddData(val);
                      NextDataWord();    //fPdata++;
                      if (fPdata - fPdatastartMsg < fMsize)
                      {
                        for (Int_t shift = 24; shift > 0; shift -= 8)
                        {
                          val = (fWorkData >> shift) & 0xFF;
                          theMsg->AddData(val);
                        }
                        NextDataWord();    //fPdata++;
                      }
                      boardDisplay->hWishboneSource->Fill(theMsg->GetSource());
                      // TODO: histogram of wishbone addresses and data words?

                    }
                    else
                    {
                      // slow control message types do not have address fields.
                      // we embed the containing text and dump it for fun;
                      UChar_t letter;
                      for (Int_t shift = 8; shift > 0; shift -= 8)
                      {
                        letter = (fWorkData >> shift) & 0xFF;
                        theMsg->AddData(letter);
                      }

                      NextDataWord();    //fPdata++;
                      while (fPdata - fPdatastartMsg < fMsize)
                      {
                        Ctr16RAW_CHECK_PDATA;
                        for (Int_t shift = 24; shift > 0; shift -= 8)
                        {
                          letter = (fWorkData >> shift) & 0xFF;
                          theMsg->AddData(letter);
                        }
                      }
                      boardDisplay->lWishboneText->SetText(0.1, 0.9, theMsg->DumpMsg());
                      if (fPar->fVerbosity > 1)
                      {
                        printf("Control message text:\n %s", theMsg->DumpMsg().Data());
                        std::cout << std::endl;
                      }

                    }    // whishbone response or slow control

                    theBoard->AddMessage(theMsg, 0);    // wishbone messages accounted for channel 0
                  }

                }
                break;

              default:
                //printf("############ found unknown message type 0x%x, skip event %ld\n", mtype, skipped_events++);
                //GO4_SKIP_EVENT
                fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                break;
            };    // switch

            if (!skipmessage && (fPdata - fPdatastartMsg) < fMsize)
            {
              // never come here if messages are treated correctly!
              printf("############  fPdata offset 0x%x has not yet reached message length 0x%x, correcting ,\n",
                  (unsigned int) (fPdata - fPdatastartMsg), fMsize);
              fPdata = fPdatastartMsg + fMsize;
            }

            //printf("EEEEEEEE  end of message payload: fPdata offset 0x%x msglength 0x%x\n",
            //             (unsigned int) (fPdata - fPdatastartMsg), fMsize);

          }    //  while ((fPdata - fPdatastartMsg) < fMsize)
        }    //if (((vulombytecount >> 28) & 0xF) == 0x4)
      }    // while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)
    }    // while fPdata - psubevt->GetDataField() <fLwords
  }    // while subevents

  UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
  Ctr16RawEvent->SetValid(kTRUE);    // to store

  if (fPar->fSlowMotion)
  {
    Int_t evnum = source->GetCount();
    GO4_STOP_ANALYSIS_MESSAGE(
        "Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);
  }

  return kTRUE;
}

void TCtr16RawProc::NextDataWord()
{
  Ctr16RAW_CHECK_PDATA;
  Ctr16MSG_CHECK_PDATA;
  if (fWorkShift == 0)
  {
    fWorkData = *fPdata++;
  }
  else
  {
    fWorkData = 0;
    UInt_t mask = 0;
    for (UChar_t b = 0; b < (32-fWorkShift); ++b)
      mask |= (1 << b);
    fWorkData = (*fPdata << (32-fWorkShift));
    fPdata++;
    Ctr16RAW_CHECK_PDATA;
    Ctr16MSG_CHECK_PDATA;
    fWorkData |= (*fPdata >> fWorkShift) & mask;
  }

  // fWorkShift defines the n lower bits of the current subevent word that are used for next data word d
  // bits:
  //|31---------fWorkshift--0|  index
  //|------------- |ddddddddd|  (subevent word 1)
  //|dddddddddddddd|---------|  (subevent word 2)

}

void TCtr16RawProc::SwitchDataAlignment()
{
  (fWorkShift == 0) ? fWorkShift = 16 : fWorkShift = 0;
}

Bool_t TCtr16RawProc::UpdateDisplays()
{
  static Bool_t CalibrateWasOn = kFALSE;

// maybe later some advanced analysis from output event data here

  for (unsigned i = 0; i < TCtr16RawEvent::fgConfigCtr16Boards.size(); ++i)
  {
    UInt_t brdid = TCtr16RawEvent::fgConfigCtr16Boards[i];
    TCtr16Board *theBoard = Ctr16RawEvent->GetBoard(brdid);
    if (theBoard == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("FillDisplays Configuration error: Board id %d does not exist!", brdid);
      //return kFALSE;
    }
    TCtr16BoardDisplay *boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("FillDisplays Configuration error: Board id %d does not exist as histogram display set!",
          brdid);
      //return kFALSE;
    }

    // here calculate integral and differential ADC nonlinearities:
    boardDisplay->hADCNonLinInt->Reset("");
    boardDisplay->hADCNonLinDiff->Reset("");

    if (!CalibrateWasOn && fPar->fDoCalibrate)
    {
      // begin of calibration, reset value histograms
      boardDisplay->hADCValues->Reset("");
      boardDisplay->hADCCorrection->Reset("");
      TGo4Log::Info("TCtr16BoardDisplay: Begin new ADC calibration for Board %d ", boardDisplay->GetDevId());

    }
    if (CalibrateWasOn && !fPar->fDoCalibrate)
    {
      TGo4Log::Info("TCtr16BoardDisplay: End ADC calibration for Board %d ", boardDisplay->GetDevId());
    }
    Double_t mean = boardDisplay->hADCValues->GetEntries() / boardDisplay->hADCValues->GetNbinsX();
    Double_t inl = 0;
    Double_t corr = 0;
    for (Int_t bix = 0; bix < boardDisplay->hADCValues->GetNbinsX(); ++bix)
    {
      Double_t val = boardDisplay->hADCValues->GetBinContent(bix + 1);
      Double_t delta = (val - mean);
      boardDisplay->hADCDeltaMeanValues->SetBinContent(bix + 1, delta);
      Double_t dnl = 0;
      if (mean)
        dnl = delta / mean;    //dnl = TMath::Abs(delta / mean);
      boardDisplay->hADCNonLinDiff->SetBinContent(bix + 1, dnl);
      if (mean)
        inl += delta / mean;
      boardDisplay->hADCNonLinInt->SetBinContent(bix + 1, inl);
      // calibrate for ADC nonlinearity corrections:
      if (fPar->fDoCalibrate)
      {
        corr = inl;    // this is point to evaluate other kind of correction optionally
        boardDisplay->hADCCorrection->SetBinContent(bix + 1, corr);
      }
    }

  }    // i board

  CalibrateWasOn = fPar->fDoCalibrate;

  return kTRUE;
}

Double_t TCtr16RawProc::CorrectedADCVal(Short_t raw, TCtr16BoardDisplay *boardDisplay)
{
  Double_t res = raw;
  Int_t corbin = boardDisplay->hADCCorrection->FindBin(raw);
  Double_t corr = boardDisplay->hADCCorrection->GetBinContent(corbin);
  res += corr;
  return res;
}

void TCtr16RawProc::ExtractTrace(TCtr16Board *board, TCtr16BoardDisplay *disp)
{

  Int_t payloadwords = board->fTracesize32bit - board->fTracedataIndex;
  if (payloadwords > Ctr16_MAXPAYLOAD / 4)
  {
    payloadwords = Ctr16_MAXPAYLOAD / 4;
    board->fToBeContinued = kTRUE;
  }
  else
  {
    board->fToBeContinued = kFALSE;
  }

  for (Int_t w = board->fTracedataIndex; w < payloadwords; ++w)
  {
    Ctr16RAW_CHECK_PDATA;
    Ctr16MSG_CHECK_PDATA;    // should not be, but who knowns
    board->fTracedata[board->fTracedataIndex] = fWorkData;    //*fPdata++;
    NextDataWord();
    Ctr16Dump("Transient_Event copies 32bit data[%d]=0x%x \n", board->fTracedataIndex,
        board->fTracedata[board->fTracedataIndex]);
    board->fTracedataIndex++;
    if (board->fTracedataIndex > Ctr16_TRACEWORDS)
    {
      printf("?????? trace payload %d beyond maximum tracewords %d, limiting...\n", board->fTracedataIndex,
      Ctr16_TRACEWORDS);
      board->fTracedataIndex = Ctr16_TRACEWORDS;

    }
  }

  if (board->fToBeContinued)
  {
    // we expect further data in the continuation frame...
    Ctr16Dump("Transient_Event expects continuation frame after %d words... \n", board->fTracedataIndex);
  }
  else
  {
    // we are ready inside the first data frame.
    FinalizeTrace(board, disp);
  }
}

void TCtr16RawProc::FinalizeTrace(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  // this is called when all transient data is received from data and continuation frames
  // we scan through the buffer and fill the transient message
  // then add it to output event
  Int_t tracelength = fPar->fTraceLength;    // for display only
  Int_t binlen = board->fTracesize12bit;    // number of sample bins (should be 16, 32, or 64) ??? -3
  if (binlen > 64)
    GO4_SKIP_EVENT_MESSAGE("Transient_Event header error: bin length %d exceeds maximum 64", binlen);
//

  /** taken from 2019 hitdetection asic unpacker  JAM23 :*/
  UShort_t val = 0;
  Int_t dixoffset = 0;    // actual sample data  begins in first 32bit word
  // j counts global bit number in stream, j_start is first in stream, j_end is last
  // k is local bit number in evdata word (lsb=0)
  UShort_t j_start = 0;    // begin of first 12 bit sample is after timestamp - here at start of new longword
  for (Int_t bin = 0; bin < binlen; ++bin)
  {
    UShort_t j_end = j_start + 12;
    Int_t dix_start = (Int_t) j_start / 32;    // data index containing first bit of sample
    Int_t dix_end = (Int_t) (j_end - 1) / 32;    // data index containing last bit of sample
    UChar_t k_start = 32 - (j_start - 32 * dix_start);    //  start bit number in evdata word
    UChar_t k_end = 32 - (j_end - 32 * dix_end);    // end bit number in evdata word
    Ctr16Dump("Transient_Event jstart:%d jend:%d dix_start:%d dix_end:%d k_start:%d k_end:%d \n", (int ) j_start,
        (int ) j_end, dix_start, dix_end, (int ) k_start, (int ) k_end);
    if (dix_start == dix_end)
    {
      // easy case, sample is inside one evdata word:
      val = (board->fTracedata[dixoffset + dix_start] >> k_end) & 0xFFF;
      Ctr16Dump("Transient_Event sees NON spanning value\n");
    }
    else if (dix_end == dix_start + 1)
    {
      // spanning over 2 evdata words:
      UChar_t mask_start = 0, mask_end = 0;
      for (UChar_t b = 0; b < k_start; ++b)
        mask_start |= (1 << b);

      for (UChar_t b = 0; b < (32 - k_end); ++b)
        mask_end |= (1 << b);
      val = ((board->fTracedata[dixoffset + dix_start] & mask_start) << (12 - k_start));
      val |= (board->fTracedata[dixoffset + dix_end] >> k_end) & mask_end;
      Ctr16Dump("Transient_Event sees SPANNING value\n");
    }
    else
    {
      // never come here
      GO4_STOP_ANALYSIS_MESSAGE("NEVER COME HERE: mismatch of evsample indices - dix_end:%d and dix_start:%d", dix_end,
          dix_start)

    }
    Ctr16Dump("Transient_Event set bin:%d to val:%d\n", bin, val);
    board->fCurrentTraceEvent->SetTraceData(bin, val);
    j_start = j_end;    // next 12 bit word
  }    // for bin

  UChar_t chan = board->fCurrentTraceEvent->GetChannel();
  board->AddMessage(board->fCurrentTraceEvent, chan);

  // now display complete trace:

  TH1 *tracesnapshot = 0;
  TH2 *trace2d = 0;
  if (disp->fSnapshotcount[chan] < Ctr16_MAXSNAPSHOTS)
  {
    tracesnapshot = disp->hTraceSnapshots[chan][disp->fSnapshotcount[chan]];
    //std::cout<< "getting histogram for  channel "<< (int) chan<<" snapshot:"<<disp->fSnapshotcount[chan]]<<", binlen="<<binlen<<", tracelen="<< tracelength<< std::endl;
    trace2d = disp->hTraceSnapshot2d[chan];
  }
  for (Int_t bin = 0; bin < binlen; ++bin)
  {
    Short_t val = board->fCurrentTraceEvent->GetTraceData(bin);
    // convert raw data to signed 8bit (2 complement) representation:
    if (val > 0x7FF)
      val = val - 0x1000;

    if (tracesnapshot && bin < tracelength)
    {
      tracesnapshot->SetBinContent(bin + 1, val);
      //std::cout<< " --- Set bin:"<<bin<<" to content:"<<val << std::endl;
    }
    if (trace2d)
      trace2d->Fill(bin, val, disp->fSnapshotcount[chan]);

    if (chan >= Ctr16_CHANNELS)
    {
      printf("Transient_Event channel:%d out of range %d\n", chan, Ctr16_CHANNELS);
    }
    else
    {
      if (bin < tracelength)
      {
        disp->hTrace[chan]->SetBinContent(1 + bin, val);
        disp->hTraceSum[chan]->AddBinContent(1 + bin, val);
      }
    }

    // value histograms:
    disp->hADCValues->Fill(val);
    Double_t corrval = CorrectedADCVal(val, disp);
    disp->hADCCValuesCorrected->Fill(corrval);
  }

  UpdateDeltaTimes(board, disp, board->fCurrentTraceEvent, chan); // delta t histograms are common with feature events

  // reset aux data members:
  board->fCurrentTraceEvent = 0;    // do not delete, message is kept in vector!
  board->fTracedataIndex = 0;
  board->fTracesize12bit = 0;
  board->fTracesize32bit = 0;
  board->fToBeContinued = kFALSE;

}




void TCtr16RawProc::UpdateDeltaTimes(TCtr16Board* board, TCtr16BoardDisplay* disp, TCtr16MsgEvent* ev, UChar_t chan)
{
  /** JAM23 from hitdetasic of 2019 - evaluate time difference between subsequent event messages*/
    UInt_t lastepoch = board->fLastMessages[chan].GetEpoch();
    UShort_t lasttimestamp = board->fLastMessages[chan].GetTimeStamp();
    if (lastepoch != 0 && lasttimestamp != 0)
    {
      UInt_t deltaepoch = ev->GetEpoch() - lastepoch;
      UShort_t deltatimestamp = ev->GetTimeStamp() - lasttimestamp;
      //ULong_t deltaT= (deltaepoch<<12) | deltatimestamp;
      // the above would require too large a histogram. We separate ts and epoch histograms:
      disp->hDeltaTSMsg[chan]->Fill(deltatimestamp);
      disp->hDeltaEPMsg[chan]->Fill(deltaepoch);
      disp->hDeltaEPMsgFine[chan]->Fill(deltaepoch);
    }
    board->fLastMessages[chan] = *ev;    // remember us for next message
    /** end 2019 time differences**/
}


