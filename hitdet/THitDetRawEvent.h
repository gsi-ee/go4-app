#ifndef THitDetRAWEVENT_H
#define THitDetRAWEVENT_H

/*
 * Raw event structures for readout of Hit Detection ASIC
 * Jörn Adamczewski-Musch, GSI-CSEE
 *
 * v0.1 10 July 2015
 *
 */

#include "TGo4EventElement.h"
#include "TGo4CompositeEvent.h"

#include <vector>
#include "TString.h"

// maximum number of boards to investigate at once
#define HitDet_MAXBOARDS 10

// number of data channels per hit detection board
#define HitDet_CHANNELS 4

// length of sampled trace per triggered readout message
#define HitDet_TRACEBINS 32

// length of sampled bins per direct ADC message
#define HitDet_DIRECTBINS 8

/**
 * Base class for single "message" inside data stream */
class THitDetMsg
{

public:

  enum MsgType
  {
    MSG_ADC_Direct = 0, MSG_Wishbone = 1, MSG_Unused = 2, MSG_ADC_Event = 3
  };

  THitDetMsg()
  {
    ;
  }
  virtual ~THitDetMsg()
  {
    ;
  }
};

/**
 * Message class for (chip event-) triggered data  */
class THitDetMsgEvent: public THitDetMsg
{

public:
  THitDetMsgEvent(UChar_t ch=0) :
      fChannel(ch), fEpoch(0), fTimeStamp(0)
  {
    for (Int_t j = 0; j < HitDet_TRACEBINS; ++j)
      fTrace[j] = 0;
  }
  virtual ~THitDetMsgEvent()
  {
    ;
  }

  /** channel number (0-3)*/
  void SetChannel(UChar_t ch)
  {
    fChannel = ch;
  }
  UShort_t GetChannel()
  {
    return fChannel;
  }

  /** Epoch counter (24 bit)*/
  void SetEpoch(UInt_t e)
  {
    fEpoch = e;
  }
  UInt_t GetEpoch()
  {
    return fEpoch;
  }

  /** Time Stamp counter (12 bit)*/
  void SetTimeStamp(UShort_t ts)
  {
    fTimeStamp = ts;
  }
  UShort_t GetTimeStamp()
  {
    return fTimeStamp;
  }

  /** sampled signal trace data*/
  void SetTraceData(Int_t ix, UShort_t val)
  {
    if ((ix >= 0) && (ix < HitDet_TRACEBINS))
      fTrace[ix] = val;
  }
  UShort_t GetTraceData(Int_t ix)
  {
    if ((ix >= 0) && (ix < HitDet_TRACEBINS))
      return fTrace[ix];
    return 0;
  }

protected:

  /** channel number (0-3)*/
  UChar_t fChannel;

  /** Epoch counter (24 bit)*/
  UInt_t fEpoch;

  /** Time Stamp counter (12 bit)*/
  UShort_t fTimeStamp;

  /** sampled signal trace data*/
  UShort_t fTrace[HitDet_TRACEBINS];

};

/**
 * Message class for direct ADC readout data  */
class THitDetMsgDirect: public THitDetMsg
{

public:
  THitDetMsgDirect(UShort_t count = 0) :
      fMsgCounter(count)
  {
    for (Int_t j = 0; j < HitDet_DIRECTBINS; ++j)
      fBin[j] = 0;
    ;
  }
  virtual ~THitDetMsgDirect()
  {
    ;
  }

  void SetBinData(Int_t ix, UShort_t val)
  {
    if ((ix >= 0) && (ix < HitDet_DIRECTBINS))
      fBin[ix] = val;
  }
  UShort_t GetBinData(Int_t ix)
  {
    if ((ix >= 0) && (ix < HitDet_DIRECTBINS))
      return fBin[ix];
    return 0;
  }
  UShort_t GetMsgCounter()
  {
    return fMsgCounter;
  }

protected:
  /** message counter value in sequence of direct readout frames (11 bit)*/
  UShort_t fMsgCounter;

  /** sampled data for each ADC bin (12 bit)*/
  UShort_t fBin[HitDet_DIRECTBINS];

};

