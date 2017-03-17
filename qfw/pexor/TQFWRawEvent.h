#ifndef TQFWRAWEVENT_H
#define TQFWRAWEVENT_H



/*
 * Raw event structures for readout of QFW beam diagnostic board
 * via PEXOR family optical receiver.
 * Jörn Adamczewski-Musch, GSI-CSEE
 *
 * v0.1 29 November 2013
 * ...
 * v0.9 14 March 2017 - added temperature and fan readout
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


//JAM2017: below new stuff for thermometers and ventilators:

// number of thermometers on board
#define PEXOR_NUMTHERMS 7

// number of fans on board
#define PEXOR_NUMFANS 4


// conversion factor temperature sensors to degrees centigrade:
#define PEXOR_TEMP_TO_C 0.0625


// conversion factor fan speed to rpm:
#define PEXOR_FAN_TO_RPM 30.0





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

      /* each vector contains trace of this loop for the channel
       * JAM2016: use double value here since we might need more precise background correction*/
      std::vector<Double_t> fQfwTrace[PEXOR_QFWCHANS];


      /* the actual number of time slices used*/
      Int_t fQfwLoopSize;

     /* the actual time of each loop */
     Int_t fQfwLoopTime;

      /* setup number (only one byte used!)*/
        UChar_t fQfwSetup;

      /** true if loop was filled with new data*/
      Bool_t fHasData; //!




      ClassDef(TQFWLoop,3)
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

      Double_t GetOffset(UInt_t ch)
      {
        if(ch>=PEXOR_QFWCHANS) return 0;
        return fQfwOffsets[ch];
      }
      void SetOffset(UInt_t ch, Double_t value)
      {
        if(ch<PEXOR_QFWCHANS) fQfwOffsets[ch]=value;
      }


      Int_t GetLastEventNumber()
      {
        return fLastEventNumber;
      }

      void SetLastEventNumber(Int_t num) {fLastEventNumber=num;}


      /** Temperature sensor getter raw value*/
        UShort_t GetTempRaw(UInt_t t)
        {
          if (t >= PEXOR_NUMTHERMS)
            return 0;
          return fTemperature[t];
        }

        /** Temperature sensor setter raw value*/
        void SetTempRaw(UInt_t t, UShort_t value)
        {
          if (t < PEXOR_NUMTHERMS)
            fTemperature[t] = value;
        }

        /** Temperature sensor temperature in degrees centigrade*/
        Double_t GetTempCelsius(UInt_t t)
        {
          if (t >= PEXOR_NUMTHERMS)
            return -1000.0;
          UShort_t raw = GetTempRaw(t);
          Double_t sign = ((raw & 0x800) == 0x800) ? -1.0 : 1.0;
          Double_t rev = ((raw & 0x7FF) * PEXOR_TEMP_TO_C) * sign;
          return rev;
        }

        /** Temperature sensor getter raw value*/
        UShort_t GetFanRaw(UInt_t f)
        {
          if (f >= PEXOR_NUMFANS)
            return 0;
          return fFanSpeed[f];
        }

        /** Temperature sensor setter raw value*/
        void SetFanRaw(UInt_t f, UShort_t value)
        {
          if (f < PEXOR_NUMFANS)
            fFanSpeed[f] = value;
        }

        /** Temperature sensor temperature in degrees centigrade*/
        Double_t GetFanRPM(UInt_t f)
        {
          if (f >= PEXOR_NUMFANS)
            return -1.0;
          UShort_t raw = GetFanRaw(f);
          Double_t rev = Double_t(raw & 0xFFFF) * PEXOR_FAN_TO_RPM;
          return rev;
        }


        void SetSensorId(ULong64_t id)
        {
          fSensorId=id;
        }

        ULong64_t GetSensorId(){return fSensorId;}


        void SetVersionId(UInt_t id)
              {
                fVersionId=id;
              }

        UInt_t GetVersionId(){return fVersionId;}

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
      Double_t fQfwOffsets[PEXOR_QFWCHANS];


      /** temperature sensor raw values (16 bit)*/
      UShort_t fTemperature[PEXOR_NUMTHERMS];

      /** fan frequency (Hz)*/
      UShort_t fFanSpeed[PEXOR_NUMFANS];

      /** ID of the temperature sensor on board. Will be used to unique identification.
       * Ideally this is fixed linked to the unique id of the setup*/
      ULong64_t fSensorId;


      /** firmware version id contained in data. Expresses Decimal Date info as pseudo hex numbers,
       * e.g. 0x17022810 means year 17, month 02, day 28, version 1.0*/
      UInt_t fVersionId;

      /** check sequence number of events and report missing events*/
      Int_t fLastEventNumber; //!

      ClassDef(TQFWBoard,5)
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
       
       /** Sequence number of events */
       Int_t fSequenceNumber;

       

	   ClassDef(TQFWRawEvent,1)
};
#endif //TEVENT_H



