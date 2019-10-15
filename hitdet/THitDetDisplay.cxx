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


      obname.Form("Board%d/Channel%d/DeltaTSMsg_%d_%d", brd, ch, brd, ch);
      obtitle.Form("Timestamp difference of subsequent messages Board %d Channel %d", brd, ch);
      Int_t maxval=0xFFF;
      hDeltaTSMsg[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), maxval+1, 0, maxval, "#delta TS (ticks)", "counts");

      obname.Form("Board%d/Channel%d/DeltaEPMsg_%d_%d", brd, ch, brd, ch);
      obtitle.Form("Epoch difference of subsequent messages Board %d Channel %d", brd, ch);
      maxval=0x3FFFFF;
      int binsize=maxval/1000;
      hDeltaEPMsg[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), binsize, 0, maxval, "#delta EP (epochs) - rebinned", "counts");


      obname.Form("Board%d/Channel%d/DeltaEPMsgFine_%d_%d", brd, ch, brd, ch);
      obtitle.Form("Epoch difference of subsequent messages Board %d Channel %d (fine)", brd, ch);
      maxval=0x3FFF;
      hDeltaEPMsgFine[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), maxval, 0, maxval, "#delta EP (epochs) - fine", "counts");



    }// for ch

    obname.Form("Board%d/TraceLong_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace (direct ADC only)", brd);
    Int_t totlength= 8 * HitDet_MAXTRACELONG; // JAM todo: different variable for stitched length here later
    hTraceLong=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");


    obname.Form("Board%d/TraceLongCorrected_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace corrected (direct ADC only)", brd);
    totlength= 8 * HitDet_MAXTRACELONG; // JAM todo: different variable for stitched length here later
    hTraceLongCorrected=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");


    obname.Form("Board%d/TraceLongFull_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Last complete Stitched Trace (direct ADC only)", brd);
    hTraceLongPrev=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongFullFit_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Fit curve for Last complete Stitched Trace (direct ADC only)", brd);
    hTraceLongPrevSinusfit =MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

