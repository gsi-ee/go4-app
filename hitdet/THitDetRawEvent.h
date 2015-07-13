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

// maximum number of boards to investigate at once
#define HitDet_MAXBOARDS 10


// number of data channels per hit detection board
#define HitDet_CHANNELS 4

// length of sampled trace per message
#define HitDet_TRACEBINS 32

/**
 * Base class for single "message" inside data stream */
class THitDetMsg {

public:
    THitDetMsg(){;}
    virtual ~THitDetMsg(){;}
};

/**
 * Message class for (chip event-) triggered data  */
class THitDetMsgEvent : public THitDetMsg
{

public:
    THitDetMsgEvent();
    virtual ~THitDetMsgEvent();

    // JAM TODO


};


/**
 * Message class for direct ADC readout data  */
class THitDetMsgDirect : public THitDetMsg
{

public:
    THitDetMsgDirect();
    virtual ~THitDetMsgDirect();

 // JAM TODO

};

/**
 * Message class for Wishbone response message  */
class THitDetMsgWishbone : public THitDetMsg
{

public:
    THitDetMsgWishbone(){;}
    THitDetMsgWishbone(UChar_t header): THitDetMsg(), fWishboneHeader(header){}
    virtual ~THitDetMsgWishbone(){;}

    UChar_t GetAckCode(){return ((fWishboneHeader >> 4) & 0x3);}
    UChar_t GetSource(){return (fWishboneHeader & 0xF);}

    void SetHeader(UChar_t head){fWishboneHeader=head;}

    void SetAddress(UChar_t add){fRegAddress=add;}
    void SetData(UChar_t dat, Int_t i){if(i>0 && i<4) fRegData[i]=dat;}


private:

    /** acknowledge type (2 bit)*/
    UChar_t fWishboneHeader;

    /** address of dumped data register (optional)*/
    UChar_t fRegAddress;

    /** data from register (optional)*/
    UChar_t fRegData[4];



};





/******************************************************************
 *
 * This class represents one HitDetection readout board with one HitDetection asic each
 */

class THitDetBoard : public TGo4EventElement {
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
      void Clear(Option_t *t="");


      /* get unique hardware id of board in set up*/
      UInt_t GetBoardId(){return fUniqueId;}


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

class THitDetRawEvent : public TGo4CompositeEvent {
   public:
      THitDetRawEvent();
      THitDetRawEvent(const char* name, Short_t id=0);
      virtual ~THitDetRawEvent();
      
      void Clear(Option_t *t="");
      
      /* access to board subevent by unique id*/
      THitDetBoard* GetBoard(UInt_t uniqueid);


      /* This array keeps the unique id numbers of configured hit detection boards*/
       static std::vector<UInt_t> fgConfigHitDetBoards;
       
       Int_t fSequenceNumber;

       

	   ClassDef(THitDetRawEvent,1)
};
#endif //TEVENT_H



