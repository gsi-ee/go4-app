#include "THitDetDisplay.h"

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

#include "THitDetRawEvent.h"
#include "THitDetRawParam.h"

//////////////////////////////////////////////////////////////////////////////////////////////////



//***********************************************************

THitDetBoardDisplay::THitDetBoardDisplay(Int_t boardid) :
  fDisplayId(boardid)
{

  printf("THitDetBoardDisplay ctor of id %d\n", fDisplayId);
  for(Int_t ch=0; ch<HitDet_CHANNELS; ++ch)
    for(Int_t i=0; i<HitDet_MAXSNAPSHOTS; ++i)
      hTraceSnapshots[ch][i]=0;

  SetMakeWithAutosave(kTRUE);
  TString obname;
  Int_t brd = fDisplayId;
  obname.Form("WishboneDump_%d", brd);
  lWishboneText = new TLatex(0.2,0.8,"-- wishbone dump --");
  lWishboneText->SetName(obname.Data());
  lWishboneText->SetNDC(); // relative pad x,y coordinates [0...1]
  AddObject(lWishboneText); // always replace previous label




  for(Int_t ch=0; ch<HitDet_CHANNELS; ++ch)
      {
        hTrace[ch] = 0;
        hTraceSum[ch] = 0;
        for(Int_t i=0; i<HitDet_MAXSNAPSHOTS; ++i)
        {
          hTraceSnapshots[ch][i]=0;
        }
        hTraceSnapshot2d[ch]=0;
      }



}

THitDetBoardDisplay::~THitDetBoardDisplay()
{

}


