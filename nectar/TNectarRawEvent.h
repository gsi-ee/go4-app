#ifndef TNECTARRAWEVENT_H
#define TNECTARRAWEVENT_H

/*
 * Raw event structures for readout of NECTAR setup
 * Jörn Adamczewski-Musch, GSI-CSEE
 *
 * v0.1 2 September 2021
 *
 */

#include "TGo4EventElement.h"
#include "TGo4CompositeEvent.h"

#include <vector>
#include "TString.h"

// maximum number of boards to investigate at once
#define MDPP_MAXBOARDS 10
// maximum number of boards to investigate at once
#define VMMR_MAXBOARDS 10


/* number of readout channels of single mdpp module*/
#define MDPP_CHANNELS 32

/* number of external trigger inputs for delta time (MDPP32 only)*/
#define MDPP_EXTDTCHANNELS 2

/* range of possible ADC values*/
#define MDPP_ADC_RANGE 0xFFFF

/* range of possible TDC values*/
#define MDPP_TDC_RANGE 0xFFFF

/* range of extended time stamp values*/
#define MDPP_TIMESTAMP_RANGE 0xFFFF




/* number of optical frontend chains for each VMMR board*/
#define VMMR_CHAINS 16

/* range of possible frontend subaddresses*/
#define VMMR_SUBADDRESS_RANGE 0xFFF

/* maximum number of channels accounted for each VMMR frontend (maximum subaddress is 12 bit?)*/
#define VMMR_CHANNELS 128



/* range of possible ADC values for VMMR*/
#define VMMR_ADC_RANGE 0xFFF

/* range of extended time stamp values*/
#define VMMR_TIMESTAMP_RANGE 0xFFFF

/** range of possible deltat values*/
#define VMMR_DT_RANGE 0xFFFF


/** this is offset of composite event ids for VMMR. Need this to handle both MDPP and VMMRsubcomponents
 * in the single TNectarRawEvent container. By this it is possible to differ same module ids in composite subevents*/
#define VMMR_COMPOSITE_ID_OFFSET 100


/******************************************************************
 *
 * Base class represents one Nectar readout board w
 */

class TNectarBoard: public TGo4EventElement
{
public:
  TNectarBoard();
  /* */
  TNectarBoard(const char* name, UInt_t unid, Short_t index);
  virtual ~TNectarBoard();


  /** Method called by the framework to clear the event element. */
  void Clear(Option_t *t = "");

  /* get unique hardware id of board in set up*/
  UInt_t GetBoardId()
  {
    return fUniqueId;
  }

  UInt_t GetEventCount()
   {
     return (fEventCounter & 0x3FFFFFFF);
   }

  /* 30 bit event counter/time stamp from "end of event mark"*/
   UInt_t fEventCounter;

   /* high 16 bits of  most recent time stamp*/
   UShort_t fExtendedTimeStamp;


private:
  /* unique hardware id of the board. THis should be independent of position in readout chain*/
  UInt_t fUniqueId;

ClassDef(TNectarBoard,1)
};


/////////////////////////////////////////////////////////////////


class TMdppMsg
{

public:

  enum MsgType
   {
     MSG_HEADER = 0, MSG_ADC = 1, MSG_TDC = 2, MSG_TIMESTAMP = 3, MSG_EOE = 4
   };


   TMdppMsg(){}

  TMdppMsg(UShort_t value) : fData(value)
   {
    ;
  }
  virtual ~TMdppMsg()
  {
    ;
  }

  /* data word. may be either ADC or TDC*/
  UShort_t fData;

};


class TMdppAdcData:  public TMdppMsg
{
  public:
   TMdppAdcData():TMdppMsg(){}
  TMdppAdcData(UShort_t adcvalue): TMdppMsg(adcvalue)
  {}
  virtual ~TMdppAdcData(){;}

};

class TMdppTdcData:  public TMdppMsg
{
  public:
  TMdppTdcData():TMdppMsg(){}
  TMdppTdcData(UShort_t tdcvalue):  TMdppMsg(tdcvalue)
  {}
  virtual ~TMdppTdcData(){;}

};


/** data of single mdpp board for one event */
class TMdppBoard: public TNectarBoard
{
public:
  TMdppBoard();
  /* */
  TMdppBoard(const char* name, UInt_t unid, Short_t index);
  virtual ~TMdppBoard();


  /** Method called by the framework to clear the event element. */
  void Clear(Option_t *t = "");

  /** Add data from TDC of channel to  buffer */
  void AddTdcMessage(TMdppTdcData* msg, UChar_t channel);

  /** Number of TDC data messages of channel in buffer */
  UInt_t NumTdcMessages(UChar_t channel);

  /** Access ADC data messages of channel at position i in buffer */
   TMdppTdcData* GetTdcMessage(UChar_t channel, UInt_t i);


   /** Add delta time data from  of external trigger input trigchan to buffer */
   void AddExtDTMessage(TMdppTdcData* msg, UChar_t trigchan);

