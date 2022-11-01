#ifndef TGet4ppRAWEVENT_H
#define TGet4ppRAWEVENT_H

/*
 * Raw event structures for readout of Get4++
 * Joern Adamczewski-Musch, GSI-CSEE
 *
 * v0.1 14 Apr 2020
 *
 */

#include "TGo4EventElement.h"
#include "TGo4CompositeEvent.h"


#include <vector>
#include "TString.h"


// test if we can fix wrong coarscounter with this?
#define Get4pp_REPAIRCOARSETIME 1



// if this is set, do not store complete events, but evaluate channel fine times for tree JAM 21.09.22
#define Get4pp_DOFINETIMSAMPLES 1


// maximum number of boards to investigate at once
#define Get4pp_MAXBOARDS 10

// number of data channels per hit detection board
#define Get4pp_CHANNELS 4

// epoch counter size (24 bit)
#define Get4pp_EPOCHRANGE 0xFFFFFF

// course counter size (12 bit)
#define Get4pp_COARSERANGE 0xFFF

// fine counter size (7 bit)
#define Get4pp_FINERANGE 0x7F

// error message size (7 bit)
#define Get4pp_ERRORRANGE 0x7F

// DLL phase range size (4 bit)
#define Get4pp_DLLRANGE 0xF

// Time in seconds of one coarse counter bin
#define Get4pp_COARSETIMEUNIT 2.0e-9

/**
 * Base class for single "message" inside data stream */
class TGet4ppMsg
{

public:

  typedef enum MsgType
  {
    MSG_TDC_Data = 0, MSG_Wishbone = 1, MSG_Unused_1 = 2, MSG_Unused_2 = 3
  } Get4MessageType_t;

  TGet4ppMsg()
  {
    ;
  }
  virtual ~TGet4ppMsg()
  {
    ;
  }
};

/**
 * Message base class for (chip event-) triggered data  */
class TGet4ppMsgEvent: public TGet4ppMsg
{

public:
  TGet4ppMsgEvent(UChar_t ch=0) : TGet4ppMsg(),
      fChannel(ch), fLeadingEdge(kFALSE), fEpoch(0)
  {

  }
  virtual ~TGet4ppMsgEvent()
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

  void SetLeadingEdge(Bool_t on)
  {
    fLeadingEdge = on;
  }
  Bool_t IsLeadingEdge()
  {
    return fLeadingEdge;
  }


  /** Epoch counter (24 bit)*/
  void SetEpoch(UInt_t e)
  {
    fEpoch = (e & Get4pp_EPOCHRANGE);
  }
  UInt_t GetEpoch()
  {
    return fEpoch;
  }






protected:

  /** channel number (0-3)*/
  UChar_t fChannel;

  /** true if edge is rising*/
  Bool_t fLeadingEdge;

  /** Epoch counter (24 bit)*/
  UInt_t fEpoch;




};

/**
 * Message class for TDC data  */
class TGet4ppMsgTDCEvent: public TGet4ppMsgEvent
{

public:
  TGet4ppMsgTDCEvent(UChar_t ch=0) : TGet4ppMsgEvent(ch),
  	  fCoarseTime(0), fFineTime(0)
  {

  }
  virtual ~TGet4ppMsgTDCEvent()
  {
    ;
  }


  /** Coarse Time Stamp counter (12 bit)*/
    void SetCoarseTime(UShort_t ts)
    {
      fCoarseTime = (ts & Get4pp_COARSERANGE);
    }
    UShort_t GetCoarseTime()
    {
      return fCoarseTime;
    }

    /** Fine Time Stamp counter (7 bit)*/
    void SetFineTime(UChar_t ts)
    {
    	fFineTime = (ts & Get4pp_FINERANGE);
    }
    UChar_t GetFineTime()
    {
    	return fFineTime;
    }

    /** evaluate full timestamp in fine time granularity counts units*/
    Double_t GetFullTime()
    {
      Double_t correctedcoarse=fCoarseTime;
#ifdef      Get4pp_REPAIRCOARSETIME
      if(((fFineTime >> 6) & 0x1) == 0x1) correctedcoarse--;
#endif
      Double_t ts =((Double_t) fEpoch * (Double_t) (Get4pp_COARSERANGE +1) * (Double_t)(Get4pp_FINERANGE+1) + (Double_t) correctedcoarse * (Double_t)(Get4pp_FINERANGE+1) + (Double_t) fFineTime);
    	return ts;
    }


