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

// for top level loop only, not used anymore JAM 27-03-23
#if 0
#define Ctr16_NEXT_DATAWORD \
status=NextDataWord(); \
if(status==1) {skipmessage=kTRUE;  fPdata = fPdatastartMsg + fMsize;  break;} \
if(status>=2) goto end_of_event;
#endif

// for subfunctions:
#define Ctr16_NEXT_DATAWORD_RETURN \
status=NextDataWord(); \
if(status!=0) return status;

// for top level loop:
#define Ctr16_CALL_UNPACKER(X) \
status=X;\
if(status==1) {skipmessage=kTRUE;  fPdata = fPdatastartMsg + fMsize;  break;} \
if(status>=2) return kFALSE;

// for subfunctions:
#define Ctr16_CALL_UNPACKER_RETURN(X) \
status=X; \
if(status!=0) return status;

//***********************************************************
TCtr16RawProc::TCtr16RawProc() :
    TGo4EventProcessor(), fPar(0),fMbsEvt(0), fPsubevt(0), Ctr16RawEvent(0), fPdata(0), fPdatastart(0), fLwords(0),
        fPdatastartMsg(0), fMsize(0), fWorkData(0), fWorkShift(0)
#ifdef Ctr16_DO_MEMORYSAMPLES
        ,fEventCounter(0)
#endif
{
}

//***********************************************************
// this one is used in standard factory
TCtr16RawProc::TCtr16RawProc(const char *name) :
    TGo4EventProcessor(name), fPar(0), fMbsEvt(0), fPsubevt(0), Ctr16RawEvent(0), fPdata(0), fPdatastart(0), fLwords(0),
        fPdatastartMsg(0), fMsize(0), fWorkData(0), fWorkShift(0)
#ifdef Ctr16_DO_MEMORYSAMPLES
        ,fEventCounter(0)
#endif

{
  skipped_events = 0;
  skipped_frames = 0;
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
  //Int_t status = 0;
  Ctr16RawEvent = (TCtr16RawEvent*) target;
  Ctr16RawEvent->SetValid(kFALSE);    // not store
  fMbsEvt = (TGo4MbsEvent*) GetInputEvent();
  if (fMbsEvt == 0)
  {
    cout << "AnlProc: no input event !" << endl;
    return kFALSE;
  }
  UShort_t triggertype = fMbsEvt->GetTrigger();

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

#ifdef Ctr16_DO_MEMORYSAMPLES
    if(fPar->fMemorySampleBoard>0 &&  TGo4Analysis::Instance()->IsNewInputFile())
    {
      fEventCounter=0;
      TGo4Analysis::Instance()->ClearObjects("Histograms");
      printf ("NNNNNNNNNN Found new input file: clearing histograms and resetting the counter!!\n");
    }
    fEventCounter++; // local counter, independent of MBS header
#endif




  fMbsEvt->ResetIterator();
  while ((fPsubevt = fMbsEvt->NextSubEvent()) != 0)
  {    // loop over subevents
    fPdata = fPsubevt->GetDataField();
    fLwords = fPsubevt->GetIntLen();

#ifdef    Ctr16_USE_VULOM
    if(!ProcessVulomSubevent()) goto end_of_event;
#else
    if(!ProcessGosipSubevent()) goto end_of_event;
#endif
  }    // while subevents

  UpdateDisplays();    // we fill the raw displays immediately, but may do additional histogramming later
  end_of_event:

  if (fPar->fSlowMotion || (fMbsEvt->GetCount() == fPar->fStopAtEvent))
  {
    Int_t evnum = fMbsEvt->GetCount();
    fPar->fSlowMotion = kTRUE;
    GO4_STOP_ANALYSIS_MESSAGE(
        "Stopped for slow motion mode after MBS event count %d. Click green arrow for next event. please.", evnum);

  }

  return kTRUE;
}



  Bool_t TCtr16RawProc::ProcessVulomSubevent()
  {
    Int_t status = 0;
    Bool_t skipmessage = kFALSE;
    while (fPdata - fPsubevt->GetDataField() < fLwords)
     {

      if ((unsigned) *fPdata == 0xbad00bad)
        {
          GO4_SKIP_EVENT_MESSAGE("**** TCtr16RawProc: Found BAD mbs event (marked 0x%x), skip it.", (*fPdata));
        }
       // vulom status word:
       Ctr16RawEvent->fVULOMStatus = *fPdata++;
       // from get4++ JAM2020: need to check if status word has valid format here:
       if (((Ctr16RawEvent->fVULOMStatus >> 12) & 0x3) != 0x3)
       {
         Ctr16Dump("VULOM: wrong vulom status word: 0x%x skip it.. \n", Ctr16RawEvent->fVULOMStatus);

         continue;
       }

       //event trigger counter:
       Ctr16RawEvent->fSequenceNumber = *fPdata++;
       // data length
       Ctr16RawEvent->fDataCount = *fPdata++;
       //1 + *fPdata++; // from get4++ unpacker: payload is one more according to f_user_readout JAM 10-22
       Ctr16Dump("VULOM: status: 0x%x counter: 0x%x length: 0x%x \n", Ctr16RawEvent->fVULOMStatus,
           Ctr16RawEvent->fSequenceNumber, Ctr16RawEvent->fDataCount);

       if (Ctr16RawEvent->fDataCount > (fLwords - 3))
       {
         // do not put this into loggin queue for gui...
         printf(
             "**** TCtr16RawProc: Mismatch with subevent len %d and data count 0x%8x - vulom status:0x%x seqnum:0x%x - eventnumber=%d \n",
             fLwords, Ctr16RawEvent->fDataCount, Ctr16RawEvent->fVULOMStatus, Ctr16RawEvent->fSequenceNumber, fMbsEvt->GetCount());




         GO4_SKIP_EVENT
         ;
         // avoid that we run optional second step on invalid raw event!
       }

       fPdatastart = fPdata;    // remember begin of asic payload data section
       fPdata++;    // skip first  word?
       fPdata++;    // skip another word with 0 bytecount flag?
       // now fetch boardwise subcomponents for output data and histograming:
       Int_t sfp = 0;    // JAM here we later could evaluate a board identifier mapped to a slot/sfp number contained in subevent
       Int_t dev=0;
       UInt_t brdid = fPar->fBoardID[sfp][dev];    // get hardware identifier from "DAQ link index" number
       TCtr16Board *theBoard = Ctr16RawEvent->GetBoard(brdid);
       if (theBoard == 0)
       {
         GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as subevent, sfp:%d dev:%d", brdid,
             sfp,dev);

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
       //printf("EEEEEEEEEEE Event %d begins, data count= %d \n",fMbsEvt->GetCount(), Ctr16RawEvent->fDataCount);
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
           if (((vulombytecount & 0xFF) % sizeof(Int_t)) != 0)
             fMsize++;    // ?test this!
           Ctr16Debug("VVVV Vulom container, header 0x%x with message size: %d \n", vulombytecount, fMsize);
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
                   Ctr16_CALL_UNPACKER(HandleDataFrame(theBoard, boardDisplay));
                 }
                 break;

               case TCtr16Msg::Frame_Continuation:
                 {
                   Ctr16_CALL_UNPACKER(HandleContinuationFrame(theBoard, boardDisplay));
                 }
                 break;

               case TCtr16Msg::Frame_Error:
                 {
                   Ctr16_CALL_UNPACKER(HandleErrorFrame(theBoard, boardDisplay));
                 }
                 break;

               case TCtr16Msg::Frame_Wishbone:
                 // wishbone response (error message)
                 {
                   Ctr16_CALL_UNPACKER(HandleWishboneFrame(theBoard, boardDisplay));
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
                   (unsigned int ) (fPdata - fPdatastartMsg), fMsize);
               fPdata = fPdatastartMsg + fMsize;
             }

             //printf("EEEEEEEE  end of message payload: fPdata offset 0x%x msglength 0x%x\n",
             //             (unsigned int) (fPdata - fPdatastartMsg)end_of_event, fMsize);

           }    //  while ((fPdata - fPdatastartMsg) < fMsize)
         }    //if (((vulombytecount >> 28) & 0xF) == 0x4)
         else
         {
           Ctr16Warn("!!!!!!!!! Vulom container wrong bytecount header 0x%x - skipped!\n", vulombytecount);

           // JAM23-02-23 hunt for the bug
            //fPar->fVerbosity=3;
            //fPar->fSlowMotion=1;

         }
       }    // while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)
     }    // while fPdata - fPsubevt->GetDataField() <fLwords
    return kTRUE;
  }

