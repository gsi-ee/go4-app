#ifndef TNectarDISPLAY_H
#define TNectarDISPLAY_H

class TNectarRawParam;

#include "TGo4EventProcessor.h"
#include "TGo4WinCond.h"

#include <vector>
#include "TNectarRawEvent.h"

#include "TGo4Analysis.h"

#include "TLatex.h"





/********************************
 * Histogram display container for each Nectar board
 *  (JAM September 2021)
 *
 * */
class TNectarBoardDisplay: public  TGo4EventProcessor
{

public:

  TNectarBoardDisplay() : TGo4EventProcessor(), fDisplayId(0)
  {
    ;
  }
  TNectarBoardDisplay(UInt_t boardid): TGo4EventProcessor(), fDisplayId(boardid){;}
  virtual ~TNectarBoardDisplay(){;}

  /* recreate histograms using actual parameters*/
  virtual void InitDisplay(UInt_t parentid=0, Bool_t replace = kFALSE){;}

  /** reset some histograms here e*/
  virtual void ResetDisplay(){;}



   //
 UInt_t GetDevId()
     {
       return fDisplayId;
     }



protected:

  UInt_t fDisplayId;


ClassDef(TNectarBoardDisplay,1)
};
////////////////////////////////////////////////////////////////////

class TMdppDisplay: public  TNectarBoardDisplay
{
public:
  TMdppDisplay() : TNectarBoardDisplay(), hExtTimeStamp(0),hMsgTypes(0)
   {;}
   TMdppDisplay(UInt_t boardid);
   virtual ~TMdppDisplay();

   /* recreate histograms using actual parameters*/
   virtual void InitDisplay(UInt_t parentid=0, Bool_t replace = kFALSE);

   /** reset some histograms here e*/
   virtual void ResetDisplay();


   /* ADC values accumulated*/
   TH1 *hRawADC[MDPP_CHANNELS];

   /* TDC values accumulated*/
   TH1 *hRawTDC[MDPP_CHANNELS];

   /* extended time stamps*/
   TH1 *hExtTimeStamp;

   /* statistics of messages in data stream*/
   TH1* hMsgTypes;



  ClassDef(TMdppDisplay,1)
};

/////////////////////////////////////////////////////////////////////////////////////


class TVmmrSlaveDisplay: public  TNectarBoardDisplay
{
public:
  TVmmrSlaveDisplay() : TNectarBoardDisplay(), hDeltaTime(0)
   {
     ;
   }
   TVmmrSlaveDisplay(UInt_t boardid);
   virtual ~TVmmrSlaveDisplay();

   /* recreate histograms using actual parameters*/
   virtual void InitDisplay(UInt_t parentboard, Bool_t replace = kFALSE);

   /** reset some histograms here e*/
   virtual void ResetDisplay();



   /* ADC values accumulated*/
     TH1 *hRawADC[VMMR_CHANNELS];

     /* Time differenceGate start to bus triggers*/
     TH1 *hDeltaTime;


   ClassDef(TVmmrSlaveDisplay,1)
};

class TVmmrDisplay: public  TNectarBoardDisplay
{
public:
  TVmmrDisplay() : TNectarBoardDisplay(), hExtTimeStamp(0),hMsgTypes(0)
   {
     ;
   }
   TVmmrDisplay(UInt_t boardid);
   virtual ~TVmmrDisplay();

   /* recreate histograms using actual parameters*/
   virtual void InitDisplay(UInt_t parentid=0, Bool_t replace = kFALSE);

   /** reset some histograms here e*/
   virtual void ResetDisplay();

   /** Access to the slave display of chain id. Independent of position in vector*/
   TVmmrSlaveDisplay* GetSlaveDisplay(UInt_t id);

 /* extended  time stamps*/
     TH1* hExtTimeStamp;

     /* statistics of messages in data stream*/
     TH1* hMsgTypes;

protected:

   /** Dynamically build display for given chain id. Returns the handle*/
   TVmmrSlaveDisplay* AddSlaveDisplay(UInt_t id);


   /* dynamic vector of slave histogram sets */
     std::vector<TVmmrSlaveDisplay*> fSlaveDisplays;




  ClassDef(TVmmrDisplay,1)
};






#endif //TNectarDISPLAY_H

