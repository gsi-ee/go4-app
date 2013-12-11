#ifndef TQFWPROFILEEVENT_H
#define TQFWPROFILEEVENT_H



#define PEXOR_QFW_GRIDS 2
#define PEXOR_QFW_CUPS 5

/* default number of wires for first init:*/
#define PEXOR_QFW_WIRES 32

/* default number of wires for first init:*/
#define PEXOR_QFW_CUPSEGMENTS 2


/* this enables storing the background corrected wire currents to ROOT tree
 * will decrease performance!*/
//#define QFW_STORECURRENTS 1


/*
 * Advanced event structures for readout of QFW beam diagnostic board
 * via PEXOR family optical receiver.
 * Jörn Adamczewski-Musch, GSI-CSEE
 *
 * v0.3 11 December 2013
 *
 */


#include "TGo4CompositeEvent.h"

class TQFWProfileParam;


// TODO composite events here, collection of grids/cups

/*
 * This class contains mapping between one single grid/cup wire and
 * qfw board/channel
 * */

class TQFWChannelMap
{
  public:
  TQFWChannelMap(){;}
  TQFWChannelMap(UInt_t boardid, Int_t qfwchannel): fBoardID(boardid), fQFWChannel(qfwchannel) {}
  virtual ~TQFWChannelMap(){;}


  /* unique board id for this channel of the grid/cup etc*/
    UInt_t fBoardID;
    /* qfw channel on this board connected to this channel*/
    Int_t fQFWChannel;
    ClassDef(TQFWChannelMap,1)
};


class TQFWGrid : public TGo4EventElement
{

public:
     TQFWGrid(){;}
     /* */
     TQFWGrid(const char* name, UInt_t unid, Short_t index);
     virtual ~TQFWGrid();

     /* get unique hardware id of board in set up*/
     UInt_t GetGridId(){return fUniqueId;}

     Int_t GetNumXWires()
       {
         return fXChannels.size();
       }

     Int_t GetNumYWires()
         {
             return fYChannels.size();
         }
     TQFWChannelMap& GetXChannelMap(Int_t index)
     {
       return fXChannels[index];
     }

     TQFWChannelMap& GetYChannelMap(Int_t index)
          {
            return fYChannels[index];
          }


     /* set up map for X wires of this grid.
      * wire number will be incremented with each call*/
     void AddXMapping(UInt_t board ,Int_t channel);

     /* set up map for Y wires of this grid.
          * wire number will be incremented with each call*/
     void AddYMapping(UInt_t board ,Int_t channel);

     /* erase previous mappings*/
     void ClearMappings();

     /** Method called by the framework to clear the event element. */
     void Clear(Option_t *t="");

     /* references to qfw board and channels for x wires*/
     std::vector<TQFWChannelMap> fXChannels; //! do not put this into tree

     /* references to qfw board and channels for y wires*/
     std::vector<TQFWChannelMap> fYChannels; //! do not put this into tree


     /* calibrated charge/current for each X wire*/
      std::vector<Double_t> fXCurrent;

      /* calibrated charge/current for each Y wire*/
            std::vector<Double_t> fYCurrent;

     /* mean beam position from current x profile*/
          Double_t fBeamMeanX;

          /* mean beam position from current y profile*/
          Double_t fBeamMeanY;

          /* beam rms width from current x profile*/
          Double_t fBeamRMSX;

          /* beam rms width from current y profile*/
          Double_t fBeamRMSY;



  private:

     /* unique hardware id of the grid. */
     UInt_t fUniqueId;


  ClassDef(TQFWGrid,1)
};



class TQFWCup : public TGo4EventElement
{

public:
     TQFWCup(){;}
     /* */
     TQFWCup(const char* name, UInt_t unid, Short_t index);
     virtual ~TQFWCup();

     /* get unique hardware id of board in set up*/
     UInt_t GetCupId(){return fUniqueId;}



     /* set up map for segements of this cup.
      * cup segment number will be incremented with each call*/
     void AddMapping(UInt_t board ,Int_t channel);

     /** Method called by the framework to clear the event element. */
     void Clear(Option_t *t="");

     /* erase previous mappings*/
         void ClearMappings();


     Int_t GetNumSegments()
               {
                 return fChannels.size();
               }
     TQFWChannelMap& GetChannelMap(Int_t index)
        {
          return fChannels[index];
        }

     /* references to qfw board and channels for cup segments*/
     std::vector<TQFWChannelMap> fChannels; //! do not put this into tree

     /* calibrated charge/current*/
     std::vector<Double_t> fCurrent;


  private:

     /* unique hardware id of the grid. */
     UInt_t fUniqueId;

  ClassDef(TQFWCup,1)
};







/* ************************************************************************
 *
 * The top event structure
 * with all board components in the readout chain
 *
 * */

class TQFWProfileEvent : public TGo4CompositeEvent
{
   public:
      TQFWProfileEvent();
      TQFWProfileEvent(const char* name, Short_t id=0);
      virtual ~TQFWProfileEvent();


      /* this function will initialize geometry and mapping for grids
       * from values stored in fParameter*/
      void SetupGrids();

      /* this function will initialize geometry and mapping for cups
       * from values stored in fParameter*/
      void SetupCups();




      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");

      /* access to profile grid subevent by unique id*/
      TQFWGrid* GetGrid(UInt_t uniqueid);

      /* access to profile cup subevent by unique id*/
      TQFWCup* GetCup(UInt_t uniqueid);


//	   /* This array keeps the unique id numbers of configured qfw boards*/
//	    static std::vector<UInt_t> fgConfigQFWGrids; //!
//
//	    /* This array keeps the unique id numbers of configured qfw boards*/
//	    static std::vector<UInt_t> fgConfigQFWCups; //!

	    static TQFWProfileParam* fParameter; //!


	   ClassDef(TQFWProfileEvent,1)
};
#endif //TQFWPROFILEEVENT_H