Bool_t TCtr16RawProc::ProcessGosipSubevent()
{
  /////////////////// TODO JAM 27-03-2023
  //GO4_STOP_ANALYSIS_MESSAGE("**** TCtr16RawProc: ProcessGosipSubevent not yet implemented! Stopped.");
  //return kTRUE;
  static int badcounter = 0;
  ////////////////////////////////////////////////////////////////////////////////
  /// Here begin regular kilom data from token readout: JAM skeleton taken from febex/poland
  Int_t status = 0;
  Bool_t skipmessage = kFALSE;
  while (fPdata - fPsubevt->GetDataField() < fLwords)
  {

    if ((*fPdata & 0xffff0000) == 0xadd00000)    // we have padding word (initial data of sfp, skip it:)
    {
      Int_t dma_padd = (*fPdata & 0xff00) >> 8;
      Int_t cnt(0);
      while (cnt < dma_padd)
      {
        if ((*fPdata & 0xffff0000) != 0xadd00000)
        {
          //TGo4Log::Error("Wrong padding format - missing add0");
          GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong padding format - missing add0");
          // avoid that we run second step on invalid raw event!
          //return kFALSE;
        }
        if (((*fPdata & 0xff00) >> 8) != dma_padd)
        {
          //TGo4Log::Error("Wrong padding format - 8-15 bits are not the same");
          GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong padding format - 8-15 bits are not the same");
          // avoid that we run second step on invalid raw event!
          //return kFALSE;
        }
        if ((*fPdata & 0xff) != cnt)
        {
          //TGo4Log::Error("Wrong padding format - 0-7 bits not as expected");
          GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Wrong padding format - 0-7 bits not as expected");
          // avoid that we run second step on invalid raw event!
          //return kFALSE;
        }
        fPdata++;
        cnt++;
      }
      continue;
    }
    else if ((unsigned) *fPdata == 0xbad00bad)
    {

      printf("############ found bad event %d  at MBS count  %d\n", badcounter,fMbsEvt->GetCount());
      fMbsEvt->PrintEvent();
      badcounter++;
      GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Found BAD mbs event (marked 0x%x), skip it.", (*fPdata));


    }
    else if ((*fPdata & 0xff) != 0x34)    // regular channel data
    {
      //GO4_STOP_ANALYSIS_MESSAGE("Wrong optic format - 0x34 are expected0-7 bits not as expected");
      //TGo4Log::Error("Wrong optic format 0x%x - 0x34 are expected0-7 bits not as expected", (*fPdata & 0xff));
      //GO4_SKIP_EVENT_MESSAGE
      Ctr16Warn("**** TQFWRawProc: Wrong optic format 0x%x - 0x34 are expected - 0-7 bits not as expected",
          (*fPdata & 0xff));
      GO4_SKIP_EVENT;

      // avoid that we run second step on invalid raw event!
      //return kFALSE;
    }

    fPdatastart = fPdata;    // remember begin of optic payload data section
    // unsigned trig_type   = (*fPdata & 0xf00) >> 8;
    unsigned sfp_id = (*fPdata & 0xf000) >> 12;
    unsigned device_id = (*fPdata & 0xff0000) >> 16;
    // unsigned channel_id  = (*fPdata & 0xff000000) >> 24;
     fPdata++;

    UInt_t opticlen = *fPdata++;
    if (opticlen > fLwords * sizeof(Int_t))
    {
      //TGo4Log::Error("Mismatch with subevent len %d and optic len %d", fLwords * 4, opticlen);
      GO4_SKIP_EVENT_MESSAGE("**** TQFWRawProc: Mismatch with subevent len %d and optic len %d",
          fLwords * sizeof(Int_t), opticlen);
      // avoid that we run second step on invalid raw event!
      //return kFALSE;
    }

    Ctr16RawEvent->fDataCount = opticlen / sizeof(Int_t);

    //int eventcounter = *fPdata;
    //event trigger counter:
    //Ctr16RawEvent->fSequenceNumber = *fPdata++;  // JAM3-04-2023 not in data!

    //TGo4Log::Info("Internal Event number 0x%x", eventcounter);
    // board id calculated from SFP and device id:
    UInt_t brdid = fPar->fBoardID[sfp_id][device_id];
    TCtr16Board *theBoard = Ctr16RawEvent->GetBoard(brdid);
    if (theBoard == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as subevent, sfp:%d dev:%d", brdid,
          sfp_id, device_id);

      return kFALSE;
    }
    TCtr16BoardDisplay *boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE("Configuration error: Board id %d does not exist as histogram display set!", brdid);
      return kFALSE;
    }
    boardDisplay->ResetDisplay();

    // TODO: here evaluate data contents



    // process

    while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)
    {

      // before scanning messages, find out containerlenght between leading and trailing headers (0xaf1 -> 0xbf1) - this is real "data count"?
      Int_t contheader = *fPdata;
      Int_t *contstart = fPdata;
      Int_t contlen = 0;
      if (((contheader >> 24) & 0xFF) == 0xaf)    // check for next container  header
      {
        //Int_t chipid = (vulombytecount >> 16) & 0xFF;
        //boardDisplay->hChipId->Fill(chipid); // TODO: chip id in the data?
        // since message header does not give us length, we look for the trail marker to find out size:
        Int_t contcounter = contheader & 0xFFFF; // check counter + trigger + bufid
        Char_t conttrigger= (contheader >> 20) & 0xF; // mbs trigger type
        Char_t contbufid= (contheader >> 16) & 0xF; // gosip buffer id (0 or 1)
        Int_t trailcounter = 0;
        Char_t trailtrigger=0;
        Char_t trailbufid=0;
        for (contlen = 0; contlen < Ctr16RawEvent->fDataCount; ++contlen)
        {
          Int_t conttrailer = *(fPdata + contlen);
          if (((conttrailer >> 24) & 0xFF) == 0xbf)
          {
            trailcounter = conttrailer & 0xFFFF; // check counter + trigger + bufid
            trailtrigger = (conttrailer >> 20) & 0xF;
            trailbufid= (conttrailer >> 16) & 0xF;

            if ((contcounter == trailcounter) && (conttrigger==trailtrigger) && (contbufid==trailbufid))
            {
              Ctr16Debug("GGGG Gosip message container found correct trailer 0x%x after header 0x%x at size: %d\n", conttrailer, contheader, contlen);
              break;
            }
            }
        }
        if (contlen >= Ctr16RawEvent->fDataCount)
        {
          Ctr16Dump(
              "GGGG Gosip message container: can not find trailer for header 0x%x after size: %d - skip message \n",
              contheader, contlen);
          fPdata += contlen; // will end gosip container loop
          continue;
        }
       ///////////////////////////////////// OK
        // now loop over this container
        // first evaluate the 3 container header words:
        // contheader (siehe oben)
        fPdata++;
        // chipid(1 byte) | TS3 (24bit)
        Int_t idheader=*fPdata++;
        Int_t chipid =   (idheader >> 24) & 0xF;
        boardDisplay->hChipId->Fill(chipid);
        ULong_t timestamp_fpga = (idheader & 0xFFFFFFUL);
        // TS2 (32bit) -  TS= 56bit FPGA timestamp, counts with 10 ns units since begin of readout
        timestamp_fpga |= *fPdata++;

        Ctr16Debug("GGGG Gosip message container, header 0x%x with size: %d, evcounter:0x%x trigger:%d bufid:%d chipid:%d fpgaTS:0x%lx\n", contheader, contlen,
                      (contcounter & 0xFFFF), conttrigger, contbufid,chipid, timestamp_fpga);


        ////////////////////////////
        // find out length of next "message" between "separators" 0x0000017e -> msize  :
        while ((fPdata - contstart) < contlen)
        {
          Int_t separator = *fPdata++;
          if ((separator & 0xFFFFFFFF) != 0x17E)
          {
            Ctr16Debug("GGGG no message separator 0x17E after GOSIP container header, instead 0x%x - try next\n",
                separator);
            continue;    // find first marker
          }

          // OK, now evaluate message length from next separator:

          for (fMsize = 0; fPdata + fMsize - contstart < contlen; ++fMsize)
          {
            Int_t nextsep = *(fPdata + fMsize);
            if ((nextsep & 0xFFFFFFFF) == 0x17E)
              break;
          }
          if(fPdata + fMsize - contstart >=contlen)
          {
            Ctr16Debug("GGGG no trailing separator 0x17E at %d words after the first 0x%x, exceeding kilom container! skip \n",
                          fMsize, separator);

            fPdata+=contlen; // ends inner loop, try for next container if any in this gosip payload
            continue;
          }

          fMsize--; // JAM 25-05-2023: ignore trailing CRC word before next separator!

          Ctr16Dump("GGGG Gosip message container, separator 0x%x with message size: %d \n", separator, fMsize);
          fPdatastartMsg = fPdata;    // payload after separator
          //////////////////////
          // the rest is almost same as for vulom, but frame data is initally not 32 bit aligned:
          // 0x0000017e - 7e is "flag" from chip data  frame
          // 0xFF73PPPP - FF - DAdr destination address from chip
          //            - 73 - SAdr source addddress
          //            - PPPP payload begin with frame header, e.g 80PP for error frame
          //
          fPdata++;    // for NextDataWord() below, forward to next but one word
          fWorkShift = 16;    // begin frame handling after chip header
          Bool_t first=kTRUE;
          while ((fPdata - fPdatastartMsg) < fMsize)
          {

            //inside message loop, we use aligned data words:
            if(first) {
              NextDataWord(); // need to call this here, otherwise will skip all short frames
              first=kFALSE;
            }
            // evaluate message type from header:
            Int_t header = fWorkData;
            // not we do not increment fPdata here, do this inside msg types
            Int_t frametype = ((header >> 30) & 0x3);
            boardDisplay->hFrameTypes->Fill(frametype);
            //printf("MMMMMMMM message type %d \n",mtype);
            //Ctr16Dump("GGGG Gosip message container has frame type: %d \n", frametype);
            switch (frametype)
            {
              case TCtr16Msg::Frame_Data:
                {
                  Ctr16_CALL_UNPACKER(HandleDataFrame(theBoard, boardDisplay));
                }
                break;

              case TCtr16Msg::Frame_Continuation:
                {
                  Ctr16_CALL_UNPACKER(HandleContinuationFrame(theBoard, boardDisplay));
                }
                break;

              case TCtr16Msg::Frame_Error:
                {
                  Ctr16_CALL_UNPACKER(HandleErrorFrame(theBoard, boardDisplay));
                }
                break;

              case TCtr16Msg::Frame_Wishbone:
                // wishbone response (error message)
                {
                  Ctr16_CALL_UNPACKER(HandleWishboneFrame(theBoard, boardDisplay));
                }
                break;

              default:
                Ctr16Warn("GGGG Gosip message container found unknown frame  type %d \n", frametype);
                fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:
                break;
            };    // switch

            if (!skipmessage && (fPdata - fPdatastartMsg) < fMsize)
            {
              // never come here if messages are treated correctly!
              Ctr16Warn("############  fPdata offset 0x%x has not yet reached message length 0x%x, correcting ,\n",
                  (unsigned int ) (fPdata - fPdatastartMsg), fMsize);
              fPdata = fPdatastartMsg + fMsize;
            }

            //printf("EEEEEEEE  end of message payload: fPdata offset 0x%x msglength 0x%x\n",
            //             (unsigned int) (fPdata - fPdatastartMsg)end_of_event, fMsize);

          }    //  while ((fPdata - fPdatastartMsg) < fMsize)

          fPdata++;  // JAM 25-05-2023: we have ignored trailing CRC word before next separator at fMsize, so skip it now.
        }    //    while ((fPdata - contstart) < contlen)

      }    // if (((contheader >> 20) & 0xFFF) == 0xaf1)
      else
      {
        Ctr16Warn("!!!!!!!!! GOSIP buffer container wrong header 0x%x - skipped!\n", contheader);
        fPdata++;
      }
    }    // while ((fPdata - fPdatastart) < Ctr16RawEvent->fDataCount)

  }    // while(fPdata - fPsubevt->GetDataField() < fLwords)

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
    for (UChar_t b = 0; b < (32 - fWorkShift); ++b)
      mask |= (1 << b);
    fWorkData = (*(fPdata - 1) << (32 - fWorkShift));
    fWorkData |= (*fPdata >> fWorkShift) & mask;
    fPdata++;
