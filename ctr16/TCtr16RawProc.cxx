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

/** aktivate this to skip all continuation frames. for debugging */
//#define Ctr16_IGNORE_CONTINUATION 1

/** if enabled, use only first partial event in each continuation frame*/
//#define Ctr16_SKIP_CONTINUATION_REST 1

static unsigned long skipped_events = 0;
static unsigned long skipped_frames = 0;

#define Ctr16Debug( args... ) \
if (fPar->fVerbosity>2) printf( args);

#define Ctr16Dump( args... ) \
if (fPar->fVerbosity>1) printf( args);

#define Ctr16Warn( args... ) \
if(fPar->fVerbosity>0) printf( args);


/* helper macros for BuildEvent to check if payload pointer is still inside delivered region:*/

#define  Ctr16EVENT_CHECK_PDATA                                    \
if((fPdata - fPsubevt->GetDataField()) > fLwords ) \
    Ctr16Warn("############ unexpected end of event for subevent size :0x%x, skip event %ld\n", fLwords, skipped_events++);\
     GO4_SKIP_EVENT \
     }


#define  Ctr16RAW_CHECK_PDATA                                    \
    if((fPdata - fPdatastart) > Ctr16RawEvent->fDataCount)  \
    { \
     Ctr16Warn("############ unexpected end of payload for datacount:0x%x after 0x%x words, end of event (#oversized=%ld) \n", Ctr16RawEvent->fDataCount, (unsigned int) (fPdata - fPdatastart), skipped_events++);\
     return 2;\
    }

#define  Ctr16MSG_CHECK_PDATA                                    \
if((fPdata - fPdatastartMsg) > fMsize + 1 ) \
{ \
  Ctr16Warn("############ pdata offset 0x%x exceeds message size:0x%x, skip message \n", (unsigned int)(fPdata - fPdatastartMsg), fMsize);\
  return 1;\
}

// this one is to discard last message that may be cut off by vulom daq:
#define  Ctr16EVENTLASTMSG_CHECK_PDATA                                    \
if((fPdata - fPsubevt->GetDataField()) >= fLwords ) \
 { \
  return 3; \
}

/*printf("############ fPdata offset 0x%x exceeds  subevent size :0x%x, skip message %ld\n", (unsigned int) (fPdata - fPsubevt->GetDataField()), fLwords, skipped_msgs++);\*/




#define Ctr16_NEXT_DATAWORD \
status=NextDataWord(); \
if(status==1) {skipmessage=kTRUE;  fPdata = fPdatastartMsg + fMsize;  break;} \
if(status>=2) goto end_of_event;

//***********************************************************
TCtr16RawProc::TCtr16RawProc() :
    TGo4EventProcessor(), fPar(0), fPsubevt(0), Ctr16RawEvent(0), fPdata(0), fPdatastart(0), fLwords(0), fPdatastartMsg(0),
        fMsize(0), fWorkData(0), fWorkShift(0)
{
}