    /** evaluate full timestamp in seconds unit*/
    Double_t GetTimeInSeconds()
    {
    	//Double_t ts =( fEpoch * (Double_t)(Get4pp_COARSERANGE +1) + fCoarseTime + fFineTime/ (Double_t) (Get4pp_FINERANGE+1) )* Get4pp_COARSETIMEUNIT;
    	Double_t ts = GetFullTime() * Get4pp_COARSETIMEUNIT / Double_t (Get4pp_FINERANGE+1);
    	return ts;
    }


  /** Coarse Time Stamp counter (12 bit)*/
  UShort_t fCoarseTime;

  /** Fine Time Stamp counter (7 bit)*/
   UChar_t fFineTime;
};



/**
 * Message class for Error event data  */
class TGet4ppMsgErrorEvent: public TGet4ppMsgEvent
{

public:

	typedef enum ErrorType {

		ERR_Init_Readout = 0x0,
		ERR_Sync = 0x1,
		ERR_Epochcounter_Sync = 0x2,
		ERR_Epoch = 0x3,
		ERR_FIFO_Write = 0x4,
		ERR_Lost_Event = 0x5,
		ERR_Channel_State = 0x6,
		ERR_Tokenring_State = 0x7,
		ERR_Token = 0x8,
		ERR_Readout = 0x9,
		ERR_SPI = 0xA,
		ERR_DLL_Lock = 0xB,
		ERR_DLL_Reset = 0xC,
		ERR_Unknown = 0x7F
	} Get4ErrorType_t;

  TGet4ppMsgErrorEvent(UChar_t ch=0) : TGet4ppMsgEvent(ch),
		  fDLLPhase(0), fErrorCode(ERR_Unknown)
  {

  }
  virtual ~TGet4ppMsgErrorEvent()
  {
    ;
  }

  /** Error Code  (7 bit)*/
   void SetErrorCode(Get4ErrorType_t ec)
   {
   	//fErrorCode = (ec & Get4pp_ERRORRANGE); //compiler does not like type mixing
	 fErrorCode = ec;
   }

   Get4ErrorType_t GetErrorCode()
   {
   	return fErrorCode;
   }

	TString FormErrorMessage(Get4ErrorType_t ec) {
		TString mess;
		switch (ec) {
		case ERR_Init_Readout:
			mess = "Initialisation of readout";
			break;

		case ERR_Sync:
			mess = "Sync error";
			break;

		case ERR_Epochcounter_Sync:
			mess = "Epoch counter sync error";
			break;

		case ERR_Epoch:
			mess = "Epoch error";
			break;

		case ERR_FIFO_Write:
			mess = "FIFO write error";
			break;

		case ERR_Lost_Event:
			mess = "Lost event errort";
			break;

		case ERR_Channel_State:
			mess = "Channel state error";
			break;

		case ERR_Tokenring_State:
			mess = " Token ring state error";
			break;

		case ERR_Token:
			mess = " Token error";
			break;

		case ERR_Readout:
			mess = "Readout error";
			break;

		case ERR_SPI:
			mess = " SPI error";
			break;

		case ERR_DLL_Lock:
			mess = " DLL Lock Error";
			break;

		case ERR_DLL_Reset:
			mess = "DLL Reset invoked";
			break;

		case ERR_Unknown:
		default:
			mess = "Unknown error";
			break;
		}

		return mess;
	}

   TString GetErrorMessage()
   {
	   return FormErrorMessage(fErrorCode);
   }


   /** DLL phase  (4 bit)*/
   void SetDLLPhase(UChar_t dp)
   {
	   fDLLPhase = (dp & Get4pp_DLLRANGE);
   }
   UChar_t GetDLLPhase()
     {
     	return fDLLPhase;
     }



  /**   current phase of the DLL reset generator.(4 bit)*/
  UShort_t fDLLPhase;