// JAM 23-05-2023: do not check here, we might need to process still next wordata!
//    Ctr16EVENTLASTMSG_CHECK_PDATA;
//    Ctr16RAW_CHECK_PDATA;
//    Ctr16MSG_CHECK_PDATA;
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
  if (fWorkShift == 0)
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


#ifdef Ctr16_DO_MEMORYSAMPLES
  // for labtest: write fine time histogram bins to tree when we have enough statistics
  Ctr16RawEvent->SetValid(kFALSE);    // do not store by default
  UInt_t brdid = fPar->fMemorySampleBoard;    // use defined board under test
  if (brdid > 0)
  {
    TCtr16BoardDisplay* boardDisplay = GetBoardDisplay(brdid);
    if (boardDisplay == 0)
    {
      GO4_SKIP_EVENT_MESSAGE(
          "Configuration error in UpdateDisplays: Board id %d does not exist as histogram display set!", brdid);
      return kFALSE;
    }

    if (fEventCounter == fPar->fMemorySampleStatsLimit)
    {
      // we reach the point to get the accuumlated samples:
      TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();

      s_filhe* head = source->GetMbsSourceHeader();
      if (head)
      {
        char *filename = head->filhe_file;
        Ctr16RawEvent->fLmdFileName = filename;    //
        TString(filename, head->filhe_file_l);
        //We expect files of form BaselineTraces_23-03-28-1506_Clock162.0MHz_380.lmd
        Int_t year=0, month=0, day=0, time=0, infonum=0;
        Float_t clockfreq=0;

        Int_t rev = sscanf(filename, "BaselineTraces_%d-%d-%d-%d_Clock%fMHz_%x", &year, &month, &day, &time, &clockfreq,
            &infonum);
        if (rev > 0)
        {
          // JAM 28-03-23: we copy local ints of scanf to more narrow variables for tree here:
          Ctr16RawEvent->fYear= year & 0xFF;
          Ctr16RawEvent->fMonth= month & 0xFF;
          Ctr16RawEvent->fDay= day & 0xFF;
          Ctr16RawEvent->fTime= time & 0xFFFF;
          Ctr16RawEvent->fClockFreq=clockfreq;
          Ctr16RawEvent->fInfonumber=infonum & 0xFFFF;

          printf("Got From filename: %s :year:%d month:%d day:%d time:%d freq:%E MHz and id:0x%x \n", filename,
              Ctr16RawEvent->fYear, Ctr16RawEvent->fMonth, Ctr16RawEvent->fDay,  Ctr16RawEvent->fTime,
              Ctr16RawEvent->fClockFreq,  Ctr16RawEvent->fInfonumber);
        }
        else
        {
              printf("Error %d when scanning filename: %s", rev, filename);
        } //BaselineTraces_
      } // head
      //Get4ppWarn
      printf("UpDateDisplays: writing channel infos to output event after %d events, filename:%s\n", fEventCounter,
          Ctr16RawEvent->fLmdFileName.Data());

         // here evaluation of entries,  mean and sigma:


        Double_t entries=0,mu=0, sigma=0;

        for (int bl = 0; bl < Ctr16_BLOCKS; ++bl)
         {
           for (int row = 0; row < Ctr16_CHANNELROWS; ++row)
           {
             for (int cell = 0; cell < Ctr16_MEMORYCELLS; ++cell)
             {
               TH1* his=boardDisplay->hADCValuesPerCell[bl][row][cell];
               if(his==0) continue; // skip not  used
               entries=his->GetEntries();
               mu=his->GetMean();
               sigma=his->GetStdDev(); // TODO: maybe define range with go4 condition here?
               Ctr16RawEvent->fADCMean[bl][row][cell]=mu;
               Ctr16RawEvent->fADCSigma[bl][row][cell]=sigma;
               Ctr16RawEvent->fADCEntries[bl][row][cell]=entries;
             }
           }
         }

      // set output event valid for tree storage
      Ctr16RawEvent->SetValid(kTRUE);
    }
    else if (fEventCounter > fPar->fMemorySampleStatsLimit)
    {
      // after sample was stored, just skip all other events in file
      Ctr16RawEvent->SetValid(kFALSE);
      if((fEventCounter%500 ==0))printf("SSSS ignoring event %d from %s ...\n", fEventCounter, Ctr16RawEvent->fLmdFileName.Data());
       //GO4_SKIP_EVENT;
      // JAM DEBUG
//      fEventCounter=0;
//      TGo4Analysis::Instance()->ClearObjects("Histograms");
//      printf ("RRRR DEBUG: clearing histograms and resetting the counter!!\n");
    }
    else
    {
      // if not yet at sampling point, do not store
      Ctr16RawEvent->SetValid(kFALSE);
    }
  }    // boardid

