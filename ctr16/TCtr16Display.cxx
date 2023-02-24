#include "TCtr16Display.h"

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

#include "TCtr16RawEvent.h"
#include "TCtr16RawParam.h"

//////////////////////////////////////////////////////////////////////////////////////////////////



//***********************************************************

TCtr16BoardDisplay::TCtr16BoardDisplay(Int_t boardid) : TGo4EventProcessor(),
hADCValues(0), hADCDeltaMeanValues (0), hADCNonLinDiff(0), hADCNonLinInt(0), hADCCorrection(0), hADCCValuesCorrected(0),
hFrameTypes(0), hMsgTypes(0), hDataTypes(0), hChannels(0), hDatawords(0), hMemoryRow(0),
hChipId(0), hWishboneAck(0), hWishboneSource(0), lWishboneText(0),
fDisplayId(boardid)
{

  printf("TCtr16BoardDisplay ctor of id %d\n", fDisplayId);
  for(Int_t ch=0; ch<Ctr16_CHANNELS; ++ch)
    for(Int_t i=0; i<Ctr16_MAXSNAPSHOTS; ++i)
      hTraceSnapshots[ch][i]=0;

  SetMakeWithAutosave(kTRUE);
  TString obname;
  Int_t brd = fDisplayId;
  obname.Form("WishboneDump_%d", brd);
  lWishboneText = new TLatex(0.2,0.8,"-- wishbone dump --");
  lWishboneText->SetName(obname.Data());
  lWishboneText->SetNDC(); // relative pad x,y coordinates [0...1]
  AddObject(lWishboneText); // always replace previous label




  for(Int_t ch=0; ch<Ctr16_CHANNELS; ++ch)
      {
        hTrace[ch] = 0;
        hTraceSum[ch] = 0;
        for(Int_t i=0; i<Ctr16_MAXSNAPSHOTS; ++i)
        {
          hTraceSnapshots[ch][i]=0;
        }
        hTraceSnapshot2d[ch]=0;
      }



}

TCtr16BoardDisplay::~TCtr16BoardDisplay()
{

}