//***********************************************************
// this one is used in standard factory
TCtr16RawProc::TCtr16RawProc(const char *name) :
    TGo4EventProcessor(name), fPar(0), fPsubevt(0), Ctr16RawEvent(0), fPdata(0), fPdatastart(0), fLwords(0), fPdatastartMsg(0),
        fMsize(0), fWorkData(0), fWorkShift(0)
{
  skipped_events=0;
  skipped_frames=0;
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
// crate number:   fPsubevt->GetSubcrate(),
// procid:         fPsubevt->GetProcid(),
// and/or control: fPsubevt->GetControl()
// here we use only crate number

  source->ResetIterator();
  while ((fPsubevt = source->NextSubEvent()) != 0)
  {    // loop over subevents
    fPdata = fPsubevt->GetDataField();
    fLwords = fPsubevt->GetIntLen();

    if ((unsigned) *fPdata == 0xbad00bad)
    {
      GO4_SKIP_EVENT_MESSAGE("**** TCtr16RawProc: Found BAD mbs event (marked 0x%x), skip it.", (*fPdata));
    }
    Bool_t skipmessage = kFALSE;
    Int_t status=0;
    // loop over single subevent data:ThresholdSetting5
    while (fPdata - fPsubevt->GetDataField() < fLwords)
    {
      // vulom status word:
      Ctr16RawEvent->fVULOMStatus = *fPdata++;
      // from get4++ JAM2020: need to check if status word has valid format here:
      if (((Ctr16RawEvent->fVULOMStatus >> 12) & 0x3) != 0x3)
      {
          Ctr16Dump("VULOM: wrong vulom status word: 0x%x skip it.. \n",
              Ctr16RawEvent->fVULOMStatus);

          continue;
      }


      //event trigger counter:
      Ctr16RawEvent->fSequenceNumber = *fPdata++;
      // data length
      Ctr16RawEvent->fDataCount = *fPdata++;
          //1 + *fPdata++; // from get4++ unpacker: payload is one more according to f_user_readout JAM 10-22
      Ctr16Dump("VULOM: status: 0x%x counter: 0x%x length: 0x%x \n",
                          Ctr16RawEvent->fVULOMStatus,
                          Ctr16RawEvent->fSequenceNumber, Ctr16RawEvent->fDataCount);


      if (Ctr16RawEvent->fDataCount > (fLwords - 3))
      {
        // do not put this into loggin queue for gui...
        printf(
            "**** TCtr16RawProc: Mismatch with subevent len %d and data count 0x%8x - vulom status:0x%x seqnum:0x%x \n",
            fLwords, Ctr16RawEvent->fDataCount, Ctr16RawEvent->fVULOMStatus, Ctr16RawEvent->fSequenceNumber);
        GO4_SKIP_EVENT;
        // avoid that we run optional second step on invalid raw event!
      }

      fPdatastart = fPdata;    // remember begin of asic payload data section
      fPdata++;    // skip first  word?
      fPdata++; // skip another word with 0 bytecount flag?
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
      //printf("EEEEEEEEEEE Event %d begins, data count= %d \n",source->GetCount(), Ctr16RawEvent->fDataCount);
      while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)
      {

        // first vulom wrapper containing total message length:
        Int_t vulombytecount = *fPdata++;

        if (((vulombytecount >> 28) & 0xF) == 0x4)    // check if we are still on track with expected vulom header
        {
          Int_t chipid = (vulombytecount >> 16) & 0xFF;
          boardDisplay->hChipId->Fill(chipid);

          fPdatastartMsg = fPdata;    // remember start of message for checking


          fMsize = (vulombytecount & 0xFF) / sizeof(Int_t);    // message size in 32bit words (was & 0x3F)
          if(((vulombytecount & 0xFF) % sizeof(Int_t)) !=0)
            fMsize++; // ?test this!

          Ctr16Debug("VVVV Vulom container, header 0x%x with message size: %d \n",vulombytecount,fMsize);

          fWorkData = *fPdata++;    //init work buffer as aligned
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
                  Ctr16_NEXT_DATAWORD;    //fPdata++;
                  Ctr16Dump("FFFFFF Data Frame with epoch 0x%x - fPdata:%p, fPdatastartMsg:%p\n", epoch, fPdata, fPdatastartMsg);
                  // scan payload for events:
                  while (fPdata - fPdatastartMsg < fMsize)
                  {
                    Ctr16Dump(" Data Frame loop: fPdata:%p, cursor:%d fMsize:%d\n", fPdata, (Int_t )(fPdata-fPdatastartMsg),fMsize);
                  TCtr16Msg::DataType evtype = (TCtr16Msg::DataType) ((fWorkData >> 30) & 0x3);

                  boardDisplay->hDataTypes->Fill(evtype);

                  switch (evtype)
                  {
                    case TCtr16Msg::Data_Transient:
                      {
                        status=UnpackTrace(theBoard, boardDisplay, epoch);
                        if(status==1){
                          skipmessage=kTRUE;
                          fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                          break;
                        }
                        else if(status>=2)
                          {
                            goto end_of_event;
                          }

#ifdef Ctr16_IGNORE_CONTINUATION
                        // check here if a minimal trace would fit into rest of vulom container. if not, we discard the rest:
                        if(fMsize -(fPdata - fPdatastartMsg) < 6 )
                        {
                          skipmessage=kTRUE;
                          fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                        break;
                        }
#endif

                        if(fPdata - fPdatastartMsg >= fMsize )
                        {
                          fPdata = fPdatastartMsg + fMsize;
                          // align to header of next frame, since we are already further
                        }

                      }
                      break;

                    case TCtr16Msg::Data_Feature:
                      {
                        // find the feature JAM DEBUG
                        //fPar->fVerbosity=3;
                        //fPar->fSlowMotion=1;

                        UChar_t fullchannel = ((fWorkData >> 26) & 0xF);
                        UChar_t row = ((fWorkData >> 24) & 0x3);
                        boardDisplay->hMemoryRow->Fill(row);
                        TCtr16MsgFeature *fmsg = new TCtr16MsgFeature(fullchannel);
                        fmsg->SetEpoch(epoch);
                        fmsg->SetRow(row);
                        UShort_t ts = (fWorkData >> 5) & 0xFFF;
                        fmsg->SetTimeStamp(ts);
                        UChar_t finetime = (fWorkData & 0x3F);
                        fmsg->SetFineTime(finetime);
                        Ctr16_NEXT_DATAWORD;
                        UShort_t ampl = (fWorkData >> 16) & 0xFFFF;
                        fmsg->SetAmplitude(ampl);

                        Ctr16Warn("DFDFDF Data_Feature channel:%d ampl:%d ts:%d ftime:%d - pdata=0x%x , fWorkData=0x%x\n",
                                                                            fullchannel,ampl, ts, finetime, *fPdata, fWorkData);

                        // feature histograms here:
                        boardDisplay->hFeatureAmplitude[fullchannel]->Fill(ampl);
                        boardDisplay->hFeatureFineTime[fullchannel]->Fill(finetime);
                        UpdateDeltaTimes(theBoard, boardDisplay, fmsg, fullchannel); // evaluate delta T between messages also here
                        theBoard->AddMessage(fmsg, fullchannel);
                        SwitchDataAlignment();// next message will begin in lower 16 bit part of this data word
                        Ctr16_NEXT_DATAWORD;
                        // TODO: check if another feature event would fit into rest of vulom container
                        Ctr16Debug("DFDFDF restlen= %ld u32words \n",fMsize -(fPdata - fPdatastartMsg));

                      }
                      break;

                    default:
                        Ctr16Warn(
                            "############ found unknown data type 0x%x, *pdata=0x%x, fWorkdata=0x%x skip data frame %ld\n",
                            evtype, *fPdata, fWorkData, skipped_frames++)
                        ;
                        // JAMDEBUG
                        Int_t *cursor = fPdata;
                        while (cursor - fPdatastartMsg < fMsize)
                        {
                          Ctr16Warn("%p: 0x%x\t", cursor, *cursor);
                          cursor++;
                        }
                        Ctr16Warn("\n");
                        ////// ENDDEBUG
                        fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:

                        break;
                  } // switch eventtype
                } //  while (fPdata - fPdatastartMsg < fMsize)

                  // repeat loop over next event from here

            }
                break;

              case TCtr16Msg::Frame_Continuation:
                {
                    // TODO: continuation frames also for feature events?
                  if (!theBoard->fToBeContinued)
                  {
                    Ctr16Warn("!!!! Unexpected continuation frame with header 0x%x! Skip message %ld \n", header, skipped_frames++);
                    //closer debugging from here JAM DEBUG
//                        fPar->fVerbosity=3;
//                        fPar->fSlowMotion=1;
//                        Ctr16Warn("CCCCCC Dumping continuation frame:\n")
//                        Int_t* cursor= fPdata;
//                     while (cursor - fPdatastartMsg < fMsize)
//                     {
//                         Ctr16Warn("%p: 0x%x\t", cursor, *cursor);
//                         cursor++;
//                     }
//                     Ctr16Warn("\n");
                     ///////////////////
                    skipmessage=kTRUE;
                    fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                    break;
                  }
                  if (theBoard->fCurrentTraceEvent == 0)
                  {
                    // TODO: in this case might expect some feature event?
                    Ctr16Warn("!!!! Continuation frame without previous transient message! Skip message %ld\n",
                        skipped_frames++);
                    skipmessage=kTRUE;
                    fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                    break;
                  }

                  // check if epochs match with already accounted:
                  UInt_t epoch = header & 0xFFFFFF;
                  Ctr16Dump("CCCCCC continuation frame for epoch 0x%x \n", epoch);
                  if (epoch != theBoard->fCurrentTraceEvent->GetEpoch())
                  {
                    Ctr16Warn("!!!! Continuation frame with epoch mismatch:  previous 0x%x current 0x%x ! Skip message %ld\n",
                        epoch, theBoard->fCurrentTraceEvent->GetEpoch(), skipped_frames++);
                    skipmessage=kTRUE;
                    fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                    break;
                  }
                  Ctr16_NEXT_DATAWORD;
                  status = ExtractTrace(theBoard, boardDisplay);
                  if (status == 1)
                  {
                    skipmessage = kTRUE;
                    fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                    break;
                  }
                  else if (status >= 2)
                  {
                    goto end_of_event;
                  }

#ifdef Ctr16_SKIP_CONTINUATION_REST
                  // for the moment, we just skip rest of this frame JAM 20-02-2023
                   Ctr16Warn("continuation frame skips rest of payload %ld words\n", fMsize - (fPdata - fPdatastartMsg));
                  skipmessage=kTRUE;
                  //fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                  Ctr16Warn("SSSS: ");
                  while(fPdata-fPdatastartMsg < fMsize)
                  {
                    Ctr16Warn("%x\t",*fPdata++);
                  }
                  Ctr16Warn("\nCCCCCC\n");
                  /////// end skip debug
#else
                  // here try regular evaluation of addtional traces:
                  while (fPdata - fPdatastartMsg < fMsize)
                  {
                    status = UnpackTrace(theBoard, boardDisplay, epoch);
                    if (status == 1)
                    {
                      skipmessage = kTRUE;
                      fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                      break;
                    }
                    else if (status >= 2)
                    {
                      goto end_of_event;
                    };

                    if(fPdata - fPdatastartMsg >= fMsize )
                      {
                        fPdata = fPdatastartMsg + fMsize;
                          // align to header of next frame, since after last UnpackTrace we are already further
                      }

                  }

#endif

                }
                break;

              case TCtr16Msg::Frame_Error:
                {
                  UInt_t epoch = header & 0xFFFFFF;
                  while(fPdata-fPdatastartMsg < fMsize)
                    {
                      Ctr16_NEXT_DATAWORD;
                      UChar_t code =(fWorkData>>24) & 0xFF;
                      UShort_t ts = (fWorkData>>8) & 0xFFF;
                      Ctr16Dump("ERROR FRAME: code:0x%x epoch 0x%x timestamp:0x%x fWorkData:0x%x fWorkShift:%d *pdata:0x%x\n", code, epoch, ts, fWorkData, fWorkShift, *fPdata);
                      // TODO: create message for output event?
                      boardDisplay->hErrorcodes->Fill(code);
                      boardDisplay->hErrorTimestamp->Fill(ts);
                      fWorkShift+=8;
                      if(fWorkShift==32)
                        {
                          fWorkShift=0;
                          fPdata-=1; // rewind one word instead of shifting beyond word boundaries
                        }
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
                        boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Start);
                    }
                    else if ((ctype & TCtr16MsgWishbone::Ctrl_Threshold & 0x3) != 0) // since low 2 bit contain block number, just test for possible mask
                    {
                      isThresholdMessage = kTRUE;
                      boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Threshold);
                    }
                    else if(ctype == TCtr16MsgWishbone::Ctrl_Init)
                    {
                      boardDisplay->hMsgTypes->Fill(TCtr16Msg::Message_Init);
                    }
                    else
                    {
                      Ctr16Warn("WWW - found unknown wishbone slow control type 0x%x - skip message! \n",ctype);
                      fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                        break;
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

                    // find the feature JAM DEBUG
 //                                           fPar->fVerbosity=3;
 //                                           fPar->fSlowMotion=1;


                    // use threshold messages instead of wishbone container:
//
//                    Byte Bit 7  6  5  4  3  2  1  0
//                           | 0  1  0  1  0  0 | Blk|    Header mit Block Nummer
//                           |11    ..              4|    Mean value
//                           | 3        0|11        8|    Mean value |  FWHM
//                           | 7                    0|    FWHM
//                           |11                    4|    Threshold DAC
//                           | 3        0|11        8|    Threshold DAC  | Tracking DAC
//                           | 7                    0|    Tracking DAC
//                           |11                    4|    Baseline DAC Ch 0
//                           | 3        0|11        8|    Baseline DAC Ch 0  | Baseline DAC Ch 1
//                           | 7                    0|    Baseline DAC Ch 1
//                           |11                    4|    Baseline DAC Ch 2
//                           | 3        0|11        8|    Baseline DAC Ch 2  | Baseline DAC Ch 3
//                           | 7                    0|    Baseline DAC Ch 3
//



                    delete theMsg;
      //              Int_t bi=0;
                    while (fPdata - fPdatastartMsg < fMsize)
                    {

                      UChar_t block = (fWorkData >> 10) & 0x3;  // fWorkData is still at begin of message header
                      UShort_t mean = (fWorkData >>12) & 0xFFF;
                      UShort_t fwhm = fWorkData & 0xFFF;
                      Ctr16_NEXT_DATAWORD;
                      UShort_t thres = (fWorkData >> 20) & 0xFFF;
                      UShort_t track = (fWorkData >> 8) & 0xFFF;
                      fWorkShift+=8;
                      if(fWorkShift==32)
                      {
                        fWorkShift=0;
                        fPdata--;
                      }
                        Ctr16_NEXT_DATAWORD;
                      TCtr16MsgThreshold* msg[4]; // one frame contains info for 4 channels in block
                      UShort_t baseline[4];
                      baseline[0] = (fWorkData >> 20) & 0xFFF;
                      baseline[1] = (fWorkData >> 8) & 0xFFF;
                      baseline[2] = (fWorkData & 0xFF);
                      Ctr16_NEXT_DATAWORD;
                      baseline[2] |= (fWorkData >> 24) & 0xF;
                      baseline[3] |= (fWorkData) & 0xFFF;

                      for(Int_t c=0; c<4;++c){
                        msg[c]= new TCtr16MsgThreshold(0);
                        msg[c]->SetBlock(block);
                        msg[c]->SetBlockChannel(c);
                        msg[c]->SetMean(mean);
                        msg[c]->SetFWHM(fwhm);
                        msg[c]->SetTracking(track);
                        msg[c]->SetThreshold(thres);
                        msg[c]->SetBaseline(baseline[c]);
                        Int_t fullchan= msg[c]->GetChannel();
                        theBoard->AddMessage(msg[c], fullchan);

                        // histograms of threshold values for each channel
                        boardDisplay->hThresholdBaseline[fullchan]->Fill(mean);
                        boardDisplay->hThresholdNoise[fullchan]->Fill(fwhm);
                        boardDisplay->hThresholdSetting[fullchan]->Fill(thres);
                      }
                      // pretend that we could have another message in this frame:
                      Ctr16_NEXT_DATAWORD;
                    }
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
                      // here optional rest of data: only if data follows is set!
                      if (theMsg->GetAckCode() == TCtr16MsgWishbone::ACK_Data)
                        {
                        UChar_t val = fWorkData && 0xFF;    // fWorkData is still at begin of message header
                        theMsg->AddData(val);
                        Ctr16_NEXT_DATAWORD;
                        while (fPdata - fPdatastartMsg < fMsize)
                          {
                            for (Int_t shift = 24; shift >= 0; shift -= 8)
                            {
                              val = (fWorkData >> shift) & 0xFF;
                              theMsg->AddData(val);
                            }
                            Ctr16_NEXT_DATAWORD;
                          } // while
                        }
                      else
                      {
                        Ctr16_NEXT_DATAWORD;
                      }
                      boardDisplay->hWishboneSource->Fill(theMsg->GetSource());
                      // TODO: histogram of wishbone addresses and data words?

                    }
                    else
                    {
                      // slow control message types do not have address fields.
                      // we embed the containing text and dump it for fun;
                      UChar_t letter;
                      for (Int_t shift = 16; shift >= 0; shift -= 8)
                      {
                        letter = (fWorkData >> shift) & 0xFF;
                        theMsg->AddData(letter);
                      }

                      Ctr16_NEXT_DATAWORD;    //fPdata++;
                      while (fPdata - fPdatastartMsg < fMsize)
                      {

                        for (Int_t shift =24 ; shift >= 0; shift -= 8)
                        {
                          letter = (fWorkData >> shift) & 0xFF;
                          theMsg->AddData(letter);
                        }
                            Ctr16_NEXT_DATAWORD;
                      }
                      boardDisplay->lWishboneText->SetText(0.1, 0.9, theMsg->DumpMsg());
                      //Ctr16Warn("CCC Control message text:\n %s", theMsg->DumpMsg().Data());
                      printf("CCC See Control message of type:%d\n %s", theMsg->GetControlMessageType(),theMsg->DumpMsg().Data());
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
              Ctr16Warn("############  fPdata offset 0x%x has not yet reached message length 0x%x, correcting ,\n",
                  (unsigned int) (fPdata - fPdatastartMsg), fMsize);
              fPdata = fPdatastartMsg + fMsize;
            }

            //printf("EEEEEEEE  end of message payload: fPdata offset 0x%x msglength 0x%x\n",
            //             (unsigned int) (fPdata - fPdatastartMsg)end_of_event, fMsize);

          }    //  while ((fPdata - fPdatastartMsg) < fMsize)
        }    //if (((vulombytecount >> 28) & 0xF) == 0x4)
        else
        {
          Ctr16Warn("!!!!!!!!! Vulom container wrong bytecount header 0x%x - skipped!\n",vulombytecount);

          // JAM23-02-23 hunt for the bug
          //fPar->fVerbosity=3;
          //fPar->fSlowMotion=1;

        }
      }    // while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)
    }    // while fPdata - fPsubevt->GetDataField() <fLwords
  }    // while subevents

  UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
  Ctr16RawEvent->SetValid(kTRUE);    // to store