#else
  Ctr16RawEvent->SetValid(kTRUE);    // everything to store for regular mode
#endif







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

Int_t TCtr16RawProc::HandleDataFrame(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  Int_t status = 0;
  Int_t header = fWorkData;
  // first get epoch number
  UInt_t epoch = header & 0xFFFFFF;
  Ctr16_NEXT_DATAWORD_RETURN
  ;
  Ctr16Dump("FFFFFF Data Frame with epoch 0x%x - fPdata:%p, fPdatastartMsg:%p\n", epoch, fPdata, fPdatastartMsg);
  // scan payload for events:
  while (fPdata - fPdatastartMsg < fMsize)
  {
    Ctr16Dump(" Data Frame loop: fPdata:%p, cursor:%d fMsize:%d\n", fPdata, (Int_t )(fPdata - fPdatastartMsg), fMsize);
    TCtr16Msg::DataType evtype = (TCtr16Msg::DataType) ((fWorkData >> 30) & 0x3);

    disp->hDataTypes->Fill(evtype);

    switch (evtype)
    {
      case TCtr16Msg::Data_Transient:
        {
          Ctr16_CALL_UNPACKER_RETURN(UnpackTrace(board, disp, epoch));
        }
        break;

      case TCtr16Msg::Data_Feature:
        {
          // find the feature JAM DEBUG
          //fPar->fVerbosity=3;
          //fPar->fSlowMotion=1;
          Ctr16_CALL_UNPACKER_RETURN(UnpackFeature(board, disp, epoch));
        }
        break;

      default:
        Ctr16Warn("############ found unknown data type 0x%x, *pdata=0x%x, fWorkdata=0x%x skip data frame %ld\n",
            evtype, *fPdata, fWorkData, skipped_frames++)
        ;
        // JAMDEBUG
        Int_t *cursor = fPdata;
        while (cursor - fPdatastartMsg < fMsize)
        {
          Ctr16Warn("%p: 0x%x\t", cursor, *cursor);
          cursor++;
        }
        Ctr16Warn("\n")
        ;
        ////// ENDDEBUG
        fPdata = fPdatastartMsg + fMsize;    // do not skip complete event, but just the current message:

        break;
    }    // switch eventtype
  }    //  while (fPdata - fPdatastartMsg < fMsize)

  // repeat loop over next event from here

  return status;
}

