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
  for(Int_t ch=0; ch<HitDet_CHANNELS; ++ch)
    for(Int_t i=0; i<HitDet_MAXSNAPSHOTS; ++i)
      hTraceSnapshots[ch][i]=0;

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

      for(Int_t i=0; i<numsnapshots; ++i)
      {
          if(i>=HitDet_MAXSNAPSHOTS) continue;
          obname.Form("Board%d/Channel%d/Trace_%d_%d_%d", brd, ch, brd, ch,i);
          obtitle.Form("HitDetection Board %d Channel %d Trace %d", brd, ch, i);
          hTraceSnapshots[ch][i] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

     } //for i
      obname.Form("Board%d/Channel%d/TraceSnapshots_%d_%d", brd, ch, brd, ch);
      obtitle.Form("HitDetection Board %d Channel %d Trace snapshot overview", brd, ch);
      hTraceSnapshot2d[ch]=MakeTH2('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, numsnapshots, 0, numsnapshots,"time (bins)", "message sequence", "counts");

    }// for ch
    obname.Form("Board%d/TraceLong_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace (direct ADC only)", brd);
    Int_t totlength= 8 * numsnapshots; // JAM todo: different variable for stitched length here later
    hTraceLong=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");
    obname.Form("Board%d/TraceLongSum_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace Sums(direct ADC only)", brd);
    hTraceLongSum=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "time (bins)", "counts");

    obname.Form("Board%d/TraceLongFFT_%d", brd, brd);
    obtitle.Form("HitDetection Board %d Stitched Trace FFT (direct ADC only)", brd);
    hTraceLongFFT=MakeTH1('I', obname.Data(), obtitle.Data(), totlength, 0, totlength, "1/t ", "counts");

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