end_of_event:

  if (fPar->fSlowMotion || (source->GetCount() == fPar->fStopAtEvent))
  {
    Int_t evnum = source->GetCount();
     fPar->fSlowMotion=kTRUE;
    GO4_STOP_ANALYSIS_MESSAGE(
        "Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);

  }

  return kTRUE;
}

Int_t TCtr16RawProc::NextDataWord()
{
  Ctr16EVENTLASTMSG_CHECK_PDATA;
  Ctr16RAW_CHECK_PDATA;
  Ctr16MSG_CHECK_PDATA;
  if (fWorkShift == 0)
  {
    fWorkData = *fPdata++;
  }
  else
  {
    UInt_t mask = 0;
    for (UChar_t b = 0; b < (32-fWorkShift); ++b)
      mask |= (1 << b);
    fWorkData = (*(fPdata-1) << (32-fWorkShift));
    fWorkData |= (*fPdata >> fWorkShift) & mask;
    fPdata++;
    Ctr16EVENTLASTMSG_CHECK_PDATA;
    Ctr16RAW_CHECK_PDATA;
    Ctr16MSG_CHECK_PDATA;
  }

  // fWorkShift defines the n lower bits of the current subevent word that are used for next data word d
  // bits:
  //|31---------fWorkshift--0|  index
  //|------------- |ddddddddd|  (subevent word 1)
  //|dddddddddddddd|---------|  (subevent word 2)
 return 0;
}