Int_t TCtr16RawProc::HandleContinuationFrame(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  Int_t status = 0;
  Int_t header = fWorkData;
  // TODO: continuation frames also for feature events?
  if (!board->fToBeContinued)
  {
    Ctr16Warn("!!!! Unexpected continuation frame with header 0x%x! Skip message %ld \n", header, skipped_frames++);
    return 1;        // do not skip complete event, but just the current message:
  }
  if (board->fCurrentTraceEvent == 0)
  {
    // TODO: in this case might expect some feature event?
    Ctr16Warn("!!!! Continuation frame without previous transient message! Skip message %ld\n", skipped_frames++);
    return 1;    // do not skip complete event, but just the current message:
  }

  // check if epochs match with already accounted:
  UInt_t epoch = header & 0xFFFFFF;
  Ctr16Dump("CCCCCC continuation frame for epoch 0x%x \n", epoch);
  if (epoch != board->fCurrentTraceEvent->GetEpoch())
  {
    Ctr16Warn("!!!! Continuation frame with epoch mismatch:  previous 0x%x current 0x%x ! Skip message %ld\n", epoch,
        board->fCurrentTraceEvent->GetEpoch(), skipped_frames++);
    return 1;    // do not skip complete event, but just the current message:
  }
  Ctr16_NEXT_DATAWORD_RETURN;
  Ctr16_CALL_UNPACKER_RETURN(ExtractTrace(board, disp));
  // here try regular evaluation of addtional traces:
  while (fPdata - fPdatastartMsg < fMsize)
  {
    Ctr16_CALL_UNPACKER_RETURN(UnpackTrace(board, disp, epoch));
  }
  return status;
}

Int_t TCtr16RawProc::HandleErrorFrame(TCtr16Board*, TCtr16BoardDisplay *disp)
{
  Int_t status = 0;
  Int_t header = fWorkData;
  UInt_t epoch = header & 0xFFFFFF;
  //Ctr16Dump("HandleErrorFrame starts: fWorkData:0x%x fWorkShift:%d *pdata:0x%x\n", fWorkData, fWorkShift, *fPdata);
#ifdef Ctr16_USE_VULOM
  Int_t endpoint=fMsize; // no crc trailer here
#else
  Int_t endpoint=fMsize+1;
#endif
  while (fPdata - fPdatastartMsg < endpoint)
  {
    //Ctr16Dump("HandleErrorFrame before Ctr16_NEXT_DATAWORD_RETURN: fWorkData:0x%x fWorkShift:%d *pdata:0x%x endpoint:%d\n", fWorkData, fWorkShift, *fPdata, endpoint);
    Ctr16_NEXT_DATAWORD_RETURN
    ;
    UChar_t code = (fWorkData >> 24) & 0xFF;
    UShort_t ts = (fWorkData >> 8) & 0xFFF;
    Ctr16Dump("ERROR FRAME: code:0x%x epoch 0x%x timestamp:0x%x fWorkData:0x%x fWorkShift:%d *pdata:0x%x\n", code,
        epoch, ts, fWorkData, fWorkShift, *fPdata);
    // TODO: create message for output event?
    disp->hErrorcodes->Fill(code);
    disp->hErrorTimestamp->Fill(ts);
    fWorkShift += 8;
    if (fWorkShift == 32)
    {
      fWorkShift = 0;
      fPdata -= 1;    // rewind one word instead of shifting beyond word boundaries
    }
#ifndef Ctr16_USE_VULOM
    endpoint = (fWorkShift ? fMsize+1 : fMsize); // JAM 25-05-23: account workshift, otherwise we might skip last message

    if(fWorkShift && (fPdata - fPdatastartMsg >=endpoint))
    {
      fPdata -= 1; // for last iteration might to have rewind JAM 25-05-23
      break;
    }
#endif
  }

  return status;
}