/**
 * Message class for Wishbone response message  */
class THitDetMsgWishbone: public THitDetMsg
{

public:

  enum AckType
  {
    ACK_Acknowledge = 0, ACK_Unused = 1, ACK_Error = 2, ACK_Data = 3
  };

  THitDetMsgWishbone() :
      fWishboneHeader(0), fRegAddress(0)
  {
    ;
  }
  THitDetMsgWishbone(UChar_t header) :
      THitDetMsg(), fWishboneHeader(header), fRegAddress(0)
  {
  }
  virtual ~THitDetMsgWishbone()
  {
    ;
  }

  UChar_t GetAckCode()
  {
    return ((fWishboneHeader >> 4) & 0x3);
  }

  UChar_t GetSource()
  {
    return (fWishboneHeader & 0xF);
  }

  void SetHeader(UChar_t head)
  {
    fWishboneHeader = head;
  }

  void SetAddress(UChar_t add)
  {
    fRegAddress = add;
  }
  UChar_t GetAddress()
  {
    return fRegAddress;
  }

  void AddData(UChar_t dat)
  {
    fData.push_back(dat);
  }

  UChar_t GetData(Int_t ix)
  {
    return fData[ix];
  }

  size_t GetDataSize()
  {
    return fData.size();
  }

  /* fill string object with properties and contents of data field*/
  TString DumpMsg()
  {
    TString head, add, data;
    head.Form("Wishbone message ack:0x%x src 0x%x\n", GetAckCode(), GetSource());
    if (GetAddress())
      add.Form(" - address: 0x%x\n", GetAddress());
    data.Form(" - data size 0x%x, data:\n", (int) GetDataSize());
    for (size_t i = 0; i < GetDataSize(); ++i)
    {
      char key=GetData(i);
      data.Append((const char*) &key, 1);
    }
    return (head + add + data);
  }

private:

  /** acknowledge type (2 bit)*/
  UChar_t fWishboneHeader;

  /** address of dumped data register (optional)*/
  UChar_t fRegAddress;

  /** data stream of unknown length (optional)*/
  std::vector<UChar_t> fData;

};

/******************************************************************
 *
 * This class represents one HitDetection readout board with one HitDetection asic each
 */

class THitDetBoard: public TGo4EventElement
{
public:
  THitDetBoard();
  /* */
  THitDetBoard(const char* name, UInt_t unid, Short_t index);
  virtual ~THitDetBoard();

  void AddMessage(THitDetMsg* msg, UChar_t channel);

  /* vectors of hit detection messages within the mbs event, sorted for the event readout channels
   * note that direct readout and wishbone messages are put into first channel 0
   * */
  std::vector<THitDetMsg*> fMessages[HitDet_CHANNELS];

  /** Method called by the framework to clear the event element. */
  void Clear(Option_t *t = "");

  /* get unique hardware id of board in set up*/
  UInt_t GetBoardId()
  {
    return fUniqueId;
  }

private:
  /* unique hardware id of the board. THis should be independent of position in readout chain*/
  UInt_t fUniqueId;

ClassDef(THitDetBoard,1)
};

/* ************************************************************************
 *
 * The top event structure
 * with all board components in the readout chain
 *
 * */

class THitDetRawEvent: public TGo4CompositeEvent
{
public:
  THitDetRawEvent();
  THitDetRawEvent(const char* name, Short_t id = 0);
  virtual ~THitDetRawEvent();

  void Clear(Option_t *t = "");

  /* access to board subevent by unique id*/
  THitDetBoard* GetBoard(UInt_t uniqueid);

  /* This array keeps the unique id numbers of configured hit detection boards*/
  static std::vector<UInt_t> fgConfigHitDetBoards;

  /** Event sequence number incremented by MBS Trigger*/
  Int_t fSequenceNumber;

  /** vulom status register*/
  Int_t fVULOMStatus;

  /** number of payload words in vulom buffer (u32)*/
  Int_t fDataCount;

ClassDef(THitDetRawEvent,1)
};
#endif //TEVENT_H
