#ifndef TCtr16RAWEVENT_H
#define TCtr16RAWEVENT_H

/*
 * Raw event structures for readout of CTR16 chip
 * Jorn Adamczewski-Musch (JAM), GSI EEL
 *
 * v0.6 27-03-2023
 *
 */

#include "TGo4EventElement.h"
#include "TGo4CompositeEvent.h"

#include <vector>
#include "TString.h"


// enable this define to use test readout with vulom. If disabled we assume GOSIP readout via kilom
#define Ctr16_USE_VULOM 1

// test if go4 rolling graph have any impact on memory?
#define Ctr16_USE_ROLLINGGRAPH 1

// if this is set, do not store complete events, but evaluate ADC histograms for memory cells into tree JAM 21.09.22
//#define Ctr16_DO_MEMORYSAMPLES 1



// switches between interpretation of ADC values as signed if defined
#define Ctr16_BOTHPOLARITY 1

// maximum number of boards to investigate at once
#define Ctr16_MAXBOARDS 4


// / maximum number of sfp readout chains used
#define Ctr16_MAXSFP 4

//maximum number of boards at each chanin
#define Ctr16_MAXDEVS 5


// number of data channels per hit detection board
#define Ctr16_CHANNELS 16

// maximum length of sampled trace per triggered readout message
#define Ctr16_TRACEBINS 64

// maximum required longword buffer for trace data
#define Ctr16_TRACEWORDS Ctr16_TRACEBINS * 3 / 8

// maximum number of bytes in a data frame
#define Ctr16_MAXPAYLOAD 64

// range of feature extracted amplitude values
#define Ctr16_FEATURE_AMPRAMGE 0xFFFF

// range of feature extracted fine time values
#define Ctr16_FEATURE_FINETIMERANGE 0x3F


// range of threshold setup values
#define Ctr16_THRESHOLD_RANGE 0xFFF


// range of memory blocks
#define Ctr16_BLOCKS 4

// range of rows+channels per block
#define Ctr16_CHANNELROWS 16

// range of sub memorycells
#define Ctr16_MEMORYCELLS 16


//  000               X   0.350 pF     1
//  001            X  X   1.655 pF     4.7
//  010         X  X  X   4.246 pF    12
//  011      X  X  X  X  14.611 pF    42
//  100   X  X  X  X  X  55.071 pF   157
// csa coefficients for amplification:
#define Ctr16_CSAFACTORS { 1.0, 4.7, 12.0, 42.0, 157, -1, -1, 115.0}
// last value for fix simul setup with 40.46 pF JAM

/** define range of CSA amplification */
#define Ctr16_CSAMAXFACTOR 160





/**
 * Base class for single "data message" inside data stream */
class TCtr16Msg
{

public:

  enum MessageType
    {
      Message_Init = 0, Message_Start=1, Message_Threshold = 2, Message_Wishbone = 3
    };


  enum FrameType
  {
    Frame_Continuation = 0, Frame_Wishbone = 1, Frame_Error = 2, Frame_Data = 3
  };

  enum DataType
  {
    Data_None =0, Data_Unused=1, Data_Transient = 3, Data_Feature = 2
  };


  TCtr16Msg()
  {
    ;
  }
  virtual ~TCtr16Msg()
  {
    ;
  }
};

/**
 * Message class for all data containers with channel  */
class TCtr16MsgData: public TCtr16Msg
{

public:
  TCtr16MsgData(UChar_t ch=0) :
       fChannel(ch)
   {

   }

  /** full  channel number (0-16)*/
   void SetChannel(UChar_t ch)
   {
     fChannel = ch;
   }
   UChar_t GetChannel()
   {
     return fChannel;
   }

   void SetBlock(UChar_t b)
   {
     fChannel &= ~(0x3<<2);
     fChannel |= ((b & 0x3) << 2);
   }

   UChar_t GetBlock()
   {
     return (fChannel >> 2) & 0x3;
   }

   void SetBlockChannel(UChar_t b)
     {
       fChannel &= ~(0x3);
       fChannel |= (b & 0x3);
     }