Int_t TCtr16RawProc::HandleWishboneFrame(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  Int_t status = 0;
  Int_t header = fWorkData;
  UChar_t wishhead = (header >> 24) & 0xFF;
  UChar_t ctrlhead = (header >> 16) & 0xFF;
  //printf("MSG_WishboneEvent header=0x%x\n",wishhead);

  Ctr16Dump("WFWF Handle Wishbone Frame sees wishhead:0x%x ctrlhead:0x%x - pdata=0x%x , fWorkData=0x%x ,workshift:%d\n",(int) wishhead, (int) ctrlhead,
         *fPdata, fWorkData, fWorkShift);
  TCtr16MsgWishbone *theMsg = new TCtr16MsgWishbone(wishhead);
  disp->hWishboneAck->Fill(theMsg->GetAckCode());
  // first evaluate what kind of data we have:
  Bool_t isThresholdMessage = kFALSE;
  Bool_t isSlowControlType = kFALSE;
  TCtr16MsgWishbone::ControlType ctype = TCtr16MsgWishbone::Ctrl_None;




  if (theMsg->GetAckCode() == TCtr16MsgWishbone::ACK_SlowControl)
  {
    ctype = (TCtr16MsgWishbone::ControlType) ctrlhead;
    if ((ctype & TCtr16MsgWishbone::Ctrl_Threshold)==TCtr16MsgWishbone::Ctrl_Threshold) // low 2 bit contain block number, just test for mask
        {
          isThresholdMessage = kTRUE;
          disp->hMsgTypes->Fill(TCtr16Msg::Message_Threshold);
        }
    else if ((ctype & TCtr16MsgWishbone::Ctrl_Start)==TCtr16MsgWishbone::Ctrl_Start)
    {
      disp->hMsgTypes->Fill(TCtr16Msg::Message_Start);
    }
      else if ((ctype & TCtr16MsgWishbone::Ctrl_Init)==TCtr16MsgWishbone::Ctrl_Init)
    {
      disp->hMsgTypes->Fill(TCtr16Msg::Message_Init);
    }
    else
    {
      Ctr16Warn("WWW - found unknown wishbone slow control type 0x%x - skip message! \n", ctype);
      //printf("WWW - found unknown wishbone slow control type 0x%x at event %d - skip message! \n", ctype,fMbsEvt->GetCount());
      return 1;    // do not skip complete event, but just the current message
    }
    isSlowControlType = kTRUE;
  }
  else
  {
    disp->hMsgTypes->Fill(TCtr16Msg::Message_Wishbone);

  }
  // end data type evaluation

  if (isThresholdMessage)
  {
    delete theMsg;    // do not use wishbone message, but dedicated threshold messages in unpacker
    while (fPdata - fPdatastartMsg < fMsize)
    {
      Ctr16_CALL_UNPACKER_RETURN(UnpackThresholdMessage(board, disp));
      // pretend that we could have another message in this frame:
      Ctr16_NEXT_DATAWORD_RETURN
      ;
    }
  }
  else
  {
    // treat data as wishbone message:
    theMsg->SetControlMessageType(ctype);
    if (!isSlowControlType)
    {

      Ctr16_CALL_UNPACKER_RETURN(UnpackWishboneMessage(board, disp, theMsg));
    }
    else
    {

      Ctr16_CALL_UNPACKER_RETURN(UnpackSlowControlMessage(board, disp, theMsg));
    }    // whishbone response or slow control
    board->AddMessage(theMsg, 0);    // wishbone messages accounted for channel 0
  }    // threshold or other wishbone

  if (fPdata - fPdatastartMsg >= fMsize)
     {
       fPdata = fPdatastartMsg + fMsize;
       // align to header of next frame, since after last next dataworde we might be already further
     }

  return status;
}

Int_t TCtr16RawProc::UnpackTrace(TCtr16Board *board, TCtr16BoardDisplay *disp, UInt_t epoch)
{
  Int_t status = 0;
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
  UShort_t ts = (fWorkData >> 4) & 0xFFF;
  UChar_t csa= fWorkData & 0x7;
  tmsg->SetTimeStamp(ts);
  tmsg->SetCsa(csa);
  board->fTracesize12bit = ((fWorkData >> 16) & 0xFF);
  // here check if we have a valid tracelenght:
  if ((board->fTracesize12bit != 16) && (board->fTracesize12bit != 32) && (board->fTracesize12bit != 64))
  {
    Ctr16Warn(
        "Data_Transient with illegal 12bit trace size: %d,  pdata=0x%x , fWorkData=0x%x -  skip rest of message\n",
        board->fTracesize12bit, *fPdata, fWorkData);
    return 1;
  }

  board->fTracesize32bit = board->fTracesize12bit * 3 / 8;
  // account partially filled last data word here:
  if ((Float_t) (board->fTracesize12bit) * 3.0 / 8.0 > (Float_t) board->fTracesize32bit)
    board->fTracesize32bit++;

  Ctr16Debug("DDDD Data_Transient with 12bit trace size: %d, expect %d data words. pdata=0x%x , fWorkData=0x%x\n",
      board->fTracesize12bit, board->fTracesize32bit, *fPdata, fWorkData);

  Ctr16Dump("Data_Transient - channel:%d size12:%d size32:%d\n", fullchannel, board->fTracesize12bit,
      board->fTracesize32bit);
  if ((fPdata - fPdatastart - 3) + board->fTracesize32bit > fLwords)
  {
    Ctr16Warn(
        "Transient Event header error: 12 bit size %d does not fit into mbs subevent buffer of restlen %ld words \n",
        board->fTracesize32bit, fLwords - (fPdata - fPdatastart - 3));
    GO4_SKIP_EVENT
  }
  disp->hDatawords->Fill(board->fTracesize12bit);
  //disp->hChannels->Fill(fullchannel);
  Ctr16_NEXT_DATAWORD_RETURN
  ;
  status = ExtractTrace(board, disp);
  return status;

}

