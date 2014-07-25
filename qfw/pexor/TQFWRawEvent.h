#ifndef TQFWRAWEVENT_H
#define TQFWRAWEVENT_H



/*
 * Raw event structures for readout of QFW beam diagnostic board
 * via PEXOR family optical receiver.
 * Jörn Adamczewski-Musch, GSI-CSEE
 *
 * v0.1 29 November 2013
 *
 */









#include "TGo4EventElement.h"
#include "TGo4CompositeEvent.h"


// maximum number of sfp readout chains used:
#define PEXOR_MAXSFP 4
// maximum number of devices per sfpchain:
#define PEXOR_MAXDEVS 10

// number of loops
#define PEXOR_QFWLOOPS  3

// maximum number of slices in loop
#define PEXOR_QFWSLICES 20

// number of channels in one board
#define PEXOR_QFWCHANS  32
// number of QFW in one board
#define PEXOR_QFWNUM 8






#define DEADTIME_BINS 4096
#define FC_BINS 4096

/*
 * This class represents data of one single time loop over all qfw channels of one board
 */
class TQFWLoop : public TGo4EventElement {
   public:
      TQFWLoop();
      TQFWLoop(const char* name, Short_t ix);
      virtual ~TQFWLoop();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");


      /* Evaluate actual charge calibration from setup*/
      Double_t GetCoulombPerCount();

      /* Evaluate time per slice from setup*/
      Double_t GetMicroSecsPerTimeSlice();

      /* build setup string*/
      TString GetSetupString();

      /* each vector contains trace of this loop for the channel*/
      std::vector<Int_t> fQfwTrace[PEXOR_QFWCHANS];


      /* the actual number of time slices used*/
      Int_t fQfwLoopSize;

     /* the actual time of each loop */
     Int_t fQfwLoopTime;

      /* setup number (only one byte used!)*/
        UChar_t fQfwSetup;

      ClassDef(TQFWLoop,1)
};



/******************************************************************
 *
 * This class represents one QFW board with 8 qfws.
 */

class TQFWBoard : public TGo4CompositeEvent {
   public:
      TQFWBoard();
      /* */
      TQFWBoard(const char* name, UInt_t unid, Short_t index);
      virtual ~TQFWBoard();

      /* get unique hardware id of board in set up*/
      UInt_t GetBoardId(){return fUniqueId;}

      /* access to the time loop data object*/
      TQFWLoop* GetLoop(UInt_t ix)
         {
            //return &fQfwLoop[ix];
            return (TQFWLoop*) getEventElement(ix);
         }

      /* error scaler value of qfw 0...8 on the board*/
      UInt_t GetErrorScaler(UInt_t qfwid)
         {
            if(qfwid>=PEXOR_QFWNUM) return 0;
            return fQfwErr[qfwid];
         }

      void SetErrorScaler(UInt_t qfwid, UInt_t value)
         {
            if(qfwid<PEXOR_QFWNUM) fQfwErr[qfwid]=value;
         }

      Int_t GetOffset(UInt_t ch)
      {
        if(ch>=PEXOR_QFWCHANS) return 0;
        return fQfwOffsets[ch];
      }
      void SetOffset(UInt_t ch, UInt_t value)
      {
        if(ch<PEXOR_QFWCHANS) fQfwOffsets[ch]=value;
      }


      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");

      /* setup number (only one byte used!)*/
        UChar_t fQfwSetup;

       TString GetSetupString();


      /* build setup string from value. generic helper used in loop and board until we rearrange data...*/
      static TString GetSetupString(UChar_t setup);


   private:

      /* unique hardware id of the board. THis should be independent of position in readout chain*/
      UInt_t fUniqueId;

      /* error scaler for each qfw on the board*/
      UInt_t fQfwErr[PEXOR_QFWNUM];

      /* Offset values as measured by last frontend trigger*/
      Int_t fQfwOffsets[PEXOR_QFWCHANS];


      ClassDef(TQFWBoard,2)
};


/* ************************************************************************
 *
 * The top event structure
 * with all board components in the readout chain
 *
 * */

class TQFWRawEvent : public TGo4CompositeEvent {
   public:
      TQFWRawEvent();
      TQFWRawEvent(const char* name, Short_t id=0);
      virtual ~TQFWRawEvent();
      
      void Clear(Option_t *t="");
      
      /* access to board subevent by unique id*/
      TQFWBoard* GetBoard(UInt_t uniqueid);


      /* This array keeps the unique id numbers of configured qfw boards*/
       static std::vector<UInt_t> fgConfigQFWBoards;
       
       Int_t fSequenceNumber;

       

	   ClassDef(TQFWRawEvent,1)
};
#endif //TEVENT_H