   /** Number of delta time data messages of external trigger input trigchan in buffer */
   UInt_t NumExtDTMessages(UChar_t trigchan);

   /** Access ADC data messages of channel at position i in buffer */
   TMdppTdcData* GetExtDtMessage(UChar_t trigchan, UInt_t i);



  /** Add data from ADC of channel to  buffer */
  void AddAdcMessage(TMdppAdcData* msg, UChar_t channel);

  /** Number of ADC data messages of channel in buffer */
  UInt_t NumAdcMessages(UChar_t channel);

  /** Access ADC data messages of channel at position i in buffer */
  TMdppAdcData* GetAdcMessage(UChar_t channel, UInt_t i);

  /* ADC resolution information from event header*/
  Short_t GetAdcResolution();

  /* TDC resolution information from event header*/
  Short_t GetTdcResolution();

  void SetResolution(Short_t value)
  {
    fResolution=value;
  }

  protected:

  /* TDC and ADC resolution information from event header*/
  Short_t fResolution;



  std::vector<TMdppAdcData*> fAdcMessages[MDPP_CHANNELS];
  std::vector<TMdppTdcData*> fTdcMessages[MDPP_CHANNELS];

  std::vector<TMdppTdcData*> fExtTrigDT[MDPP_EXTDTCHANNELS];


  ClassDef(TMdppBoard,1)
};


//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

/* data of VMMR frontend*/
class TVmmrMsg
{

public:


  enum MsgType
     {
       MSG_HEADER = 0, MSG_ADC = 1, MSG_DELTA_T = 2, MSG_TIMESTAMP = 3, MSG_EOE = 4
     };
     
TVmmrMsg(){;}
  TVmmrMsg(UShort_t data, UShort_t FE_subaddress): fData(data),fFrontEndSubaddress(FE_subaddress)
   {
    ;
  }
  virtual ~TVmmrMsg()
  {
    ;
  }

  /* data word. may be either ADC or something else*/
    UShort_t fData;

    /* subadress of connected slave. May be channel number or something else*/
    UShort_t fFrontEndSubaddress;
};



class TVmmrAdcData:  public TVmmrMsg
{
public:
    TVmmrAdcData():TVmmrMsg(){}
  TVmmrAdcData(UShort_t adcvalue, UShort_t channel):
    TVmmrMsg((adcvalue & VMMR_ADC_RANGE), (channel & VMMR_SUBADDRESS_RANGE))
  {}

  virtual ~TVmmrAdcData(){;}
};





/* data of one frontend readout board at Vmrr*/
class TVmmrSlave
{

public:

  TVmmrSlave();

   virtual ~TVmmrSlave();

   void Clear(Option_t *t = "");

   void AddAdcMessage(TVmmrAdcData* msg);


   /** Number of ADC data messages of channel in buffer */
   UInt_t NumAdcMessages();

     /** Access ADC data messages at position i in buffer */
   TVmmrAdcData* GetAdcMessage(UInt_t i);



   /* Time difference - Gate start to bus trigger */
   Short_t fDeltaTGate;

protected:

   std::vector<TVmmrAdcData*> fMessages;

   ClassDef(TVmmrSlave,1)
};

/** data of single mdpp board for one event */
class TVmmrBoard: public TNectarBoard
{
public:
  TVmmrBoard();
  /* */
  TVmmrBoard(const char* name, UInt_t unid, Short_t index);
  virtual ~TVmmrBoard();

  /** Access to substructure for frontend slave board connected at chain bus number of id*/
  TVmmrSlave* GetSlave(UInt_t id);



  /** Method called by the framework to clear the event element. */
  void Clear(Option_t *t = "");

protected:

  /** Create new substucture for frontend slave board connected at chain bus number of id*/
  TVmmrSlave* AddSlave(UInt_t id);

 /* dynamic vector of slave data */
  std::vector<TVmmrSlave*> fSlaves;

  ClassDef(TVmmrBoard,1)
};




/* ************************************************************************
 *
 * The top event structure
 * with all board components in the readout chain
 *
 * */

class TNectarRawEvent: public TGo4CompositeEvent
{
public:
  TNectarRawEvent();
  TNectarRawEvent(const char* name, Short_t id = 0);
  virtual ~TNectarRawEvent();

  void Clear(Option_t *t = "");


  /* access to VMMR board subevent by unique id*/
    TVmmrBoard* GetVmmrBoard(UInt_t uniqueid);

    /* access to MDPP board subevent by unique id*/
    TMdppBoard* GetMdppBoard(UInt_t uniqueid);


  /* access to board subevent by unique id*/
  TNectarBoard* GetBoard(UInt_t uniqueid);


  /* This array keeps the unique id numbers of configured VMMR boards*/
  static std::vector<UInt_t> fgConfigVmmrBoards;

  /* This array keeps the unique id numbers of configured MDPP boards*/
  static std::vector<UInt_t> fgConfigMdppBoards;

  /** Event sequence number incremented by MBS Trigger*/
  Int_t fSequenceNumber;


ClassDef(TNectarRawEvent,1)
};
#endif //TEVENT_H