Int_t TCtr16RawProc::ExtractTrace(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  Int_t status = 0;
  Int_t payloadwords = board->fTracesize32bit - board->fTracedataIndex;
  Int_t restlen = fMsize - (fPdata - fPdatastartMsg)  +1;
  if(fWorkShift) restlen++;;// JAM 01-06-23: account workshift, otherwise we might expect continuation falsly
  if (restlen <payloadwords)
  {
    Ctr16Debug("ExtractTrace sees rest of buffer %d too short for desired payload %d, expect continuation! \n",
        restlen , payloadwords);
    payloadwords=restlen;
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
    board->fTracedata[board->fTracedataIndex] = fWorkData;
    Ctr16_NEXT_DATAWORD_RETURN
    ;
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
  Ctr16Debug(
      "ExtractTrace has copied %d payloadwords, fTracedataIndex=%d, pdata=%p, *pData=0x%x, fWorkData=0x%x , fWorkShift=%d, fToBeContinued=%d\n",
      payloadwords, board->fTracedataIndex, fPdata, *fPdata, fWorkData, fWorkShift, board->fToBeContinued);

/// JAM 01-jun-2023: this shift alignment will induce errors for next trace if several are within one frame!
  // here align again for next trace:
//  Int_t usedwords = board->fTracesize32bit - 1;
//  if (usedwords == 0)
//    usedwords = 1;
//  fWorkShift = (32 - (board->fTracesize12bit * 12) % (usedwords * 32));    // JAM 24-03-23: quite redundant, since or 12bit traces always fit into 32 bit words
  /////////////////////////////////////////////////////////////////////////
  Ctr16Debug("ExtractTrace after NO shift alignment, pdata=%p, *pData=0x%x, fWorkData=0x%x , fWorkShift=%d\n", fPdata,
      *fPdata, fWorkData, fWorkShift);
  if (board->fToBeContinued)
  {
    // we expect further data in the continuation frame...
    Ctr16Dump("Transient_Event expects continuation frame after %d words... \n", board->fTracedataIndex);    //Ctr1Dump
  }
  else
  {
    // we are ready inside the first data frame.
    FinalizeTrace(board, disp);
  }
  if (fPdata - fPdatastartMsg >= fMsize)
   {
     fPdata = fPdatastartMsg + fMsize;
     // align to header of next frame, since we might be already further
   }
  return 0;
}

void TCtr16RawProc::FinalizeTrace(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  // this is called when all transient data is received from data and continuation frames
  // we scan through the buffer and fill the transient message
  // then add it to output event
  Int_t tracelength = fPar->fTraceLength;    // for display only
  Int_t binlen = board->fTracesize12bit;    // number of sample bins (should be 16, 32, or 64)
  if (binlen > 64)
    GO4_SKIP_EVENT_MESSAGE("Transient_Event header error: bin length %d exceeds maximum 64", binlen);
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
    disp->hTrace[chan]->Reset("");    //avoid mixing of traces in same mbs event, only keep latest

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

    // JAM 04-07-2023: ADC values per channel
    disp->hADCPerChan[chan]->Fill(val);

    // JAM 01-03-2023: ADC values in memory cell partitions:

    Char_t block=board->fCurrentTraceEvent->GetBlock();
    Char_t channelrow= (board->fCurrentTraceEvent->GetBlockChannel() << 2) | (board->fCurrentTraceEvent->GetRow() & 0x3);
    Char_t cell=((board->fCurrentTraceEvent->GetTimeStamp() & 0xF) + (bin & 0xF)) % 16;
    Short_t fullcell= ((block & 0x3) << 8) |((channelrow & 0xF) << 4) | (cell & 0xF);
    disp->hADCValuesPerCell[block & 0x3][channelrow & 0xF][cell & 0xF]->Fill(val);
    disp->hMemoryCell->Fill(fullcell);
    disp->hMemoryBlockRowMap->Fill(block, channelrow,1);
    disp->hMemoryRowCellMap[(Int_t) block]->Fill(channelrow,cell,1);


  }    // for bin
  disp->fSnapshotcount[chan]++;
  UpdateDeltaTimes(board, disp, board->fCurrentTraceEvent, chan);    // delta t histograms are common with feature events

  disp->hChannels->Fill(chan); // 22-06-23 account channel no sooner than trace is complete
  disp->hCsaAmp[chan]->Fill(board->fCurrentTraceEvent->GetCsa());


  // reset aux data members:
  board->fCurrentTraceEvent = 0;    // do not delete, message is kept in vector!
  board->fTracedataIndex = 0;
  board->fTracesize12bit = 0;
  board->fTracesize32bit = 0;
  board->fToBeContinued = kFALSE;
}

void TCtr16RawProc::UpdateDeltaTimes(TCtr16Board *board, TCtr16BoardDisplay *disp, TCtr16MsgEvent *ev, UChar_t chan)
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

  // also do statistics of current timestamps here:
  disp->hEpochs[chan]->Fill(ev->GetEpoch());
  disp->hTimestamps[chan]->Fill(ev->GetTimeStamp());

}

Int_t TCtr16RawProc::UnpackThresholdMessage(TCtr16Board *board, TCtr16BoardDisplay *disp)
{
  // use threshold messages instead of wishbone container:
//
//                    Byte Bit 7  6  5  4  3  2  1  0
//                           | 0  1  0  1  1  0 | Blk|    Header mit Block Nummer
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
  Int_t status = 0;
  UChar_t block = (fWorkData >> 16) & 0x3;    // fWorkData is still at begin of message header
  UShort_t mean = (fWorkData >> 4) & 0xFFF;
  UShort_t fwhm = (fWorkData & 0xF) <<8;
  Ctr16_NEXT_DATAWORD_RETURN
  fwhm |= (fWorkData >> 24) & 0xFF;

   UShort_t thres = (fWorkData >> 12) & 0xFFF;
   UShort_t track = (fWorkData) & 0xFFF;
   Ctr16_NEXT_DATAWORD_RETURN
  TCtr16MsgThreshold *msg[4];    // one frame contains info for 4 channels in block
  UShort_t baseline[4];
  baseline[0] = (fWorkData >> 20) & 0xFFF;
  baseline[1] = (fWorkData >> 8) & 0xFFF;
  baseline[2] = (fWorkData & 0xFF);
  Ctr16_NEXT_DATAWORD_RETURN
  ;
  baseline[2] |= (fWorkData >> 28) & 0xF;
  baseline[3] = (fWorkData >> 16) & 0xFFF;




  for (Int_t c = 0; c < 4; ++c)
  {
    msg[c] = new TCtr16MsgThreshold(0);
    msg[c]->SetBlock(block);
    msg[c]->SetBlockChannel(c);
    msg[c]->SetMean(mean);
    msg[c]->SetFWHM(fwhm);
    msg[c]->SetTracking(track);
    msg[c]->SetThreshold(thres);
    msg[c]->SetBaseline(baseline[c]);
    Int_t fullchan = msg[c]->GetChannel();
    board->AddMessage(msg[c], fullchan);
    // histograms of threshold values for each channel
    disp->hThresholdBaseline[fullchan]->Fill(baseline[c]);
  }    // for
  // histograms of threshold values blockwise:
  disp->hThresholdMean[block]->Fill(mean);
  disp->hThresholdTracking[block]->Fill(track);
  disp->hThresholdNoise[block]->Fill(fwhm);
  disp->hThresholdSetting[block]->Fill(thres);

#ifdef   Ctr16_USE_ROLLINGGRAPH
  // JAM 6-07-2023: use trending histograms:
  disp->fTrendThresholdMean[block]->Fill(mean,0,0);
  disp->fTrendThresholdTracking[block]->Fill(track,0,0);
  disp->fTrendThresholdNoise[block]->Fill(fwhm,0,0);
  disp->fTrendThresholdSetting[block]->Fill(thres,0,0);
#endif


  Ctr16Dump("TRTR Threshold message block:%d mean:%d track:%d noise:%d thrset:%d - pdata=0x%x , fWorkData=0x%x\n",block, (int) mean,
      (int) track,(int)fwhm, (int) thres, *fPdata, fWorkData);


  SwitchDataAlignment();    // next message will begin in lower 16 bit part of this data word
  Ctr16_NEXT_DATAWORD_RETURN
  return status;
}

