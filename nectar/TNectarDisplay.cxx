#include "TNectarDisplay.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "TGo4Analysis.h"
#include "TGo4Log.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"

#include "TNectarRawEvent.h"
#include "TNectarRawParam.h"

//////////////////////////////////////////////////////////////////////////////////////////////////



//***********************************************************

TMdppDisplay::TMdppDisplay(UInt_t boardid) :
  TNectarBoardDisplay(boardid)
{

  printf("TMdppDisplay ctor of id %d\n", fDisplayId);


}

//////////////////////////////



TMdppDisplay::~TMdppDisplay()
{

}


void TMdppDisplay::InitDisplay(UInt_t dummy, Bool_t replace)
{


    if (replace)
      SetMakeWithAutosave(kFALSE);

    TGo4Log::Info("TMdppDisplay: Initializing histograms with replace=%d for Board %d ",
        replace,
        GetDevId());

    // Creation of histograms (or take them from autosave)
    TString obname;
    TString obtitle;
    TString foldername;
    TString binlabel;
    Int_t brd = fDisplayId;

    for(Int_t ch=0; ch<MDPP_CHANNELS; ++ch)
    {
      obname.Form("Raw/MDPP/Board_%d/ADC/ADC_%d_Channel_%d", brd, brd, ch);
      obtitle.Form("MDPP Board %d ADC Channel %d ", brd, ch);
      hRawADC[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), MDPP_ADC_RANGE, 0, MDPP_ADC_RANGE, "ADC value", "counts");

      obname.Form("Raw/MDPP/Board_%d/TDC/TDC_%d_Channel_%d", brd, brd, ch);
      obtitle.Form("MDPP Board %d TDC Channel %d ", brd, ch);
      hRawTDC[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), MDPP_TDC_RANGE, 0, MDPP_TDC_RANGE, "ADC value", "counts");

    }// for ch

    obname.Form("Raw/MDPP/Board_%d/MDPP_ExtendedTimestamps_%d", brd, brd);
    obtitle.Form("MDPP Board %d Extended timestamps", brd);
    hExtTimeStamp = MakeTH1('I', obname.Data(), obtitle.Data(), MDPP_TIMESTAMP_RANGE, 0, MDPP_TIMESTAMP_RANGE, "Timestamp high word", "counts");


    obname.Form("Raw/MDPP/Board_%d/MDPP_MsgTypes_%d", brd, brd);
    obtitle.Form("MDPP Board %d Message types", brd);
    hMsgTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 5, 0, 5);
    if (IsObjMade()) {
          hMsgTypes->GetXaxis()->SetBinLabel(1 + TMdppMsg::MSG_HEADER, "HEAD");
          hMsgTypes->GetXaxis()->SetBinLabel(1 + TMdppMsg::MSG_ADC, "ADC");
          hMsgTypes->GetXaxis()->SetBinLabel(1 + TMdppMsg::MSG_TDC, "TDC");
          hMsgTypes->GetXaxis()->SetBinLabel(1 + TMdppMsg::MSG_TIMESTAMP, "ExtTS");
          hMsgTypes->GetXaxis()->SetBinLabel(1 + TMdppMsg::MSG_EOE, "EoE");
        }




    SetMakeWithAutosave(kTRUE);

//    obname.Form("ADC_direct_Brd%d_Tr", brd);
//  TGo4Picture* pic = GetPicture(obname.Data());
//  if (pic == 0)
//  {
//    obtitle.Form("ADC direct trace Board%d", brd);
//    pic = new TGo4Picture(obname.Data(), obtitle.Data());
//
//    pic->SetDivision(2, 2);
//    pic->Pic(0, 0)->AddObject(hTraceLongPrev);
//    pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
//    pic->Pic(0, 0)->AddObject(cWindowFFT);
//
//    pic->Pic(0, 1)->AddObject(hTracePartFFT);
//    pic->Pic(0, 1)->SetLogScale(1);
//    pic->Pic(0, 1)->SetLineAtt(4, 1, 1);
//    pic->Pic(0, 1)->SetFillAtt(4, 3001);
//
//    pic->Pic(1, 0)->AddObject(hTraceLong);
//    pic->Pic(1, 0)->SetLineAtt(3, 1, 1);
//    pic->Pic(1, 0)->SetFillAtt(3, 3001);
//
//    pic->Pic(1, 1)->AddObject(hTraceLongFFT);
//    pic->Pic(1, 1)->SetLogScale(1);
//    pic->Pic(1, 1)->SetLineAtt(4, 1, 1);
//    pic->Pic(1, 1)->SetFillAtt(4, 3001);
//    AddPicture(pic, Form("Board%d", brd));
//  }



}


void TMdppDisplay::ResetDisplay()
{
  // if something needs to be cleared for each event, do this here

//  for(Int_t ch=0; ch<MDPP_CHANNELS; ++ch)
//      {
////        hTrace[ch]->Reset();
//
//      }// for ch

}


///////////////////////////////////////////////////////////////




TVmmrDisplay::TVmmrDisplay(UInt_t boardid) : TNectarBoardDisplay(boardid)
{

  printf("TVmmrDisplay ctor of id %d\n", boardid);

}