void THitDetBoardDisplay::InitDisplay(Int_t tracelength, Int_t numsnapshots, Bool_t replace)
{


    if (replace)
      SetMakeWithAutosave(kFALSE);

    TGo4Log::Info("THitDetBoardDisplay: Initializing histograms with tracelength %d snapshots %d for Board %d ",
        tracelength,numsnapshots,
        GetDevId());

    // Creation of histograms (or take them from autosave)
    TString obname;
    TString obtitle;
    TString foldername;
    TString binlabel;
    Int_t brd = fDisplayId;

    for(Int_t ch=0; ch<HitDet_CHANNELS; ++ch)
    {
      obname.Form("Board%d/Channel%d/Trace_%d_%d_Last", brd, ch, brd, ch);
      obtitle.Form("HitDetection Board %d Channel %d Last Trace", brd, ch);
      hTrace[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

      obname.Form("Board%d/Channel%d/SumTrace_%d_%d", brd, ch, brd, ch);
      obtitle.Form("Accumulated HitDetection Board %d Channel %d Trace", brd, ch);
      hTraceSum[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

      for(Int_t i=0; i<HitDet_MAXSNAPSHOTS; ++i)
      {
//          if(i>=HitDet_MAXSNAPSHOTS) continue;
          obname.Form("Board%d/Channel%d/Trace_%d_%d_%d", brd, ch, brd, ch,i);
          obtitle.Form("HitDetection Board %d Channel %d Trace %d", brd, ch, i);
          hTraceSnapshots[ch][i] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

     } //for i
      obname.Form("Board%d/Channel%d/TraceSnapshots_%d_%d", brd, ch, brd, ch);
      obtitle.Form("HitDetection Board %d Channel %d Trace snapshot overview", brd, ch);
      hTraceSnapshot2d[ch]=MakeTH2('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, numsnapshots, 0, numsnapshots -1,"time (bins)", "message sequence", "counts");

    }// for ch
    obname.Form("Board%d/TraceLong_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace (direct ADC only)", brd);
    Int_t totlength= 8 * HitDet_MAXTRACELONG; // JAM todo: different variable for stitched length here later
    hTraceLong=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongFull_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Last complete Stitched Trace (direct ADC only)", brd);
    hTraceLongPrev=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongSum_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace Sums(direct ADC only)", brd);
    hTraceLongSum=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongFFT_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace FFT (direct ADC only)", brd);
    hTraceLongFFT=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "1/t ", "counts");


    obname.Form("Board%d/TracePartFFT_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace FFT from window(direct ADC only)", brd);
    hTracePartFFT=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "1/t ", "counts");


    obname.Form("Board%d/ADC_Values_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Accumulated ADC values", brd);
    hADCValues=MakeTH1('I', obname.Data(), obtitle.Data(), 4095, -2048, 2047, "ADC value", "counts");

    obname.Form("Board%d/ADC_DeltaValues_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Accumulated ADC mean deviation", brd);
    hADCDeltaMeanValues=MakeTH1('D', obname.Data(), obtitle.Data(), 4095, -2048, 2047, "ADC value", "#delta Mean");

    obname.Form("Board%d/ADC_DiffNL_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Differential nonlinearity", brd);
    hADCNonLinDiff=MakeTH1('D', obname.Data(), obtitle.Data(), 4095, -2048, 2047, "ADC value", "DNL");

    obname.Form("Board%d/ADC_IntNL_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Integral nonlinearity", brd);
    hADCNonLinInt=MakeTH1('D', obname.Data(), obtitle.Data(), 4095, -2048, 2047, "ADC value", "INL");


    obname.Form("Board%d/MsgTypes_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Message types", brd);
    hMsgTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);

    if (IsObjMade()) {
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_ADC_Direct, "ADC direct readout");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_ADC_Event, "Triggered readout");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_Unused, "unused");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_Wishbone, "Wishbone response");
    }

    obname.Form("Board%d/Wishbone/AckCode_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Wishbone ack codes", brd);
    hWishboneAck=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
    if (IsObjMade()) {
      hWishboneAck->GetXaxis()->SetBinLabel(1 + THitDetMsgWishbone::ACK_Acknowledge, "Acknowledged");
      hWishboneAck->GetXaxis()->SetBinLabel(1 + THitDetMsgWishbone::ACK_Unused, "unused");
      hWishboneAck->GetXaxis()->SetBinLabel(1 + THitDetMsgWishbone::ACK_Error, "Access Error");
      hWishboneAck->GetXaxis()->SetBinLabel(1 + THitDetMsgWishbone::ACK_Data, "Data follows");
        }

    obname.Form("Board%d/Wishbone/Source_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Wishbone source id", brd);
    hWishboneSource=MakeTH1('I', obname.Data(), obtitle.Data(), 16, 0, 16);

    obname.Form("Board%d/FFTWindow_%d", brd, brd);
    cWindowFFT=MakeWinCond(obname.Data(),1024, 2048,hTraceLongPrev->GetName());






    SetMakeWithAutosave(kTRUE);

    obname.Form("ADC_direct_Brd%d_Tr", brd);
  TGo4Picture* pic = GetPicture(obname.Data());
  if (pic == 0)
  {
    obtitle.Form("ADC direct trace Board%d", brd);
    pic = new TGo4Picture(obname.Data(), obtitle.Data());

    pic->SetDivision(2, 2);
    pic->Pic(0, 0)->AddObject(hTraceLongPrev);
    pic->Pic(0, 0)->SetLineAtt(3, 1, 1);    // solid line
    pic->Pic(0, 0)->AddObject(cWindowFFT);

    pic->Pic(0, 1)->AddObject(hTracePartFFT);
    pic->Pic(0, 1)->SetLogScale(1);
    pic->Pic(0, 1)->SetLineAtt(4, 1, 1);
    pic->Pic(0, 1)->SetFillAtt(4, 3001);

    pic->Pic(1, 0)->AddObject(hTraceLong);
    pic->Pic(1, 0)->SetLineAtt(3, 1, 1);
    pic->Pic(1, 0)->SetFillAtt(3, 3001);

    pic->Pic(1, 1)->AddObject(hTraceLongFFT);
    pic->Pic(1, 1)->SetLogScale(1);
    pic->Pic(1, 1)->SetLineAtt(4, 1, 1);
    pic->Pic(1, 1)->SetFillAtt(4, 3001);
    AddPicture(pic, Form("Board%d", brd));
  }
}


void THitDetBoardDisplay::ResetDisplay(Bool_t cleartracelong)
{
  for(Int_t ch=0; ch<HitDet_CHANNELS; ++ch)
      {
        hTrace[ch]->Reset();
        for(Int_t i=0; i<HitDet_MAXSNAPSHOTS; ++i)
        {
            if(hTraceSnapshots[ch][i]) hTraceSnapshots[ch][i]->Reset("");

       } //for i
        hTraceSnapshot2d[ch]->Reset("");
      }// for ch
  if(cleartracelong)
    hTraceLong->Reset(""); // TODO: probably we want to have long traces over more than one mbs event?

}