   UChar_t GetBlockChannel()
      {
        return (fChannel) & 0x3;
      }



protected:

  /** full channel number (0-16).
   * This is composed out of block number and channel number in block*/
    UChar_t fChannel;



};

/** Message class for control threshold message data.
 * Generated after completion of automatic threshold scan  */



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


class TCtr16MsgThreshold: public TCtr16MsgData
{

  public:

  TCtr16MsgThreshold(UChar_t ch=0):  TCtr16MsgData(ch),fMean(0), fNoiseWidth(0),fThreshold(0), fTracking(0),fBaseline(0)
   {
     ;
   }
   virtual ~TCtr16MsgThreshold()
   {
     ;
   }
   void SetMean (UShort_t b)
     {
       fMean = (b & 0xFFF);
     }


   void SetBaseline (UShort_t b)
   {
     fBaseline = (b & 0xFFF);
   }

   void SetFWHM (UShort_t n)
     {
       fNoiseWidth = (n & 0xFFF);
     }

   void SetThreshold (UShort_t t)
   {
         fThreshold = (t & 0xFFF);
   }

   void SetTracking (UShort_t t)
   {
         fThreshold = (t & 0xFFF);
   }

   UShort_t GetMean()
    {
      return fMean;
    }

   UShort_t GetBaseLine()
   {
     return fBaseline;
   }

   UShort_t GetFWHM()
      {
        return fNoiseWidth;
      }

   UShort_t GetThreshold()
   {
     return fThreshold;
   }

   UShort_t GetTracking()
     {
       return fTracking;
     }

  protected:



  /** mean baseline value determined by threshold scan*/
  UShort_t fMean;

  /** FWHM of noise width*/
   UShort_t fNoiseWidth;

   /** DAC Threshold finally set*/
   UShort_t fThreshold;

   /** DAC Threshold Tracking value*/
   UShort_t fTracking;

   /** baseline set by threshold scan*/
    UShort_t fBaseline;





};





/**
 * Message class for (chip event-) triggered data  */
class TCtr16MsgEvent: public TCtr16MsgData
{

public:



  /** csa coefficients for amplification */
  static Float_t fgCSAFactor[8];



  TCtr16MsgEvent(UChar_t ch=0) :
    TCtr16MsgData(ch), fEpoch(0), fTimeStamp(0), fRow(0),fCsa(0)
  {
  }
  virtual ~TCtr16MsgEvent()
  {
    ;
  }



  /** Epoch counter (24 bit)*/
  void SetEpoch(UInt_t e)
  {
    fEpoch = (e & 0xFFFFFF);
  }
  UInt_t GetEpoch()
  {
    return fEpoch;
  }

  /** Time Stamp counter (12 bit)*/
  void SetTimeStamp(UShort_t ts)
  {
    fTimeStamp = (ts & 0xFFF);
  }
  UShort_t GetTimeStamp()
  {
    return fTimeStamp;
  }

  /** analoge memory row number (2 bits) */
  void SetRow(UChar_t row)
    {
      fRow = (row & 0x3);
    }
    UChar_t GetRow()
    {
      return fRow;
    }

    void SetCsa(UChar_t csa)
      {
        fCsa = (csa & 0x3);
      }
    UChar_t GetCsa()
      {
        return fCsa;
      }

    Float_t GetCsaFactor()
    {
      Float_t val=TCtr16MsgEvent::fgCSAFactor[fCsa];
      return val;
    }


protected:


  /** Epoch counter (24 bit)*/
  UInt_t fEpoch;

  /** Time Stamp counter (12 bit)*/
  UShort_t fTimeStamp;

  /** Memory row index of data*/
  UChar_t fRow;

  /** CSA feedback configuration value (amplification)*/
  UChar_t fCsa;

};






/**
 * Message class for transient (sampled trace) data
 *  */
class TCtr16MsgTransient: public TCtr16MsgEvent
{
public:

  TCtr16MsgTransient(UChar_t ch=0) :
      TCtr16MsgEvent(ch)
    {
          fTrace.clear();
    }
    virtual ~TCtr16MsgTransient()
    {
      ;
    }


    /** sampled signal trace data */

    void AddTraceData(UShort_t val)
    {
      if(TraceLength() <= Ctr16_TRACEBINS)
        fTrace.push_back(val);
    }

     void SetTraceData(Int_t ix, UShort_t val)
     {
       if ((ix >= 0) && (ix < Ctr16_TRACEBINS))
         fTrace[ix] = val;
     }
     UShort_t GetTraceData(Int_t ix)
     {
       if ((ix >= 0) && (ix < Ctr16_TRACEBINS))
         return fTrace[ix];
       return 0;
     }

     size_t TraceLength()
     {
       return fTrace.size();
     }

protected:

  /** sampled signal trace data*/
     std::vector<UShort_t> fTrace;
};

/**
 * Message class for feature extracted data
 * */
class TCtr16MsgFeature: public TCtr16MsgEvent
{
public:

  TCtr16MsgFeature(UChar_t ch=0) :
      TCtr16MsgEvent(ch),fFineTime(0),fAmplitude(0)
    {

    }
  virtual ~TCtr16MsgFeature()
    {
      ;
    }

  /** Fine Time  (6 bit)*/
   void SetFineTime(UChar_t ft)
   {
     fFineTime = (ft & 0x3F);
   }
   UChar_t GetFineTime()
   {
     return fFineTime;
   }

   void SetAmplitude(UShort_t a)
    {
      fAmplitude = a;
    }
    UShort_t GetAmplitude()
    {
      return fAmplitude;
    }


protected:

  /** finetime (6 bit)*/
  UChar_t fFineTime;

  /** pulse amplitude */
  UShort_t fAmplitude;

    };





/**
 * Message class for Wishbone response message  */
class TCtr16MsgWishbone: public TCtr16Msg
{

public:

  enum AckType
  {
    ACK_Acknowledge = 0, ACK_SlowControl = 1, ACK_Error = 2, ACK_Data = 3
  };

  enum ControlType
   {
    Ctrl_None = 0x49, Ctrl_Init = 0x50, Ctrl_Start = 0x54, Ctrl_Threshold = 0x58
     // JAM23: note that Threshold control messages are kept in a separate class independent of wishbone message
     // 6-july-2023: Ctrl_Threshold id is changed
   };



  TCtr16MsgWishbone() :
      fWishboneHeader(0), fRegAddress(0),fControlType(Ctrl_None)
  {
    ;
  }
  TCtr16MsgWishbone(UChar_t header) :
      TCtr16Msg(), fWishboneHeader(header), fRegAddress(0), fControlType(Ctrl_None)
  {
  }
  virtual ~TCtr16MsgWishbone()
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

  ControlType GetControlMessageType()
  {
    return fControlType;
  }
  void SetControlMessageType(ControlType t)
   {
    fControlType=t;
   }




  /* fill string object with properties and contents of data field*/
  TString DumpMsg()
  {
    TString head, add, data;
    head.Form("WW Wishbone message ack:0x%x src 0x%x\n", GetAckCode(), GetSource());
    if (GetAddress())
     add.Form("WW - address: 0x%x\n", GetAddress());
   data.Form("WW - data size 0x%x, data:\nWW -", (int) GetDataSize());
    // TODO: evaluate here different control messages and put their payload right?

    for (size_t i = 0; i < GetDataSize(); ++i)
    {
      char key=GetData(i);
      data.Append(key, 1);
    }
    data.Append('\n',1);
    return (head + add + data);
  }

private:

  /** acknowledge type (2 bit)*/
  UChar_t fWishboneHeader;

  /** address of dumped data register (optional)*/
  UChar_t fRegAddress;

  /** remember type of embedded control message*/
  ControlType fControlType;

  /** data stream of unknown length (optional)*/
  std::vector<UChar_t> fData;

};

/******************************************************************
 *
 * This class represents one Ctr16ection readout board with one Ctr16ection asic each
 */