void TCtr16RawProc::SwitchDataAlignment()
{
  if(fWorkShift == 0)
   {
    fWorkShift = 16;
   }
   else
   {
     fWorkShift = 0;
     fPdata--;
   }
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



Int_t TCtr16RawProc::UnpackTrace(TCtr16Board *board, TCtr16BoardDisplay *disp, UInt_t epoch )
{
  Int_t status=0;
  UChar_t fullchannel = ((fWorkData >> 26) & 0xF);
  UChar_t row = ((fWorkData >> 24) & 0x3);
  disp->hMemoryRow->Fill(row);


  if (board->fCurrentTraceEvent == 0)
     {
       board->fCurrentTraceEvent = new TCtr16MsgTransient(fullchannel);
     }
     else
     {
       Ctr16Warn("DDDD Data_Transient has previous message without continuation! discard incomplete data \n");
       delete board->fCurrentTraceEvent;
       board->fCurrentTraceEvent = new TCtr16MsgTransient(fullchannel);
     }

     // clear some flags, if we come here from not finalized event because of skipped messages:
     board->fTracedataIndex = 0;
     board->fToBeContinued = kFALSE;

     TCtr16MsgTransient *tmsg = board->fCurrentTraceEvent;
     tmsg->SetEpoch(epoch);
     tmsg->SetRow(row);
     UShort_t ts = fWorkData & 0xFFF;
     tmsg->SetTimeStamp(ts);
     board->fTracesize12bit = ((fWorkData >> 16) & 0xFF);
     // here check if we have a valid tracelenght:
     if( (board->fTracesize12bit != 16) && (board->fTracesize12bit != 32) && (board->fTracesize12bit != 64))
     {
       Ctr16Warn("Data_Transient with illegal 12bit trace size: %d,  pdata=0x%x , fWorkData=0x%x -  skip rest of message\n",
                board->fTracesize12bit, *fPdata, fWorkData);
       return 1;
     }



     board->fTracesize32bit = board->fTracesize12bit * 3 / 8;
     // account partially filled last data word here:
      if ((Float_t) (board->fTracesize12bit) * 3.0 / 8.0 > (Float_t) board->fTracesize32bit)
                              board->fTracesize32bit++;

      Ctr16Debug("DDDD Data_Transient with 12bit trace size: %d, expect %d data words. pdata=0x%x , fWorkData=0x%x\n",
          board->fTracesize12bit, board->fTracesize32bit, *fPdata, fWorkData);

     Ctr16Dump("Data_Transient - channel:%d size12:%d size32:%d\n", fullchannel,
         board->fTracesize12bit, board->fTracesize32bit);
     if ((fPdata - fPdatastart -3) + board->fTracesize32bit > fLwords)
     {
      Ctr16Warn(
           "Transient Event header error: 12 bit size %d does not fit into mbs subevent buffer of restlen %ld words \n",
           board->fTracesize32bit, fLwords - (fPdata - fPdatastart -3 ));
     GO4_SKIP_EVENT
     }
     disp->hDatawords->Fill(board->fTracesize12bit);
     disp->hChannels->Fill(fullchannel);
     //Ctr16_NEXT_DATAWORD;   // trace begins after event header
     status=NextDataWord(); // trace begins after event header
     if(status!=0) return status;
     status=ExtractTrace(board, disp);
  return status;

}




Int_t TCtr16RawProc::ExtractTrace(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  Int_t status=0;
  Int_t payloadwords = board->fTracesize32bit - board->fTracedataIndex;
  if(fMsize -(fPdata - fPdatastartMsg)+1 < payloadwords)
  {
    Ctr16Debug("ExtractTrace sees rest of buffer %ld too short for desired payload %d, expect continuation! \n", fMsize -(fPdata - fPdatastartMsg) - 1, payloadwords);
    payloadwords = fMsize -(fPdata - fPdatastartMsg) +1;
    board->fToBeContinued = kTRUE;
  }
  else
  {
    board->fToBeContinued = kFALSE;
  }

  for (Int_t w = 0; w < payloadwords; ++w)
  {
    Ctr16EVENTLASTMSG_CHECK_PDATA;
    Ctr16RAW_CHECK_PDATA;
    Ctr16MSG_CHECK_PDATA;    // should not be, but who knowns
    board->fTracedata[board->fTracedataIndex] = fWorkData;    //*fPdata++;
    status=NextDataWord(); // check for initial message of size 1 ?
    if(status!=0) return status; // pass on error states when scanning payload
    Ctr16Debug("Transient_Event copies 32bit data[%d]=0x%x \n", board->fTracedataIndex,
        board->fTracedata[board->fTracedataIndex]);
    board->fTracedataIndex++;
    if (board->fTracedataIndex > Ctr16_TRACEWORDS)
    {
      printf("?????? trace payload %d beyond maximum tracewords %d, limiting...\n", board->fTracedataIndex,
      Ctr16_TRACEWORDS);
      board->fTracedataIndex = Ctr16_TRACEWORDS;
    }
  }
  // here align again for next trace:

      Ctr16Debug("ExtractTrace has copied %d payloadwords, fTracedataIndex=%d, pdata=%p, *pData=0x%x, fWorkData=0x%x , fWorkShift=%d, fToBeContinued=%d\n",
      payloadwords,board->fTracedataIndex,fPdata,*fPdata, fWorkData, fWorkShift, board->fToBeContinued);

      Int_t usedwords=board->fTracesize32bit-1;
      if(usedwords==0) usedwords=1;
      fWorkShift= (32 -(board->fTracesize12bit * 12)  % (usedwords * 32));



      Ctr16Debug("ExtractTrace after shift alignment, pdata=%p, *pData=0x%x, fWorkData=0x%x , fWorkShift=%d\n",
            fPdata,*fPdata, fWorkData, fWorkShift);




  if (board->fToBeContinued)
  {
    // we expect further data in the continuation frame...
    Ctr16Dump("Transient_Event expects continuation frame after %d words... \n", board->fTracedataIndex); //Ctr1Dump
  }
  else
  {
    // we are ready inside the first data frame.
    FinalizeTrace(board, disp);
  }
  return 0;
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
    Ctr16Debug("Transient_Event jstart:%d jend:%d dix_start:%d dix_end:%d k_start:%d k_end:%d \n", (int ) j_start,
        (int ) j_end, dix_start, dix_end, (int ) k_start, (int ) k_end);
    if (dix_start == dix_end)
    {
      // easy case, sample is inside one evdata word:
      val = (board->fTracedata[dixoffset + dix_start] >> k_end) & 0xFFF;
      Ctr16Debug("Transient_Event sees NON spanning value\n");
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
      Ctr16Debug("Transient_Event sees SPANNING value\n");
    }
    else
    {
      // never come here
      GO4_STOP_ANALYSIS_MESSAGE("NEVER COME HERE: mismatch of evsample indices - dix_end:%d and dix_start:%d", dix_end,
          dix_start)

    }
    Ctr16Debug("Transient_Event set bin:%d to val:%d\n", bin, val);
    //board->fCurrentTraceEvent->SetTraceData(bin, val);
    board->fCurrentTraceEvent->AddTraceData(val);
    j_start = j_end;    // next 12 bit word
  }    // for bin

  UChar_t chan = board->fCurrentTraceEvent->GetChannel();
  board->AddMessage(board->fCurrentTraceEvent, chan);

  Ctr16Dump("FinalizeTrace for binlen=%d - channel=%d, snapshotcount=%d\n", binlen, chan, disp->fSnapshotcount[chan]);

  // now display complete trace:

  TH1 *tracesnapshot = 0;
  TH2 *trace2d = 0;
  if (disp->fSnapshotcount[chan] < Ctr16_MAXSNAPSHOTS)
  {
    tracesnapshot = disp->hTraceSnapshots[chan][disp->fSnapshotcount[chan]];
    //std::cout<< "getting histogram for  channel "<< (int) chan<<" snapshot:"<<disp->fSnapshotcount[chan]]<<", binlen="<<binlen<<", tracelen="<< tracelength<< std::endl;
    trace2d = disp->hTraceSnapshot2d[chan];
  }
  if (chan < Ctr16_CHANNELS)
    disp->hTrace[chan]->Reset(""); //avoid mixing of traces in same mbs event, only keep latest

  for (Int_t bin = 0; bin < binlen; ++bin)
  {
    Short_t val = board->fCurrentTraceEvent->GetTraceData(bin);

#ifdef    Ctr16_BOTHPOLARITY
    // convert raw data to signed 8bit (2 complement) representation:
    if (val > 0x7FF)
      val = val - 0x1000;
#endif
    if (tracesnapshot && bin < tracelength)
    {
      tracesnapshot->SetBinContent(bin + 1, val);
      //std::cout<< " --- Set bin:"<<bin<<" to content:"<<val << std::endl;
    }
    if (trace2d)
      trace2d->Fill(bin, disp->fSnapshotcount[chan], val);

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
  } // for bin
  disp->fSnapshotcount[chan]++;
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