//    TH1 *hTraceLongPrevOutsiders;
//
//      /** this histogram sums all data points modulo 8 of hTraceLongPrev that were outside the sinus fit curve hTraceLongPrevSinusfit
//       * by a certain deviation. The tolerance can be set by parameter*/
//      TH1 *hTraceLongPrevOutsidersModulo;

    obname.Form("Board%d/TraceLongFullOutsiders_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Deviation points from sinus fit of Stitched Trace (direct ADC only)", brd);
    hTraceLongPrevOutsiders =MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongFullOutsidersModulo_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Deviation points from sinus fit of Stitched Trace (direct ADC only)", brd);
    hTraceLongPrevOutsidersModulo =MakeTH1('I', obname.Data(), obtitle.Data(), 8, 0, 8, "time % 8 (bins)", "counts");

    obname.Form("Board%d/TraceLongFullCorrected_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Last complete Stitched Trace corrected (direct ADC only)", brd);
    hTraceLongPrevCorrected=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");



    obname.Form("Board%d/TraceLongSum_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace Sums(direct ADC only)", brd);
    hTraceLongSum=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongSumCorrected_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace Sums Corrected (direct ADC only)", brd);
    hTraceLongSumCorrected=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");



    obname.Form("Board%d/TraceLongFFT_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace FFT (direct ADC only)", brd);
    hTraceLongFFT=MakeTH1('D', obname.Data(), obtitle.Data(), totlength, 0, totlength, "1/t ", "counts");


    obname.Form("Board%d/TracePartFFT_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace FFT from window(direct ADC only)", brd);
    hTracePartFFT=MakeTH1('D', obname.Data(), obtitle.Data(), totlength, 0, totlength, "1/t ", "counts");

    obname.Form("Board%d/TracePartFFT_Corrected_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Corrected Trace FFT from window (direct ADC only)", brd);
    hTracePartCorrectedFFT=MakeTH1('D', obname.Data(), obtitle.Data(), totlength, 0, totlength, "1/t ", "counts");



    obname.Form("Board%d/ADC_Values_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Accumulated ADC values", brd);
    hADCValues=MakeTH1('I', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "ADC value", "counts");

    obname.Form("Board%d/ADC_DeltaValues_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Accumulated ADC mean deviation", brd);
    hADCDeltaMeanValues=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "ADC value", "#delta Mean");

    obname.Form("Board%d/ADC_DiffNL_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Differential nonlinearity", brd);
    hADCNonLinDiff=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "ADC value", "DNL");

    obname.Form("Board%d/ADC_IntNL_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Integral nonlinearity", brd);
    hADCNonLinInt=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "ADC value", "INL");

    obname.Form("Board%d/Calibration/ADC_Correction_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Correction vector", brd);
    hADCCorrection=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "ADC value", "Calibrated correction");

    TGo4Analysis::Instance()->ProtectObjects("Calibration","+C"); // protect calibration histograms against clear from GUI



    obname.Form("Board%d/SinusBaseline_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Sinus fit baseline parameters", brd);
    hSinusfitBaseline=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "Sinus Fit Baselines", "counts");

    obname.Form("Board%d/SinusAmplitude_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Sinus fit amplitude parameters", brd);
    hSinusfitAmplitude=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "Sinus Fit Amplitudes", "counts");

    obname.Form("Board%d/SinusPeriod_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Sinus fit period parameters", brd);
    hSinusfitPeriod=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, -2048, 2048, "period (samples)", "counts");

    obname.Form("Board%d/SinusPhase_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Sinus fit phase parameters", brd);
    hSinusfitPhase=MakeTH1('D', obname.Data(), obtitle.Data(), 20000, 0, 200, "phase shift (samples)", "counts");

    obname.Form("Board%d/SinusChi2_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Sinus fit #chi^2/NDF", brd);
    hSinusfitChi2=MakeTH1('D', obname.Data(), obtitle.Data(), 20000, 0, 200, "Sinus Fit #chi^2 / NDF");









    obname.Form("Board%d/MsgTypes_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Message types", brd);
    hMsgTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);

    if (IsObjMade()) {
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_ADC_Direct, "ADC direct readout");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_ADC_Event, "Triggered readout");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_Unused, "unused");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + THitDetMsg::MSG_Wishbone, "Wishbone response");
    }

    obname.Form("Board%d/Channels_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Channel distribution (Event readout)", brd);
    hChannels=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4, "Channel number");

    obname.Form("Board%d/Datasize_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Event data size (12 bit words)", brd);
    hDatawords=MakeTH1('I', obname.Data(), obtitle.Data(), 64, 0, 64, "Data size (12 bit words)");

    obname.Form("Board%d/MemoryRow_%d", brd, brd);
    obtitle.Form("HitDetection Board %d memory row statistics (Event readout)", brd);
    hMemoryRow=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4, "Memory row");

    obname.Form("Board%d/ChipId_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Chip ID number", brd);
    hChipId=MakeTH1('I', obname.Data(), obtitle.Data(), 256, 0, 256, "Chip ID");


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

    obname.Form("Board%d/SinusfitWindow_%d", brd, brd);
    cWindowSinusFit=MakeWinCond(obname.Data(),0, 200, hTraceLongPrev->GetName());


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

  obname.Form("ADC_direct_Brd%d_Corrected", brd);
 pic = GetPicture(obname.Data());
 if (pic == 0)
 {
   obtitle.Form("ADC direct corrected trace Board%d", brd);
   pic = new TGo4Picture(obname.Data(), obtitle.Data());

   pic->SetDivision(2, 1);
   pic->Pic(0, 0)->AddObject(hTraceLongPrevCorrected);
   pic->Pic(0, 0)->SetLineAtt(5, 1, 1);    // solid line
   pic->Pic(0, 0)->AddObject(cWindowFFT);

   pic->Pic(1, 0)->AddObject(hTracePartCorrectedFFT);
   pic->Pic(1, 0)->SetLogScale(1);
   pic->Pic(1, 0)->SetLineAtt(4, 1, 1);
   pic->Pic(1, 0)->SetFillAtt(4, 3001);
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