  /** Error code (7 bit)
  Error code   Channel       Error type
  -------------------------------------
  0000000  		-			Initialisation of readout
  0000001		-			Sync error
  0000010  	    - 	  	    Epoch counter sync error
  0000011		-		    Epoch error
  0000100		yes			FIFO write error
  0000101		yes			Lost event error
  0000110		yes			Channel state error
  0000111		-			Token ring state error
  0001000		-			Token error
  0001001		-			Error readout error
  0001010		-			SPI error
  0001011		-			DLL Lock Error
  0001100		-			DLL Reset invoked
  1111111		-			Unknown error
*/


  Get4ErrorType_t fErrorCode;
};



/**
 * Message class for Wishbone response message  */
class TGet4ppMsgWishbone: public TGet4ppMsg
{

public:

  enum AckType
  {
    ACK_Acknowledge = 0, ACK_Unused = 1, ACK_Error = 2, ACK_Data = 3
  };

  TGet4ppMsgWishbone() :
      fWishboneHeader(0), fRegAddress(0)
  {
    ;
  }
  TGet4ppMsgWishbone(UChar_t header) :
      TGet4ppMsg(), fWishboneHeader(header), fRegAddress(0)
  {
  }
  virtual ~TGet4ppMsgWishbone()
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
 * This class represents one Get4ppection readout board with one Get4ppection asic each
 */

class TGet4ppBoard: public TGo4EventElement
{
public:
  TGet4ppBoard();
  /* */
  TGet4ppBoard(const char* name, UInt_t unid, Short_t index);
  virtual ~TGet4ppBoard();

  void AddMessage(TGet4ppMsg* msg, UChar_t channel);

  UInt_t NumMessages(UChar_t channel);

  TGet4ppMsg* GetMessage(UChar_t channel, UInt_t i);



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

  /* vectors of TDC messages within the mbs event, sorted for the event readout channels
   * note that direct readout and wishbone messages are put into first channel 0
   * */
#ifdef Get4pp_DOFINETIMSAMPLES
  std::vector<TGet4ppMsg*> fMessages[Get4pp_CHANNELS]; //! for finetime sample mode, do not store this into tree
#else
  std::vector<TGet4ppMsg*> fMessages[Get4pp_CHANNELS];
#endif



ClassDef(TGet4ppBoard,1)
};

/* ************************************************************************
 *
 * The top event structure
 * with all board components in the readout chain
 *
 * */
//#ifdef Get4pp_DOFINETIMSAMPLES
//class TGet4ppRawEvent: public TGo4EventElement
//#else
class TGet4ppRawEvent: public TGo4CompositeEvent
//#endif
{
public:
  TGet4ppRawEvent();
  TGet4ppRawEvent(const char* name, Short_t id = 0);
  virtual ~TGet4ppRawEvent();

  void Clear(Option_t *t = "");

  /* access to board subevent by unique id*/
  TGet4ppBoard* GetBoard(UInt_t uniqueid);

  /* This array keeps the unique id numbers of configured hit detection boards*/
  static std::vector<UInt_t> fgConfigGet4ppBoards;

  /** Event sequence number incremented by MBS Trigger*/
  Int_t fSequenceNumber;

  /** vulom status register*/
  Int_t fVULOMStatus;

  /** number of payload words in vulom buffer (u32)*/
  Int_t fDataCount;

#ifdef Get4pp_DOFINETIMSAMPLES
  TString fLmdFileName;
  Int_t fTapConfig;
  UInt_t fDelayConfig;
  Int_t fShiftChannel;
  Int_t fShiftDelay;
  /** contents of (normalized?) fine time bins at end of measurments, leading edge*/
  Double_t fFineTimeBinLeading[Get4pp_CHANNELS][Get4pp_FINERANGE+1];
  /** contents of (normalized?) fine time bins at end of measurments, trailing edge*/
  Double_t fFineTimeBinTrailing[Get4pp_CHANNELS][Get4pp_FINERANGE+1];

  /**Difference of leading edge times between channels, mean value [unit: seconds]. Note that not all indices are used*/
  Double_t fDeltaTimeLeadingMean[Get4pp_CHANNELS][Get4pp_CHANNELS];

  /**Difference of leading edge times between channels, sigma value [unit: seconds]. Note that not all indices are used*/
  Double_t fDeltaTimeLeadingSigma[Get4pp_CHANNELS][Get4pp_CHANNELS];


#endif


ClassDef(TGet4ppRawEvent,1)
};
#endif //TEVENT_H
