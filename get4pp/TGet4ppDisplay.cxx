#include "TGet4ppDisplay.h"

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

#include "TGet4ppRawEvent.h"
#include "TGet4ppRawParam.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

//***********************************************************

TGet4ppBoardDisplay::TGet4ppBoardDisplay(Int_t boardid) :
		hChipId(0), hMsgTypes(0), hChannels(0), hSyncFlags(0), hWishboneAck(0),
		hWishboneSource(0), lWishboneText(0), fDisplayId(boardid)
{

	printf("TGet4ppBoardDisplay ctor of id %d\n", fDisplayId);
	for (Int_t ch = 0; ch < Get4pp_CHANNELS; ++ch)
	{
		hEdges[ch] = 0;
		hLostEdges[ch] = 0;
		hEventTypes[ch] = 0;
		fLastLeadingEdgeTime[ch] = 0.0;
		fHasLeadingEdge[ch] = kFALSE;
		hToTinSeconds[ch] = 0;
		hErrorCodes[ch] = 0;
		hErrorDLLPhase[ch] = 0;

		for (Int_t l = 0; l < 2; ++l)
		{
			hCoarseTime[ch][l] = 0;
			hFineTime[ch][l] = 0;
			hEpochs[ch][l] = 0;
			hFullTime[ch][l] = 0;
			hTimeInSeconds[ch][l] = 0;

		}
		for (Int_t cwork = 0; cwork < Get4pp_CHANNELS; ++cwork)
		  {
		      hDeltaTime[ch][cwork]=0;
		      hDeltaTimeInSeconds[ch][cwork]=0;
		  }

	}

	SetMakeWithAutosave(kTRUE);
	TString obname;
	Int_t brd = fDisplayId;
	obname.Form("WishboneDump_%d", brd);
	lWishboneText = new TLatex(0.2, 0.8, "-- wishbone dump --");
	lWishboneText->SetName(obname.Data());
	lWishboneText->SetNDC(); // relative pad x,y coordinates [0...1]
	AddObject(lWishboneText); // always replace previous label

}

TGet4ppBoardDisplay::~TGet4ppBoardDisplay()
{

}