void TCtr16BoardDisplay::InitDisplay(Int_t tracelength, Int_t numsnapshots, Bool_t replace)
{


    if (replace)
      SetMakeWithAutosave(kFALSE);

    TGo4Log::Info("TCtr16BoardDisplay: Initializing histograms with tracelength %d snapshots %d for Board %d ",
        tracelength,numsnapshots,
        GetDevId());

    // Creation of histograms (or take them from autosave)
    TString obname;
    TString obtitle;
    TString foldername;
    TString binlabel;
    Int_t brd = fDisplayId;

    for(Int_t ch=0; ch<Ctr16_CHANNELS; ++ch)
    {
      obname.Form("Board%d/Traces/Last/Trace_%d_%d_Last", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Last Trace", brd, ch);
      hTrace[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

      obname.Form("Board%d/Traces/Sum/SumTrace_%d_%d", brd,  brd, ch);
      obtitle.Form("Accumulated Ctr16 Board %d Channel %d Trace", brd, ch);
      hTraceSum[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

      for(Int_t i=0; i<Ctr16_MAXSNAPSHOTS; ++i)
      {
          obname.Form("Board%d/Traces/Snapshots/Channel_%d/Trace_%d_%d_%d", brd, ch, brd, ch,i);
          obtitle.Form("Ctr16 Board %d Channel %d Trace %d", brd, ch, i);
          hTraceSnapshots[ch][i] = MakeTH1('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, "time (bins)", "counts");

     } //for i


      obname.Form("Board%d/Traces/Snapshots/Channel_%d/TTraceSnapshots_%d_%d", brd, ch, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Trace snapshot overview", brd, ch);
      hTraceSnapshot2d[ch]=MakeTH2('I', obname.Data(), obtitle.Data(), tracelength, 0, tracelength, numsnapshots, 0, numsnapshots -1,"time (bins)", "message sequence", "counts");




      obname.Form("Board%d/Features/Amplitudes/Amplitude_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Extracted amplitude", brd, ch);
      hFeatureAmplitude[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_FEATURE_AMPRAMGE/10, 0, Ctr16_FEATURE_AMPRAMGE,
                "amplitude (bins)", "counts");

      obname.Form("Board%d/Features/FineTimes/FineTime_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Extracted fine time", brd, ch);
      hFeatureFineTime[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_FEATURE_FINETIMERANGE, 0, Ctr16_FEATURE_FINETIMERANGE,
          "fine time (bins)", "counts");


      obname.Form("Board%d/Threshold/Baseline/ThreshBaseline_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Threshold scan DAC baseline", brd, ch);
      hThresholdBaseline[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_THRESHOLD_RANGE/10, 0,Ctr16_THRESHOLD_RANGE,
                    "DAC baseline (bins)", "counts");

      obname.Form("Board%d/Threshold/Mean/ThreshMean_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Threshold scan mean value", brd, ch);
      hThresholdMean[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_THRESHOLD_RANGE/10, 0,Ctr16_THRESHOLD_RANGE,
                          "scanned mean baseline (bins)", "counts");

      obname.Form("Board%d/Threshold/Noise/ThreshFWHM_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Threshold scan noise (FWHM)", brd, ch);
      hThresholdNoise[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_THRESHOLD_RANGE/10, 0,Ctr16_THRESHOLD_RANGE,
          "scanne noise width FWHM (bins)", "counts");

      obname.Form("Board%d/Threshold/Threshold/ThreshDAC_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Threshold DAC value", brd, ch);
      hThresholdSetting[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_THRESHOLD_RANGE/10, 0,Ctr16_THRESHOLD_RANGE,
                "threshold DAC set (bins)", "counts");

      obname.Form("Board%d/Threshold/Tracking/TrackingDAC_%d_%d", brd, brd, ch);
      obtitle.Form("Ctr16 Board %d Channel %d Tracking DAC value", brd, ch);
      hThresholdTracking[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_THRESHOLD_RANGE/10, 0,Ctr16_THRESHOLD_RANGE,
                           "tracking DAC set (bins)", "counts");


            //
            //                    Byte Bit 7  6  5  4  3  2  1  0
            //                           | 0  1  0  1  0  0 | Blk|    Header mit Block Nummer
            //                           |11    ..              4|    Mean value
            //                           | 3        0|11        8|    Mean value |  FWHM
            //                           | 7                    0|    FWHM
            //                           |11                    4|    Threshold DAC
            //                           | 3        0|11        8|    Threshold DAC  | Tracking DAC
            //                           | 7                    0|    Tracking DAC
            //                           |11                    4|    Baseline DAC Ch 0
            //                           | 3        0|11        8|    Baseline DAC Ch 0  | Baseline DAC Ch 1
            //                           | 7                    0|    Baseline DAC Ch 1
            //                           |11                    4|    Baseline DAC Ch 2
            //                           | 3        0|11        8|    Baseline DAC Ch 2  | Baseline DAC Ch 3
            //                           | 7                    0|    Baseline DAC Ch 3
            //



      obname.Form("Board%d/DeltaTime/Stamps/DeltaTSMsg_%d_%d", brd, brd, ch);
      obtitle.Form("Timestamp difference of subsequent messages Board %d Channel %d", brd, ch);
      Int_t maxval=0xFFF;
      hDeltaTSMsg[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), maxval+1, 0, maxval, "#delta TS (ticks)", "counts");

      obname.Form("Board%d/DeltaTime/Epochs/DeltaEPMsg_%d_%d", brd, brd, ch);
      obtitle.Form("Epoch difference of subsequent messages Board %d Channel %d", brd, ch);
      maxval=0x3FFFFF;
      int binsize=maxval/1000;
      hDeltaEPMsg[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), binsize, 0, maxval, "#delta EP (epochs) - rebinned", "counts");


      obname.Form("Board%d/DeltaTime/EpochsFine/DeltaEPMsgFine_%d_%d", brd,  brd, ch);
      obtitle.Form("Epoch difference of subsequent messages Board %d Channel %d (fine)", brd, ch);
      maxval=0x3FFF;
      hDeltaEPMsgFine[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), maxval, 0, maxval, "#delta EP - fine", "counts");



    }// for ch

    Int_t adcmin=0;
    Int_t adcmax=4096;
#ifdef    Ctr16_BOTHPOLARITY
    adcmin=-2048;
    adcmax=2048;
#endif
    obname.Form("Board%d/ADC/ADC_Values_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Accumulated ADC values", brd);

    hADCValues=MakeTH1('I', obname.Data(), obtitle.Data(), 4096, adcmin, adcmax, "ADC value", "counts");

    obname.Form("Board%d/ADC/ADC_Values_Corrected%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Accumulated Corrected ADC values", brd);
    hADCCValuesCorrected=MakeTH1('I', obname.Data(), obtitle.Data(), 4096, adcmin, adcmax, "ADC value", "counts");


    obname.Form("Board%d/ADC/ADC_DeltaValues_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Accumulated ADC mean deviation", brd);
    hADCDeltaMeanValues=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, adcmin, adcmax, "ADC value", "#delta Mean");

    obname.Form("Board%d/ADC/ADC_DiffNL_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Differential nonlinearity", brd);
    hADCNonLinDiff=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, adcmin, adcmax, "ADC value", "DNL");

    obname.Form("Board%d/ADC/ADC_IntNL_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Integral nonlinearity", brd);
    hADCNonLinInt=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, adcmin, adcmax, "ADC value", "INL");

    obname.Form("Board%d/Calibration/ADC_Correction_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Correction vector", brd);
    hADCCorrection=MakeTH1('D', obname.Data(), obtitle.Data(), 4096, adcmin, adcmax, "ADC value", "Calibrated correction");

    TGo4Analysis::Instance()->ProtectObjects("Calibration","+C"); // protect calibration histograms against clear from GUI




    obname.Form("Board%d/FrameTypes_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Frame types", brd);
    hFrameTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
    if (IsObjMade()) {
      hFrameTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Frame_Continuation, "Continuation");
      hFrameTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Frame_Wishbone, "Slow Control/Whishbone");
      hFrameTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Frame_Error, "Error");
      hFrameTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Frame_Data, "Data");
    }

    obname.Form("Board%d/MessageTypes_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Slow Control message types", brd);
    hMsgTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
    if (IsObjMade()) {
      hMsgTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Message_Init, "Init");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Message_Start, "Start");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Message_Threshold, "Threshold");
      hMsgTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Message_Wishbone, "Wishbone");
    }


    obname.Form("Board%d/DataTypes_%d", brd, brd);
       obtitle.Form("Ctr16 Board %d Event data types", brd);
       hDataTypes=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
       if (IsObjMade()) {
         hDataTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Data_None, "None");
         hDataTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Data_Unused, "Unused");
         hDataTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Data_Transient, "Transient");
         hDataTypes->GetXaxis()->SetBinLabel(1 + TCtr16Msg::Data_Feature, "Feature");
       }

    obname.Form("Board%d/Channels_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Channel distribution (Event readout)", brd);
    hChannels=MakeTH1('I', obname.Data(), obtitle.Data(), Ctr16_CHANNELS, 0, Ctr16_CHANNELS, "Channel number");

    obname.Form("Board%d/Datasize_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Event data size (12 bit words)", brd);
    hDatawords=MakeTH1('I', obname.Data(), obtitle.Data(), 64, 0, 64, "Data size (12 bit words)");

    obname.Form("Board%d/MemoryRow_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d memory row statistics (Event readout)", brd);
    hMemoryRow=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4, "Memory row");

    obname.Form("Board%d/ChipId_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Chip ID number", brd);
    hChipId=MakeTH1('I', obname.Data(), obtitle.Data(), 256, 0, 256, "Chip ID");


    obname.Form("Board%d/Wishbone/AckCode_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Wishbone ack codes", brd);
    hWishboneAck=MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
    if (IsObjMade()) {
      hWishboneAck->GetXaxis()->SetBinLabel(1 + TCtr16MsgWishbone::ACK_Acknowledge, "Acknowledged");
      hWishboneAck->GetXaxis()->SetBinLabel(1 + TCtr16MsgWishbone::ACK_SlowControl, "Slow Control");
      hWishboneAck->GetXaxis()->SetBinLabel(1 + TCtr16MsgWishbone::ACK_Error, "Access Error");
      hWishboneAck->GetXaxis()->SetBinLabel(1 + TCtr16MsgWishbone::ACK_Data, "Data follows");
        }

    obname.Form("Board%d/Wishbone/Source_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d Wishbone source id", brd);
    hWishboneSource=MakeTH1('I', obname.Data(), obtitle.Data(), 16, 0, 16);


    obname.Form("Board%d/ErrorCodes_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d  Error code number", brd);
    hErrorcodes=MakeTH1('I', obname.Data(), obtitle.Data(), 256, 0, 256);

    obname.Form("Board%d/ErrorTime_%d", brd, brd);
    obtitle.Form("Ctr16 Board %d  Error frame timstamps", brd);
    hErrorTimestamp=MakeTH1('I', obname.Data(), obtitle.Data(), 0xFFF, 0, 0xFFF);

    SetMakeWithAutosave(kTRUE);

}


void TCtr16BoardDisplay::ResetDisplay()
{
  for(Int_t ch=0; ch<Ctr16_CHANNELS; ++ch)
      {
        hTrace[ch]->Reset();
        for(Int_t i=0; i<Ctr16_MAXSNAPSHOTS; ++i)
        {
            if(hTraceSnapshots[ch][i]) hTraceSnapshots[ch][i]->Reset("");
       } //for i
        hTraceSnapshot2d[ch]->Reset("");
         fSnapshotcount[ch]=0;
      }// for ch

}