TVmmrDisplay::~TVmmrDisplay()
{
  for (UInt_t sl = 0; sl < fSlaveDisplays.size(); ++sl)
    {
      delete (fSlaveDisplays[sl]);
    }
    fSlaveDisplays.clear();
}


TVmmrSlaveDisplay* TVmmrDisplay::GetSlaveDisplay(UInt_t id)
{
  TVmmrSlaveDisplay* theSlave = 0;
  for (UInt_t i = 0; i < fSlaveDisplays.size(); ++i)
  {
    theSlave = fSlaveDisplays[i];
    if (theSlave->GetDevId() == id)
      return theSlave;
  }
  if(theSlave==0) return AddSlaveDisplay(id);
  return 0; // never come here.
}

TVmmrSlaveDisplay* TVmmrDisplay::AddSlaveDisplay(UInt_t id)
{

  TVmmrSlaveDisplay* theSlave = new TVmmrSlaveDisplay(id);
  theSlave->InitDisplay(kTRUE);
  fSlaveDisplays.push_back(theSlave);
  return theSlave;

}

void TVmmrDisplay::InitDisplay(UInt_t dummy, Bool_t replace)
{
  // first global  histograms for all slaves:

  if (replace)
     SetMakeWithAutosave(kFALSE);

   TGo4Log::Info("TVmmrDisplay: Initializing histograms with replace=%d for Board %d ",
       replace,
       GetDevId());

   // Creation of histograms (or take them from autosave)
   TString obname;
   TString obtitle;
   Int_t brd = fDisplayId;


   obname.Form("Raw/VMMR/Board_%d/VMRR_ExtendedTimestamps_%d", brd, brd);
   obtitle.Form("VMMR Board %d Extended timestamps", brd);
   hExtTimeStamp = MakeTH1('I', obname.Data(), obtitle.Data(), VMMR_TIMESTAMP_RANGE, 0, VMMR_TIMESTAMP_RANGE, "Timestamp high word", "counts");

   obname.Form("Raw/VMMR/Board_%d/VMMR_MsgTypes_%d", brd, brd);
   obtitle.Form("VMMR Board %d Message types", brd);
   hMsgTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 5, 0, 5);
   if (IsObjMade()) {
            hMsgTypes->GetXaxis()->SetBinLabel(1 + TVmmrMsg::MSG_HEADER, "HEAD");
            hMsgTypes->GetXaxis()->SetBinLabel(1 + TVmmrMsg::MSG_ADC, "ADC");
            hMsgTypes->GetXaxis()->SetBinLabel(1 + TVmmrMsg::MSG_DELTA_T, "DT");
            hMsgTypes->GetXaxis()->SetBinLabel(1 + TVmmrMsg::MSG_TIMESTAMP, "ExtTS");
            hMsgTypes->GetXaxis()->SetBinLabel(1 + TVmmrMsg::MSG_EOE, "EoE");
   }



  for (UInt_t sl = 0; sl < fSlaveDisplays.size(); ++sl)
      {
      fSlaveDisplays[sl]->InitDisplay(brd, replace);
      }
}


void TVmmrDisplay::ResetDisplay()
{
  // if something needs to be cleared for each event, do this here
  for (UInt_t sl = 0; sl < fSlaveDisplays.size(); ++sl)
        {
        fSlaveDisplays[sl]->ResetDisplay();
        }
}

/////////////////////////////////

TVmmrSlaveDisplay::TVmmrSlaveDisplay(UInt_t boardid) :
     TNectarBoardDisplay(boardid)
{

  printf("TVmmrSlaveDisplay ctor of id %d\n", boardid);







}




TVmmrSlaveDisplay::~TVmmrSlaveDisplay()
{

}


void TVmmrSlaveDisplay::InitDisplay(UInt_t parentboard, Bool_t replace)
{
  if (replace)
       SetMakeWithAutosave(kFALSE);

     TGo4Log::Info("TVmmrSlaveDisplay: Initializing histograms with replace=%d for Board %d ",
         replace,
         GetDevId());

     // Creation of histograms (or take them from autosave)
     TString obname;
     TString obtitle;
     TString foldername;
     TString binlabel;
     Int_t brd = fDisplayId;

     for(Int_t ch=0; ch<VMMR_CHANNELS; ++ch)
     {
       obname.Form("Raw/VMMR/Board_%d/Slave_%d/VMMR_ADC_%d_%d_Channel_%d", parentboard, brd, parentboard, brd, ch);
       obtitle.Form("VMMR Board %d Frontend %d ADC Channel %d ", parentboard, brd, ch);
       hRawADC[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), VMMR_ADC_RANGE, 0, VMMR_ADC_RANGE, "ADC value", "counts");
     }

     obname.Form("Raw/VMMR/Board_%d/Slave_%d/VMMR_DeltaT_%d_%d", parentboard, brd, parentboard, brd);
     obtitle.Form("VMMR Board %d Frontend %d : Time difference (Gate start - trigger)", parentboard, brd);
     hDeltaTime= MakeTH1('I', obname.Data(), obtitle.Data(), VMMR_DT_RANGE, 0, VMMR_DT_RANGE, "Delta T", "counts");;
}


void TVmmrSlaveDisplay::ResetDisplay()
{
  // if something needs to be cleared for each event, do this here
}