Int_t TCtr16RawProc::UnpackSlowControlMessage(TCtr16Board *board, TCtr16BoardDisplay *disp, TCtr16MsgWishbone *theMsg)
{
  Int_t status = 0;
  // slow control message types do not have address fields.
  // we embed the containing text and dump it for fun;
  UChar_t letter;
  for (Int_t shift = 16; shift >= 0; shift -= 8)
  {
    letter = (fWorkData >> shift) & 0xFF;
    theMsg->AddData(letter);
  }
  Ctr16_NEXT_DATAWORD_RETURN
  ;
  while (fPdata - fPdatastartMsg < fMsize)
  {

    for (Int_t shift = 24; shift >= 0; shift -= 8)
    {
      letter = (fWorkData >> shift) & 0xFF;
      theMsg->AddData(letter);
    }
    Ctr16_NEXT_DATAWORD_RETURN
    ;
  }
  disp->lWishboneText->SetText(0.1, 0.9, theMsg->DumpMsg());
  Ctr16Warn("CCC See Control message of type:%d\n %s", theMsg->GetControlMessageType(), theMsg->DumpMsg().Data());
  return status;
}

Int_t TCtr16RawProc::UnpackWishboneMessage(TCtr16Board *board, TCtr16BoardDisplay *disp, TCtr16MsgWishbone *theMsg)
{
  Int_t status = 0;
  // regular wishbone message:
  Int_t header = fWorkData;
  Short_t address = (header >> 8) & 0xFFFF;
  theMsg->SetAddress(address);
  // here optional rest of data: only if data follows is set!
  if (theMsg->GetAckCode() == TCtr16MsgWishbone::ACK_Data)
  {
    UChar_t val = fWorkData && 0xFF;    // fWorkData is still at begin of message header
    theMsg->AddData(val);
    Ctr16_NEXT_DATAWORD_RETURN
    ;
    while (fPdata - fPdatastartMsg < fMsize)
    {
      for (Int_t shift = 24; shift >= 0; shift -= 8)
      {
        val = (fWorkData >> shift) & 0xFF;
        theMsg->AddData(val);
      }
      Ctr16_NEXT_DATAWORD_RETURN
      ;
    }    // while
  }
  else
  {
    Ctr16_NEXT_DATAWORD_RETURN
    ;
  }
  disp->hWishboneSource->Fill(theMsg->GetSource());
  // TODO: histogram of wishbone addresses and data words?

  return status;
}

Int_t TCtr16RawProc::UnpackFeature(TCtr16Board *board, TCtr16BoardDisplay *disp, UInt_t epoch)
{
  Int_t status = 0;
  UChar_t fullchannel = ((fWorkData >> 26) & 0xF);
  UChar_t row = ((fWorkData >> 24) & 0x3);
  disp->hMemoryRow->Fill(row);
  disp->hChannels->Fill(fullchannel);
  TCtr16MsgFeature *fmsg = new TCtr16MsgFeature(fullchannel);
  fmsg->SetEpoch(epoch);
  fmsg->SetRow(row);
  UChar_t csa = (fWorkData >> 21) & 0x7; // 22-jun-23 JAM
  UShort_t ts = (fWorkData >> 6) & 0xFFF;
  fmsg->SetTimeStamp(ts);
  fmsg->SetCsa(csa);
  UChar_t finetime = (fWorkData & 0x3F);
  fmsg->SetFineTime(finetime);
  Ctr16_NEXT_DATAWORD_RETURN
  ;
  UShort_t ampl = (fWorkData >> 16) & 0xFFFF;
  fmsg->SetAmplitude(ampl);

  Ctr16Warn("DFDFDF Data_Feature channel:%d csa:%d ampl:%d ts:%d ftime:%d - pdata=0x%x , fWorkData=0x%x\n", fullchannel, (int) csa, ampl,
      ts, finetime, *fPdata, fWorkData);

  // feature histograms here:
  disp->hFeatureAmplitude[fullchannel]->Fill(ampl);
  disp->hFeatureAmplitudeScaled[fullchannel]->Fill(ampl* fmsg->GetCsaFactor());
  disp->hFeatureFineTime[fullchannel]->Fill(finetime);
  UpdateDeltaTimes(board, disp, fmsg, fullchannel);    // evaluate delta T between messages also here

  disp->hCsaAmp[fullchannel]->Fill(csa); // monitor csa factor

  board->AddMessage(fmsg, fullchannel);
  SwitchDataAlignment();    // next message will begin in lower 16 bit part of this data word
  Ctr16_NEXT_DATAWORD_RETURN
  ;
  // TODO: check if another feature event would fit into rest of vulom container
  Ctr16Debug("DFDFDF restlen= %ld u32words \n", fMsize - (fPdata - fPdatastartMsg));
  if (fPdata - fPdatastartMsg >= fMsize)
   {
     fPdata = fPdatastartMsg + fMsize;
     // align to header of next frame, since after last next dataworde we might be already further
   }
  return status;
}