void TGet4ppBoardDisplay::InitDisplay(Bool_t replace)
{

	if (replace)
		SetMakeWithAutosave(kFALSE);

	TGo4Log::Info(
			"TGet4ppBoardDisplay: Initializing histograms for Board %d, replace = %s",
			GetDevId(), (replace ? "true" : "false"));

	// Creation of histograms (or take them from autosave)
	TString obname;
	TString obtitle;
	TString foldername;
	TString binlabel;
	Int_t brd = fDisplayId;

	obname.Form("Board%d/MsgTypes_%d", brd, brd);
	obtitle.Form("Get4pp Board %d Message types", brd);
	hMsgTypes = MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);

	if (IsObjMade())
	{
		hMsgTypes->GetXaxis()->SetBinLabel(1 + TGet4ppMsg::MSG_TDC_Data,
				"TDC data");
		hMsgTypes->GetXaxis()->SetBinLabel(1 + TGet4ppMsg::MSG_Unused_1,
				"Unused 1");
		hMsgTypes->GetXaxis()->SetBinLabel(1 + TGet4ppMsg::MSG_Unused_2,
				"Unused 2");
		hMsgTypes->GetXaxis()->SetBinLabel(1 + TGet4ppMsg::MSG_Wishbone,
				"Wishbone response");
	}

	obname.Form("Board%d/Channels_%d", brd, brd);
	obtitle.Form("Get4pp Board %d Channel distribution (Event readout)", brd);
	hChannels = MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4,
			"Channel number");



	obname.Form("Board%d/SyncFlags_%d", brd, brd);
	obtitle.Form("Get4pp Board %d Sync Flag scaler", brd);
	hSyncFlags = MakeTH1('I', obname.Data(), obtitle.Data(), 2, 0, 2);

		if (IsObjMade())
		{
			hSyncFlags->GetXaxis()->SetBinLabel(1 + 0,
					"Sync");
			hSyncFlags->GetXaxis()->SetBinLabel(1 + 1,
					"EpSy");
		}

	obname.Form("Board%d/ChipId_%d", brd, brd);
	obtitle.Form("Get4++ Board %d Chip ID number", brd);
	hChipId = MakeTH1('I', obname.Data(), obtitle.Data(), 256, 0, 256,
			"Chip ID");

	for (Int_t ch = 0; ch < Get4pp_CHANNELS; ++ch)
	{

		obname.Form("Board%d/Channel%d/Edges_%d_%d", brd, ch, brd, ch);
		obtitle.Form("Get4pp Board %d Channel %d Leading/Trailing edges", brd,
				ch);
		hEdges[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), 2, 0, 2);
		if (IsObjMade())
		{
			hEdges[ch]->GetXaxis()->SetBinLabel(1, "Leading");
			hEdges[ch]->GetXaxis()->SetBinLabel(1 + 1, "Trailing");
		};

		obname.Form("Board%d/Channel%d/LostEdges_%d_%d", brd, ch, brd, ch);
		obtitle.Form(
				"Get4pp Board %d Channel %d Lost Leading/Trailing edges and Epochcounter scaler",
				brd, ch);
		hLostEdges[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), 3, 0, 3);
		if (IsObjMade())
		{
			hLostEdges[ch]->GetXaxis()->SetBinLabel(1, "Leading");
			hLostEdges[ch]->GetXaxis()->SetBinLabel(1 + 1, "Trailing");
			hLostEdges[ch]->GetXaxis()->SetBinLabel(1 + 2, "Epoch wraps");
		};

		obname.Form("Board%d/Channel%d/EventTypes_%d_%d", brd, ch, brd, ch);
		obtitle.Form("Get4pp Board %d Channel %d Event type", brd, ch);
		hEventTypes[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
		if (IsObjMade())
		{
			hEventTypes[ch]->GetXaxis()->SetBinLabel(1 + 3, "TDC data");
			hEventTypes[ch]->GetXaxis()->SetBinLabel(1 + 2, "Errors");
			hEventTypes[ch]->GetXaxis()->SetBinLabel(1 + 0, "Unknown 1");
			hEventTypes[ch]->GetXaxis()->SetBinLabel(1 + 1, "Unknown 2");
		};

		for (Int_t l = 0; l < 2; ++l)
		{
			obname.Form("Board%d/Channel%d/Epochs_%s_%d_%d", brd, ch,
					(l == 0 ? "leading" : "trailing"), brd, ch);
			obtitle.Form("Get4pp Board %d Channel %d Epoch counter %s edge",
					brd, ch, (l == 0 ? "leading" : "trailing"));
			hEpochs[ch][l] = MakeTH1('I', obname.Data(), obtitle.Data(),
					(Get4pp_EPOCHRANGE + 1) / 1000, 0, Get4pp_EPOCHRANGE + 1,
					"epoch (units)", "counts");

			obname.Form("Board%d/Channel%d/Time_coarse_%s_%d_%d", brd, ch,
					(l == 0 ? "leading" : "trailing"), brd, ch);
			obtitle.Form(
					"Get4pp Board %d Channel %d Coarse time counter %s edge",
					brd, ch, (l == 0 ? "leading" : "trailing"));
			hCoarseTime[ch][l] = MakeTH1('I', obname.Data(), obtitle.Data(),
					(Get4pp_COARSERANGE + 1), 0, (Get4pp_COARSERANGE + 1),
					"coarse time (units)", "counts");

			obname.Form("Board%d/Channel%d/Time_fine_%s_%d_%d", brd, ch,
					(l == 0 ? "leading" : "trailing"), brd, ch);
			obtitle.Form("Get4pp Board %d Channel %d Fine time counter %s edge",
					brd, ch, (l == 0 ? "leading" : "trailing"));
			hFineTime[ch][l] = MakeTH1('I', obname.Data(), obtitle.Data(),
					(Get4pp_FINERANGE + 1), 0, (Get4pp_FINERANGE + 1),
					"fine time (units)", "counts");

			Double_t fullrange_raw = ((Double_t) (Get4pp_FINERANGE + 1))
					* ((Double_t) (Get4pp_COARSERANGE + 1))
					* ((Double_t) (Get4pp_EPOCHRANGE + 1)); // 0x80000000000 = 8.8e12
			Double_t fullbins_raw = 1.0e+6;
			obname.Form("Board%d/Channel%d/Time_full_%s_%d_%d", brd, ch,
					(l == 0 ? "leading" : "trailing"), brd, ch);
			obtitle.Form("Get4pp Board %d Channel %d Full time counter %s edge",
					brd, ch, (l == 0 ? "leading" : "trailing"));
			hFullTime[ch][l] = MakeTH1('I', obname.Data(), obtitle.Data(),
					fullbins_raw, 0, fullrange_raw, "fine time (units)",
					"counts");

			Double_t fullrange_seconds = Get4pp_COARSETIMEUNIT
					* ((Double_t) (Get4pp_COARSERANGE + 1))
					* ((Double_t) (Get4pp_EPOCHRANGE + 1)); // 137.4 s
			Double_t fullbins_seconds = 1.0e+6;
			obname.Form("Board%d/Channel%d/Time_seconds_%s_%d_%d", brd, ch,
					(l == 0 ? "leading" : "trailing"), brd, ch);
			obtitle.Form(
					"Get4pp Board %d Channel %d Full time (seconds) %s edge",
					brd, ch, (l == 0 ? "leading" : "trailing"));
			hTimeInSeconds[ch][l] = MakeTH1('I', obname.Data(), obtitle.Data(),
					fullbins_seconds, 0, fullrange_seconds, "seconds)",
					"counts");

		}

		Double_t fullbins_tot = 1.0e6;
		;
		Double_t fullrange_tot = Get4pp_COARSETIMEUNIT
				/ ((Get4pp_FINERANGE + 1)) * fullbins_tot * 100; // 1500.6 us

		obname.Form("Board%d/Channel%d/ToT_%d_%d", brd, ch, brd, ch);
		obtitle.Form("Get4pp Board %d Channel %d Time over Threshold (seconds)",
				brd, ch);
		hToTinSeconds[ch] = MakeTH1('I', obname.Data(), obtitle.Data(),
				fullbins_tot, 0, fullrange_tot, "ToT (seconds)", "counts");

		////////////////////////////////

		obname.Form("Board%d/Channel%d/Errors/ErrorCodes_%d_%d", brd, ch, brd,
				ch);
		obtitle.Form("Get4pp Board %d Channel %d Error message codes", brd, ch);
		hErrorCodes[ch] = MakeTH1('I', obname.Data(), obtitle.Data(), 14, 0,
				14);
		if (IsObjMade())
		{
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Init_Readout, "Init");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Sync, "Sync");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Epochcounter_Sync,
					"EpochSync");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Epoch, "Epoch");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_FIFO_Write, "FIFOwr");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Lost_Event, "LostEvent");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Channel_State, "Channel");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Tokenring_State, "Tokenring");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Token, "Token");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_Readout, "Readout");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_SPI, "SPI");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_DLL_Lock, "DLL_Lock");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_DLL_Reset, "DLL_Reset");
			hErrorCodes[ch]->GetXaxis()->SetBinLabel(
					1 + TGet4ppMsgErrorEvent::ERR_DLL_Reset + 1, "Unknown");
		};

		/** DLL phase value of error events, per channel. Errors without specific channels are put into channel 0*/
		obname.Form("Board%d/Channel%d/Errors/DLL_Phase_%d_%d", brd, ch, brd,
				ch);
		obtitle.Form("Get4pp Board %d Channel %d Error message DLL phase", brd,
				ch);
		hErrorDLLPhase[ch] = MakeTH1('I', obname.Data(), obtitle.Data(),
				Get4pp_DLLRANGE + 1, 0, Get4pp_DLLRANGE + 1);

	} // for ch

	obname.Form("Board%d/Wishbone/AckCode_%d", brd, brd);
	obtitle.Form("Get4 Board %d Wishbone ack codes", brd);
	hWishboneAck = MakeTH1('I', obname.Data(), obtitle.Data(), 4, 0, 4);
	if (IsObjMade())
	{
		hWishboneAck->GetXaxis()->SetBinLabel(
				1 + TGet4ppMsgWishbone::ACK_Acknowledge, "Acknowledged");
		hWishboneAck->GetXaxis()->SetBinLabel(
				1 + TGet4ppMsgWishbone::ACK_Unused, "unused");
		hWishboneAck->GetXaxis()->SetBinLabel(1 + TGet4ppMsgWishbone::ACK_Error,
				"Access Error");
		hWishboneAck->GetXaxis()->SetBinLabel(1 + TGet4ppMsgWishbone::ACK_Data,
				"Data follows");
	}

	obname.Form("Board%d/Wishbone/Source_%d", brd, brd);
	obtitle.Form("Get4ppection Board %d Wishbone source id", brd);
	hWishboneSource = MakeTH1('I', obname.Data(), obtitle.Data(), 16, 0, 16);



	// below new histograms for delta t between channels JAM 20-oct-2022
  for (Int_t cref = 0; cref < Get4pp_CHANNELS; ++cref)
  {

    for (Int_t cwork = cref + 1; cwork < Get4pp_CHANNELS; ++cwork)
    {
      // standard:
      Double_t deltarange_raw = ((Double_t)(Get4pp_FINERANGE + 1)) * 10;    //  10  coarseranges= 20 ns
      Double_t deltabins_raw = ((Double_t)(Get4pp_FINERANGE + 1)) * 10;    // fine time bin granularity here

      obname.Form("Board%d/DeltaTime/raw/dTime_raw_%d_%d-%d", brd, brd, cwork, cref);
      obtitle.Form("Get4pp Board %d Leading edge #delta t raw (%d-%d)", brd, cwork, cref);
      hDeltaTime[cref][cwork] = MakeTH1('I', obname.Data(), obtitle.Data(), deltabins_raw, -deltarange_raw / 2,
          deltarange_raw / 2, "fine time units", "counts");


//
 // TESTJAM
//      Double_t deltarange_secs = Get4pp_COARSETIMEUNIT * 50000;    //  +/- 50 us
//      Double_t deltabins_secs = ((Double_t)(Get4pp_FINERANGE + 1)) * 10000 ; // 10 ns bins
// end testjam

      Double_t deltarange_secs = Get4pp_COARSETIMEUNIT * 10;    //  10  coarseranges= 20 ns
      Double_t deltabins_secs = ((Double_t)(Get4pp_FINERANGE + 1)) * 10 * 2;    // fine time bin granularity here
      obname.Form("Board%d/DeltaTime/seconds/dTime_s_%d_%d-%d", brd, brd, cwork, cref);
      obtitle.Form("Get4pp Board %d Leading edge #delta t seconds (%d-%d)", brd, cwork, cref);
      hDeltaTimeInSeconds[cref][cwork] = MakeTH1('I', obname.Data(), obtitle.Data(), deltabins_secs,
          -deltarange_secs / 2, deltarange_secs / 2, "seconds", "counts");

    }    // cwork
  }    // cref

	SetMakeWithAutosave(kTRUE);



}

void TGet4ppBoardDisplay::ResetDisplay()
{


}