class TCtr16Board: public TGo4EventElement
{
public:
  TCtr16Board();
  /* */
  TCtr16Board(const char* name, UInt_t unid, Short_t index);
  virtual ~TCtr16Board();

  void AddMessage(TCtr16Msg* msg, UChar_t channel);

  /* vectors of hit detection messages within the mbs event, sorted for the event readout channels
   * note that wishbone messages are put into first channel 0
   * */

#ifdef Ctr16_DO_MEMORYSAMPLES
  std::vector<TCtr16Msg*> fMessages[Ctr16_CHANNELS];//!for memory sample mode, do not store this into tree
#else
  std::vector<TCtr16Msg*> fMessages[Ctr16_CHANNELS];
#endif






  /** Method called by the framework to clear the event element. */
  void Clear(Option_t *t = "");

  /* get unique hardware id of board in set up*/
  UInt_t GetBoardId()
  {
    return fUniqueId;
  }


  /** JAM23- below some not persistent helpers to cope with continuation frame data: */

  /* most recent trace data container*/
  TCtr16MsgTransient* fCurrentTraceEvent; //!

  /** remember most recent message for delta T evaluation*/
  TCtr16MsgEvent fLastMessages[Ctr16_CHANNELS]; //!



  /* buffer for trace event data of most recent frames
   * set to maximum expected words*/
  Int_t fTracedata[Ctr16_TRACEWORDS]; //!

  /** Working index inside tracedata field */
  Int_t fTracedataIndex; //!

  /** Size of next trace bins*/
  UChar_t fTracesize12bit; // !

  /** size of trace word buffer*/
  UChar_t fTracesize32bit; //!

  /** true if trace data needs continuation frame to be completed*/
  Bool_t fToBeContinued; //!



private:
  /* unique hardware id of the board. THis should be independent of position in readout chain*/
  UInt_t fUniqueId;

ClassDef(TCtr16Board,1)
};

/* ************************************************************************
 *
 * The top event structure
 * with all board components in the readout chain
 *
 * */

class TCtr16RawEvent: public TGo4CompositeEvent
{
public:
  TCtr16RawEvent();
  TCtr16RawEvent(const char* name, Short_t id = 0);
  virtual ~TCtr16RawEvent();

  void Clear(Option_t *t = "");

  /* access to board subevent by unique id*/
  TCtr16Board* GetBoard(UInt_t uniqueid);

  /* This array keeps the unique id numbers of configured hit detection boards*/
  static std::vector<UInt_t> fgConfigCtr16Boards;

  /** Event sequence number incremented by MBS Trigger*/
  Int_t fSequenceNumber;

  /** vulom status register*/
  Int_t fVULOMStatus;

  /** number of payload words in vulom buffer (u32)*/
  Int_t fDataCount;


#ifdef Ctr16_DO_MEMORYSAMPLES

  /** Full name of input file*/
  TString fLmdFileName;

  /* year number from filename */
  Char_t fYear;

  /* month  number from filename */
  Char_t fMonth;

  /* day  number from filename */
  Char_t fDay;

  /* 24h clock time  from filename */
  Short_t fTime;

  /** Clock freququency from filenam*/
  Float_t fClockFreq;

  /** 3 digit hex number info in filename */
  Short_t fInfonumber;

  /**Mean value of ADC histogram for block,row, cell indices*/
  Double_t fADCMean[Ctr16_BLOCKS][Ctr16_CHANNELROWS][Ctr16_MEMORYCELLS];

  /**StdDev value of ADC histogram for block,row, cell indices*/
  Double_t fADCSigma[Ctr16_BLOCKS][Ctr16_CHANNELROWS][Ctr16_MEMORYCELLS];

  /**Number of entries in ADC histogram for block,row, cell indices*/
   Double_t fADCEntries[Ctr16_BLOCKS][Ctr16_CHANNELROWS][Ctr16_MEMORYCELLS];


#endif



ClassDef(TCtr16RawEvent,1)
};
#endif //TEVENT_H
