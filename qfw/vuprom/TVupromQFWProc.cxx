#include "TVupromQFWProc.h"

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TROOT.h"
#include "TMath.h"
#include "TGo4Analysis.h"

#include "TGo4Picture.h"
#include "TGo4MbsEvent.h"
#include "TGo4UserException.h"

#include "TVupromQFWEvent.h"
#include "TVupromQFWParam.h"

//***********************************************************
TVupromQFWProc::TVupromQFWProc() :
	TGo4EventProcessor()
{
	cout << "**** TVupromQFWProc: Create instance " << endl;
}
//***********************************************************
TVupromQFWProc::~TVupromQFWProc()
{
	cout << "**** TVupromQFWProc: Delete instance " << endl;
}
//***********************************************************
// this one is used in standard factory
TVupromQFWProc::TVupromQFWProc(const char* name) :
	TGo4EventProcessor(name)
{
	cout << "**** TVupromQFWProc: Create instance " << name << endl;
	SetMakeWithAutosave(kTRUE);
	//// init user analysis objects:

	fPar= dynamic_cast<TVupromQFWParam*>(MakeParameter("QFWSetup", "TVupromQFWParam"));
	fPar->PrintParameter();
	MapGrids();
	InitDisplay(VUPROM_QFWSLICES);
}



void TVupromQFWProc::InitDisplay(UInt_t timeslices, Bool_t replace)
{
	cout << "**** TVupromQFWProc: Init Display for "<<timeslices <<" time slices. "<< endl;
	if(replace) //TGo4Analysis::Instance()->
			SetMakeWithAutosave(kFALSE);
	// Creation of histograms (or take them from autosave)
	TString obname;
	TString obtitle;
	TString foldername;
	TString binlabel;

	hVupromSetup = MakeTH1('I', "Vuprom/SetupIds", "QFW setup id numbers", 256,
			0, 256);
	hVupromPreMtime = MakeTH1('I', "Vuprom/PreMtime", "QFW prescaler m time",
			0x10000, 0, 0x10000000);
	hVupromMtime = MakeTH1('I', "Vuprom/Mtime", "QFW scaler m time", 0x10000,
			0, 0x10000000);
	;
	for (int brd = 0; brd < VUPROM_QFWBOARDS; ++brd)
		{
		for (int qfw = 0; qfw < VUPROM_QFWNUM; ++qfw)
		{
			obname.Form("Vuprom/Board%2d/QFW_Sum%2d-%2d", brd,brd,qfw);
			obtitle.Form("QFW Board %2d Scaler %2d Accumulated", brd, qfw);
			int qbins = (1 + timeslices) * VUPROM_QFWCHANS ;
			hVupromQFW[brd][qfw] = MakeTH1('I', obname.Data(), obtitle.Data(), qbins, 0,
					qbins, "QFW scaler sums");
			if (IsObjMade())
			{
				for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
				{
					// prescaler
					binlabel.Form("PRE%d", ch);
					hVupromQFW[brd][qfw]->GetXaxis()->SetBinLabel(1 + ch,
							binlabel.Data());
					// time slices:
					for (unsigned int sl = 0; sl < timeslices; ++sl)
								{
									binlabel.Form("SCL%d-%d", sl,ch);
									hVupromQFW[brd][qfw]->GetXaxis()->SetBinLabel(1 + (1+sl)*VUPROM_QFWCHANS
											+ch, binlabel.Data());
								}
					binlabel.Form("ERR%d", ch);
					hVupromQFW[brd][qfw]->GetXaxis()->SetBinLabel(1 + (timeslices-1)
							* VUPROM_QFWCHANS + ch, binlabel.Data());
				}
			}

			obname.Form("Vuprom/Board%2d/QFW_Trace%2d-%2d", brd,brd,qfw);
			obtitle.Form("QFW Board %2d Scaler %2d", brd, qfw);
			hVupromQFWTrace[brd][qfw] = MakeTH1('I', obname.Data(), obtitle.Data(),
					qbins, 0, qbins, "QFW scalers");
			if (IsObjMade())
			{
				for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
				{
					// prescaler
					binlabel.Form("PRE%d", ch);
					hVupromQFWTrace[brd][qfw]->GetXaxis()->SetBinLabel(1 + ch,
							binlabel.Data());
					// time slices:
					for (unsigned int sl = 0; sl < timeslices; ++sl)
					{
						binlabel.Form("SCL%d-%d", sl, ch);
						hVupromQFWTrace[brd][qfw]->GetXaxis()->SetBinLabel(1 + (1 + sl)
								* VUPROM_QFWCHANS + ch, binlabel.Data());
					}
					binlabel.Form("ERR%d", ch);
					hVupromQFWTrace[brd][qfw]->GetXaxis()->SetBinLabel(1
							+ (timeslices - 1) * VUPROM_QFWCHANS + ch,
							binlabel.Data());
				}
			}
		} // qfw


	obname.Form("QFW_Rawscalers_Board%2d",brd);
	pVupromQfws[brd] = GetPicture(obname.Data());
	if (pVupromQfws[brd] == 0)
	{
		obtitle.Form("Overview all QFW Accumulated Board%2d",brd);
		foldername.Form("Vuprom/Board%2d",brd);
		pVupromQfws[brd] = new TGo4Picture(obname.Data(), obtitle.Data());
		//	int xdiv=0;
		//	int ydiv= (int) TMath::Sqrt(VUPROM_QFWNUM+1);
		//	double yrest= VUPROM_QFWNUM % ydiv;
		//	if(yrest==0)
		//	  {
		//		xdiv=ydiv;
		//	  }
		//	else
		//	  {
		//		xdiv=ydiv+1;
		//	  }

		int xdiv = 4;
		int ydiv = 2;
		pVupromQfws[brd]->SetDivision(ydiv, xdiv);
		int x = 0, y = 0;
		for (int sc = 0; sc < VUPROM_QFWNUM; ++sc)
		{
			x = (sc % xdiv);
			y = (sc - x) / xdiv;
			pVupromQfws[brd]->Pic(y, x)->AddObject(hVupromQFW[brd][sc]);
			pVupromQfws[brd]->Pic(y, x)->SetFillAtt(5, 3001);
		}
		AddPicture(pVupromQfws[brd], foldername.Data());
	}

	obname.Form("QFW_Rawscalers_Trace_Board%2d",brd);
	pVupromQfwsTrace[brd] = GetPicture(obname.Data());
	if (pVupromQfwsTrace[brd] == 0)
	{
		obtitle.Form("Overview all QFW Scalers Board%2d",brd);
		foldername.Form("Vuprom/Board%2d",brd);
		pVupromQfwsTrace[brd] = new TGo4Picture(obname.Data(), obtitle.Data());
		//	int xdiv=0;
		//	int ydiv= (int) TMath::Sqrt(VUPROM_QFWNUM+1);
		//	double yrest= VUPROM_QFWNUM % ydiv;
		//	if(yrest==0)
		//	  {
		//		xdiv=ydiv;
		//	  }
		//	else
		//	  {
		//		xdiv=ydiv+1;
		//	  }

		int xdiv = 4;
		int ydiv = 2;
		pVupromQfwsTrace[brd]->SetDivision(ydiv, xdiv);
		int x = 0, y = 0;
		for (int sc = 0; sc < VUPROM_QFWNUM; ++sc)
		{
			x = (sc % xdiv);
			y = (sc - x) / xdiv;
			pVupromQfwsTrace[brd]->Pic(y, x)->AddObject(hVupromQFWTrace[brd][sc]);
			pVupromQfwsTrace[brd]->Pic(y, x)->SetFillAtt(5, 3001);
		}
		AddPicture(pVupromQfwsTrace[brd], foldername.Data());
	}

		} // board
	////////////////////////////////////_________________________________


	/* xy beam display*/
	obname.Form("Beam/GridAccu");
	obtitle.Form("Beam grid current accumulate");

	for (int grid = 0; grid < VUPROM_QFWGRIDS; ++grid)
			{

	hBeamX[grid] = MakeTH1('I', Form("Beam/Grid%2d/Profile_X_G%d",grid,grid), Form("X Profile Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid],
			"Wire");
	hBeamY[grid] = MakeTH1('I', Form("Beam/Grid%2d/Profile_Y_G%d",grid,grid), Form("Y Profile Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid],
			"Wire");
	hBeamXSlice[grid] = MakeTH2('I', Form("Beam/Grid%2d/Profile_X_Time_G%d",grid,grid), Form("X Profile vs Time slices Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], timeslices,0, timeslices,
				"Wire","Time Slice");
	hBeamYSlice[grid] = MakeTH2('I',  Form("Beam/Grid%2d/Profile_Y_Time_G%d",grid,grid), Form("Y Profile vs Time slices Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], timeslices,0, timeslices,
			"Wire","Time Slice");

	hBeamXSliceOffs[grid] = MakeTH2('I', Form("Beam/Grid%2d/Profile_X_Time_Offset_G%d",grid,grid), Form("X Profile vs Time slices average offset Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], timeslices,0, timeslices,
					"Wire","Time Slice");
	hBeamYSliceOffs[grid] = MakeTH2('I', Form("Beam/Grid%2d/Profile_Y_Time_Offset_G%d",grid,grid), Form("Y Profile vs Time slices average offset Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], timeslices,0, timeslices,
				"Wire","Time Slice");


	hBeamPreX[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfilePre_X_G%d",grid,grid), Form("X Profile Pre Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid],
			"Wire");
	hBeamPreY[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfilePre_Y_G%d",grid,grid), Form("Y Profile Pre Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresX[grid],
			"Wire");

	hBeamAccX[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfileSum_X_G%d",grid,grid), Form("X Profile accumulated Grid%2d",grid), gBeamWiresX[grid],
			0, gBeamWiresX[grid], "Wire");
	hBeamAccY[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfileSum_Y_G%d",grid,grid), Form("Y Profile accumulated Grid%2d",grid),
			gBeamWiresY[grid], 0, gBeamWiresY[grid], "Wire");

	hBeamAccXSlice[grid] = MakeTH2('I', Form("Beam/Grid%2d/ProfileSum_X_Time_G%d",grid,grid), Form("X Profile vs Time slices accum Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], timeslices,0, timeslices,
					"Wire","Time Slice");
	hBeamAccYSlice[grid] = MakeTH2('I', Form("Beam/Grid%2d/ProfileSum_Y_Time_G%d",grid,grid), Form("Y Profile vs Time slices accum Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], timeslices,0, timeslices,
				"Wire","Time Slice");

	hBeamPreAccX[grid] = MakeTH1('I', Form("Beam/Grid%2d/ProfilePreSum_X_G%d",grid,grid),
			Form("X Profile Pre accumulated Grid%2d",grid), gBeamWiresX[grid], 0, gBeamWiresX[grid], "Wire");
	hBeamPreAccY[grid] = MakeTH1('I',  Form("Beam/Grid%2d/ProfilePreSum_Y_G%d",grid,grid),
			Form("Y Profile Pre accumulated Grid%2d",grid), gBeamWiresY[grid], 0, gBeamWiresY[grid], "Wire");

	obname.Form("Beam Display Grid%2d",grid);
	pBeamProfiles[grid] = GetPicture(obname.Data());
	if (pBeamProfiles[grid] == 0)
	{
		obtitle.Form("Overview beam projections Grid%2d",grid);
		foldername.Form("Beam/Grid%2d",grid);
		pBeamProfiles[grid] = new TGo4Picture(obname.Data(), obtitle.Data());
		pBeamProfiles[grid]->SetDivision(4, 2);
		pBeamProfiles[grid]->Pic(0, 0)->AddObject(hBeamX[grid]);
		pBeamProfiles[grid]->Pic(0, 0)->SetFillAtt(3, 3001);
		pBeamProfiles[grid]->Pic(0, 1)->AddObject(hBeamY[grid]);
		pBeamProfiles[grid]->Pic(0, 1)->SetFillAtt(4, 3001);
		pBeamProfiles[grid]->Pic(1, 0)->AddObject(hBeamAccX[grid]);
		pBeamProfiles[grid]->Pic(1, 0)->SetFillAtt(3, 3001);
		pBeamProfiles[grid]->Pic(1, 1)->AddObject(hBeamAccY[grid]);
		pBeamProfiles[grid]->Pic(1, 1)->SetFillAtt(4, 3001);
		pBeamProfiles[grid]->Pic(2, 0)->AddObject(hBeamPreX[grid]);
		pBeamProfiles[grid]->Pic(2, 0)->SetFillAtt(3, 3001);
		pBeamProfiles[grid]->Pic(2, 1)->AddObject(hBeamPreY[grid]);
		pBeamProfiles[grid]->Pic(2, 1)->SetFillAtt(4, 3001);
		pBeamProfiles[grid]->Pic(3, 0)->AddObject(hBeamPreAccX[grid]);
		pBeamProfiles[grid]->Pic(3, 0)->SetFillAtt(3, 3001);
		pBeamProfiles[grid]->Pic(3, 1)->AddObject(hBeamPreAccY[grid]);
		pBeamProfiles[grid]->Pic(3, 1)->SetFillAtt(4, 3001);
		AddPicture(pBeamProfiles[grid], foldername.Data());

	}

	hBeamMeanXY[grid]=MakeTH2('I', Form("Beam/Grid%2d/Meanpos_G%d",grid,grid), Form("Beam Mean position scaler Grid%2d",grid), gBeamWiresX[grid], 0,gBeamWiresX[grid],gBeamWiresY[grid], 0,gBeamWiresY[grid],"X","Y");

	hBeamPreMeanXY[grid]=MakeTH2('I', Form("Beam/Grid%2d/MeanposPre_G%d",grid,grid), Form("Beam Mean position prescaler Grid%2d",grid), gBeamWiresX[grid], 0,gBeamWiresX[grid],gBeamWiresY[grid], 0,gBeamWiresY[grid],"X","Y");

	 hBeamRMSX[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_X_G%d",grid,grid), Form("X Profile RMS Grid%2d",grid),
				10*gBeamWiresX[grid], 0, gBeamWiresX[grid], "Wire spacings");

	 hBeamRMSY[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_Y_G%d",grid,grid), Form("Y Profile RMS Grid%2d",grid),
				10*gBeamWiresX[grid], 0, gBeamWiresY[grid], "Wire spacings");

	 hBeamPreRMSX[grid]=MakeTH1('I', Form("Beam/Grid%2d/Pre_RMS_X_G%d",grid,grid), Form("X Profile RMS Prescaler Grid%2d",grid),
				10*gBeamWiresX[grid], 0, gBeamWiresX[grid], "Wire spacings");

	 hBeamPreRMSY[grid]=MakeTH1('I', Form("Beam/Grid%2d/Pre_RMS_Y_G%d",grid,grid), Form("Y Profile RMS Prescaler Grid%2d",grid),
				10*gBeamWiresY[grid], 0, gBeamWiresY[grid], "Wire spacings");

	 obname.Form("Beam RMS Grid%2d",grid);
	 pBeamRMS[grid]= GetPicture(obname.Data());
	 	if (pBeamRMS == 0)
	 	{
	 		obtitle.Form("Beam RMS distribution Grid%2d",grid);
	 		foldername.Form("Beam/Grid%2d",grid);
	 		pBeamRMS[grid] = new TGo4Picture(obname.Data(), obtitle.Data());
	 		pBeamRMS[grid]->SetDivision(2, 2);
	 		pBeamRMS[grid]->Pic(0, 0)->AddObject(hBeamRMSX[grid]);
	 		pBeamRMS[grid]->Pic(0, 0)->SetFillAtt(3, 3002);
	 		pBeamRMS[grid]->Pic(0, 1)->AddObject(hBeamRMSY[grid]);
	 		pBeamRMS[grid]->Pic(0, 1)->SetFillAtt(4, 3002);
	 		pBeamRMS[grid]->Pic(1, 0)->AddObject(hBeamPreRMSX[grid]);
	 		pBeamRMS[grid]->Pic(1, 0)->SetFillAtt(3, 3002);
	 		pBeamRMS[grid]->Pic(1, 1)->AddObject(hBeamPreRMSY[grid]);
	 		pBeamRMS[grid]->Pic(1, 1)->SetFillAtt(4, 3002);
	 		AddPicture(pBeamRMS[grid], foldername.Data());

	 	}


	/* mean count of beam profile part: */

       cBeamXSliceCond[grid] = MakeWinCond(Form("Beam/Grid%2d/XSliceCond_G%d",grid,grid), 0,gBeamWiresX[grid], 0, timeslices, hBeamXSlice[grid]->GetName());
       cBeamYSliceCond[grid] = MakeWinCond(Form("Beam/Grid%2d/YSliceCond_G%d",grid,grid), 0,gBeamWiresY[grid], 0, timeslices, hBeamYSlice[grid]->GetName());


	hBeamMeanCountsX[grid]=MakeTH1('I', Form("Beam/Grid%2d/Mean_Counts_X_G%d",grid,grid), Form("X Counts Mean Grid%2d",grid),
				1000, 0, 100, "Mean counts");

	hBeamMeanCountsY[grid]=MakeTH1('I', Form("Beam/Grid%2d/Mean_Counts_Y_G%d",grid,grid), Form("Y Counts Mean Grid%2d",grid),
				1000, 0, 100, "Mean counts");

	hBeamRMSCountsX[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_Counts_X_G%d",grid,grid), Form("X Counts RMS Grid%2d",grid),
				1000, 0, 100, "RMS counts");

	hBeamRMSCountsY[grid]=MakeTH1('I', Form("Beam/Grid%2d/RMS_Counts_Y_G%d",grid,grid), Form("Y Counts RMS Grid%2d",grid),
				1000, 0, 100, "RMS counts");

		} // grid

	/* vulom scaler part: */
	hVulomDeadTime = MakeTH1('I', "Vulom/DeadTime", "VulomScaler Deadtime",
			DEADTIME_BINS, 0, DEADTIME_BINS);
	hVulomFC = MakeTH1('I', "Vulom/ScalerFC", "VulomScaler FC", FC_BINS, 0,
			FC_BINS);

	for (int sc = 0; sc < VULOM_SCALERNUM; ++sc)
	{
		obname.Form("Vulom/VulomScaler%2d", sc);
		obtitle.Form("VulomScaler %2d", sc);
		hVulomScaler[sc] = MakeTH1('I', obname.Data(), obtitle.Data(),
				VULOM_SCALERCHANS, 0, VULOM_SCALERCHANS, "channel number");
	}

	/* here picture of all scalers*/
	obname.Form("VulomScalers");
	pVulomScalers = GetPicture(obname.Data());
	if (pVulomScalers == 0)
	{
		obtitle.Form("Overview all VulomScalers");
		foldername.Form("Vulom");
		pVulomScalers = new TGo4Picture(obname.Data(), obtitle.Data());
		int xdiv = 0;
		int ydiv = (int) TMath::Sqrt(VULOM_SCALERNUM);
		double yrest = VULOM_SCALERNUM % ydiv;
		if (yrest == 0)
		{
			xdiv = ydiv;
		}
		else
		{
			xdiv = ydiv + 1;
		}
		pVulomScalers->SetDivision(ydiv, xdiv);
		int x = 0, y = 0;
		for (int sc = 0; sc < VULOM_SCALERNUM; ++sc)
		{
			x = (sc % xdiv);
			y = (sc - x) / xdiv;
			pVulomScalers->Pic(y, x)->AddObject(hVulomScaler[sc]);
			pVulomScalers->Pic(y, x)->SetFillAtt(5, 3001);
		}
		AddPicture(pVulomScalers, foldername.Data());
	}

}




//-----------------------------------------------------------
// event function
Bool_t TVupromQFWProc::BuildEvent(TGo4EventElement* target)
{
	// called by framework from TVupromQFWEvent to fill it
	static UInt_t evnum=0;
	TVupromQFWEvent* VupromQFWEvent = (TVupromQFWEvent*) target;

	TGo4MbsEvent* source = (TGo4MbsEvent*) GetInputEvent();
	if (source == 0)
	{
		cout << "AnlProc: no input event !" << endl;
		return kFALSE;
	}
	if (source->GetTrigger() > 11)
	{
		cout << "**** TVupromQFWProc: Skip trigger event" << endl;
		VupromQFWEvent->SetValid(kFALSE); // not store
		return kFALSE;
	}
	// first we fill the TVupromQFWEvent with data from MBS source
	// we have up to two subevents, crate 1 and 2
	// Note that one has to loop over all subevents and select them by
	// crate number:   psubevt->GetSubcrate(),
	// procid:         psubevt->GetProcid(),
	// and/or control: psubevt->GetControl()
	// here we use only crate number

	source->ResetIterator();
	TGo4MbsSubEvent* psubevt(0);
	while ((psubevt = source->NextSubEvent()) != 0)
	{ // loop over subevents
		Int_t *pdata = psubevt->GetDataField();
		Int_t lwords = psubevt->GetIntLen();
		Int_t wordcount = 0;

		if ((psubevt->GetSubcrate() == 0) && (psubevt->GetControl() == 9)
				&& (psubevt->GetProcid() == 1))

		{
			/* first evaluate qfw data from beginning of subevent:*/

			// header with setup:
			VupromQFWEvent->fQfwSetup = *pdata++;
			wordcount++;
			VupromQFWEvent->fQfwPreMTime = *pdata++;
			wordcount++;
			VupromQFWEvent->fQfwMTime = *pdata++;
			wordcount++;


			UInt_t timeslices= *pdata++; // new: dynamic time slice number
			wordcount++;
			if(timeslices>VUPROM_QFWSLICES)
			  {
				  printf ("Warning: reducing number of time slices from 0x%x to maximum 0x%x\n", timeslices,VUPROM_QFWSLICES);
				  timeslices=VUPROM_QFWSLICES;
			  }
			// check if number of time slices was changed:
			if(timeslices && (timeslices!= VupromQFWEvent->fQfwNumTimeSlices))
				{
				  printf ("INFO: BuildEvent is changing number of time slices from 0x%x to  0x%x\n", VupromQFWEvent->fQfwNumTimeSlices,timeslices);
				  InitDisplay(timeslices,kTRUE); // replace old histograms and create new time binning
				}
			VupromQFWEvent->fQfwNumTimeSlices=timeslices;
			hVupromSetup->Fill(VupromQFWEvent->fQfwSetup);
			hVupromPreMtime->Fill(VupromQFWEvent->fQfwPreMTime);
			hVupromMtime->Fill(VupromQFWEvent->fQfwMTime);


			for (int brd = 0;  brd< VUPROM_QFWBOARDS; ++brd)
				{

				// the qfw data:
				for (int q = 0; q < VUPROM_QFWNUM; ++q)
				{
					hVupromQFWTrace[brd][q]->Reset("");

					// prescaler:
					for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
					{
						VupromQFWEvent->fQfwPre[brd][q][ch] = *pdata++;
						wordcount++;
						hVupromQFWTrace[brd][q]->SetBinContent(ch + 1,
								VupromQFWEvent->fQfwPre[brd][q][ch]);
						hVupromQFW[brd][q]->AddBinContent(ch + 1,
								VupromQFWEvent->fQfwPre[brd][q][ch]);
					}
				}
				/* scaler values:*/
				for (int q = 0; q < VUPROM_QFWNUM; ++q)
				{
					for (unsigned int sl = 0; sl < VupromQFWEvent->fQfwNumTimeSlices; ++sl)
					{
					for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
						{


						VupromQFWEvent->fQfw[brd][q][ch][sl] = *pdata++;
						wordcount++;
						hVupromQFWTrace[brd][q]->SetBinContent(ch + 1 + (1+sl)*VUPROM_QFWCHANS,
								VupromQFWEvent->fQfw[brd][q][ch][sl]);
						hVupromQFW[brd][q]->AddBinContent(ch + 1 + (1+sl)*VUPROM_QFWCHANS,
								VupromQFWEvent->fQfw[brd][q][ch][sl]);

						}

					}
				}
				/* errorcount values:*/
				for (int q = 0; q < VUPROM_QFWNUM; ++q)
				{
					for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
					{
						VupromQFWEvent->fQfwErr[brd][q][ch] = *pdata++;
						wordcount++;
						hVupromQFWTrace[brd][q]->SetBinContent(ch + 1 + 2
								* VUPROM_QFWCHANS, VupromQFWEvent->fQfwErr[brd][q][ch]);
						hVupromQFW[brd][q]->AddBinContent(ch + 1 + 2 * VUPROM_QFWCHANS,
								VupromQFWEvent->fQfwErr[brd][q][ch]);
					}
				}

				} // board
			if (wordcount +1 > lwords)
				continue; // leave subevent loop if no more data available
			//GO4_STOP_ANALYSIS_MESSAGE(
			//					"**** TVupromQFWProc: found wordcount=%d lwords=%d ",
			//					wordcount, lwords);
			/* optionally read vulom scaler values at the end: */
			for (int ch = 0; ch < VULOM_SCALERCHANS; ++ch)
			{
				for (int sc = 0; sc < VULOM_SCALERNUM; ++sc)
				{
					// TODO: use compressed data format for scaler!
					VupromQFWEvent->fVulomScaler[sc][ch] = *pdata++; // copy to output event
					if (VupromQFWEvent->fVulomScaler[sc][ch])
						hVulomScaler[sc]->Fill(ch); // accumulate histogram
				}
			}
			VupromQFWEvent->fFC_VulomScaler = *pdata++;
			hVulomDeadTime->Fill(VupromQFWEvent->fFC_VulomScaler);

			VupromQFWEvent->fFC_VulomDeadTime = *pdata++;
			hVulomFC->Fill(VupromQFWEvent->fFC_VulomDeadTime);
		} // if ids matching
		else
		{
			// if we find other ids, display them here and stop. later comment out
			GO4_STOP_ANALYSIS_MESSAGE(
					"**** TVupromQFWProc: found unknown subevent ids - crate: %d control:%d procid%d. Please check set up!",
					psubevt->GetSubcrate(), psubevt->GetControl(),
					psubevt->GetProcid());
		}
	} // while subevents

	FillGrids(VupromQFWEvent);

	VupromQFWEvent->SetValid(kTRUE); // to store
	evnum++;
	if(fPar->fSlowMotionStart>0)
		{
			if(evnum>fPar->fSlowMotionStart)
			GO4_STOP_ANALYSIS_MESSAGE("Stopped for slow motion mode at event %d",evnum);	
		}	

	
	return kTRUE;
}

void TVupromQFWProc::FillGrids(TVupromQFWEvent* out)
{

	Bool_t dostop=kFALSE;
	Double_t mtime=out->fQfwMTime * 20 / 1000; // measurement time in us
	Double_t premtime=out->fQfwPreMTime * 20 / 1000; // measurement time in us

	/* evaluate measurement setup*/
	TString setup;
	switch(out->fQfwSetup)
	{
		case 0:
			setup.Form("(-) [ 2.5pF & 0.25pC]");
		break;

		case 1:
			setup.Form("(-) [25.0pF & 2.50pC]");
		break;

		case 2:
			setup.Form("(+) [ 2.5pF & 0.25pC]");
		break;

		case 3:
			setup.Form("(+) [25.0pF & 2.50pC]");
		break;

		case 0x10:
			setup.Form("1000uA (-) [ 2.5pF & 0.25pC]");
		break;

		case 0x11:
			setup.Form("1000uA (-) [25.0pF & 2.50pC]");
		break;

		case 0x12:
			setup.Form("1000uA (+) [ 2.5pF & 0.25pC]");
		break;

		case 0x13:
			setup.Form("1000uA (+) [25.0pF & 2.50pC]");
			break;

		default:
			setup.Form("unknown setup %d", out->fQfwSetup);
			break;



	};





// APPEND TIME RANGES:
	TString mtitle;
	mtitle.Form("%s dt=%.2E us", setup.Data(), mtime);

	for (int grid = 0;  grid< VUPROM_QFWGRIDS; ++grid)
		{
		hBeamX[grid]->Reset("");
		hBeamY[grid]->Reset("");
		hBeamXSlice[grid]->Reset("");
		hBeamYSlice[grid]->Reset("");
		hBeamXSliceOffs[grid]->Reset("");
		hBeamYSliceOffs[grid]->Reset("");
		hBeamPreX[grid]->Reset("");
		hBeamPreY[grid]->Reset("");
		hBeamX[grid]->SetTitle(mtitle.Data());
		hBeamY[grid]->SetTitle(mtitle.Data());
		hBeamXSlice[grid]->SetTitle(mtitle.Data());
		hBeamYSlice[grid]->SetTitle(mtitle.Data());
		mtitle.Form("%s dt=%.2E us", setup.Data(),premtime);
		hBeamPreX[grid]->SetTitle(mtitle.Data());
		hBeamPreY[grid]->SetTitle(mtitle.Data());




	for (int brd = 0;  brd< VUPROM_QFWBOARDS; ++brd)
	{



		for (int q = 0; q < VUPROM_QFWNUM; ++q)
		{
			for (int ch = 0; ch < VUPROM_QFWCHANS; ++ch)
			{
				int xpos = gBeamX[grid][brd][q][ch];
				int ypos = gBeamY[grid][brd][q][ch];
				if (xpos >= 0)
				{
					UInt_t sum=0;
					for (int sl = 0; sl < VUPROM_QFWSLICES; ++sl)
							{
								// JAM: newly added correction and display of background offset:
								if(fPar->fMeasureBackground)
									{
										fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
									}
								hBeamXSliceOffs[grid]->SetBinContent(1 + xpos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
								sum+=fPar->GetCorrectedQFWValue(brd,q,ch,sl, out->fQfw[brd][q][ch][sl]);
								hBeamXSlice[grid]->SetBinContent(1 + xpos, 1+sl, fPar->GetCorrectedQFWValue(brd,q,ch,sl, out->fQfw[brd][q][ch][sl]));
								UInt_t prev=hBeamAccXSlice[grid]->GetBinContent(1 + xpos,1+sl);
								hBeamAccXSlice[grid]->SetBinContent(1 + xpos,1+sl, prev+ fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
							}

					hBeamX[grid]->SetBinContent(1 + xpos, sum);
					hBeamAccX[grid]->AddBinContent(1 + xpos, sum);

					hBeamPreX[grid]->SetBinContent(1 + xpos, out->fQfwPre[brd][q][ch]);
					hBeamPreAccX[grid]->SetBinContent(1 + xpos, out->fQfwPre[brd][q][ch]);
				}
				if (ypos >= 0)
				{
					UInt_t sum=0;
					for (int sl = 0; sl < VUPROM_QFWSLICES; ++sl)
							{
								if(fPar->fMeasureBackground)
									{
										fPar->AddOffsetMeasurement(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
									}
								hBeamYSliceOffs[grid]->SetBinContent(1 + ypos, 1+sl, fPar->fQFWOffsets[brd][q][ch][sl]); // show current averaged offset
								sum+=fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]);
								hBeamYSlice[grid]->SetBinContent(1 + ypos, 1+sl, fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
								UInt_t prev=hBeamAccYSlice[grid]->GetBinContent(1 + ypos,1+sl);
								hBeamAccYSlice[grid]->SetBinContent(1 + ypos, 1+sl, prev + fPar->GetCorrectedQFWValue(brd,q,ch,sl,out->fQfw[brd][q][ch][sl]));
							}
					
					hBeamY[grid]->SetBinContent(1 + ypos, sum);
					hBeamAccY[grid]->AddBinContent(1 + ypos, sum);
					hBeamPreY[grid]->SetBinContent(1 + ypos, out->fQfwPre[brd][q][ch]);
					hBeamPreAccY[grid]->SetBinContent(1 + ypos, out->fQfwPre[brd][q][ch]);
			
					if(grid==0 && ypos==6)
						{		
						// condition to find broken wire:
						if(sum<100)
						{	
							dostop=kTRUE;
						}
					}

				}
			}

		}

	}// board loop


	



		out->fBeamMeanX[grid]=hBeamX[grid]->GetMean();

		out->fBeamMeanY[grid]=hBeamY[grid]->GetMean();

		out->fBeamRMSX[grid]=hBeamX[grid]->GetRMS();

		out->fBeamRMSY[grid]=hBeamY[grid]->GetRMS();

		out->fBeamPreMeanX[grid]=hBeamPreX[grid]->GetMean();

		out->fBeamPreMeanY[grid]=hBeamPreY[grid]->GetMean();

		out->fBeamPreRMSX[grid]=hBeamPreX[grid]->GetRMS();

		out->fBeamPreRMSY[grid]=hBeamPreX[grid]->GetRMS();



		hBeamMeanXY[grid]->Fill(out->fBeamMeanX[grid],out->fBeamMeanY[grid]);
		hBeamPreMeanXY[grid]->Fill(out->fBeamPreMeanX[grid],out->fBeamPreMeanY[grid]);

		hBeamRMSX[grid]->Fill(out->fBeamRMSX[grid]);
		hBeamRMSY[grid]->Fill(out->fBeamRMSY[grid]);
		hBeamPreRMSX[grid]->Fill(out->fBeamPreRMSX[grid]);
		hBeamPreRMSY[grid]->Fill(out->fBeamPreRMSY[grid]);


		// evaluate here mean value and sigma of profile counts
		// first x direction:
		Int_t cmax=cBeamXSliceCond[grid]->GetCMax(hBeamXSlice[grid]);
		TH1I haux("temp","temp",cmax,0,cmax); // auxiliary histogram to calculate mean and rms of counts
		for(int x=0;x<gBeamWiresX[grid];++x)
		{
			for(int y=0; y<VUPROM_QFWSLICES;++y)
			{
				if(cBeamXSliceCond[grid]->Test(x,y))
					{
						haux.Fill(hBeamXSlice[grid]->GetBinContent(x+1,y+1));
					}
			}

		}
		Double_t MeanCountsX=haux.GetMean();
		Double_t RMSCountsX=haux.GetRMS();
		mtitle.Form("%s mean=%.2E sigma=%.2E", hBeamXSlice[grid]->GetTitle(), MeanCountsX, RMSCountsX);
		hBeamXSlice[grid]->SetTitle(mtitle.Data());

		// y direction:
		Int_t cmay=cBeamYSliceCond[grid]->GetCMax(hBeamYSlice[grid]);
		TH1I hauy("temp2","temp2",cmay,0,cmay); // auxiliary histogram to calculate mean and rms of counts
		for(int x=0;x<gBeamWiresY[grid];++x)
		{
			for(int y=0; y<VUPROM_QFWSLICES;++y)
			{
				if(cBeamYSliceCond[grid]->Test(x,y))
					{
						hauy.Fill(hBeamYSlice[grid]->GetBinContent(x+1,y+1));
					}
			}

		}
		Double_t MeanCountsY=hauy.GetMean();
		Double_t RMSCountsY=hauy.GetRMS();
		mtitle.Form("%s mean=%.2E sigma=%.2E", hBeamYSlice[grid]->GetTitle(), MeanCountsY, RMSCountsY);
		hBeamYSlice[grid]->SetTitle(mtitle.Data());

		hBeamMeanCountsX[grid]->Fill(MeanCountsX);
		hBeamMeanCountsY[grid]->Fill(MeanCountsY);
		hBeamRMSCountsX[grid]->Fill(RMSCountsX);
		hBeamRMSCountsY[grid]->Fill(RMSCountsY);

	}// grid loop

	
	if(dostop)
		{
			TGo4Analysis::Instance()->SendObjectToGUI(hBeamYSlice[0]);
			TGo4Analysis::Instance()->SendObjectToGUI(hBeamYSlice[0]);							
			GO4_STOP_ANALYSIS_MESSAGE("Found wire 7 below 100 counts");

		}

}


void TVupromQFWProc::MapGrids()
{

	cout << "**** TVupromQFWProc: Setting up beam coordinate mapping:" << endl;
	// map both for scalers and prescalers


	// first we reset all mappings:
	for (int g = 0; g < VUPROM_QFWGRIDS; ++g)
		{

		gBeamWiresX[g]=16;
		gBeamWiresY[g]=16;

		for (int b = 0; b < VUPROM_QFWBOARDS; ++b)
			{
				for (int qfw = 0; qfw < VUPROM_QFWNUM; ++qfw)
				{
					for (int c = 0; c < VUPROM_QFWCHANS; ++c)
					{
						gBeamX[g][b][qfw][c] = -1; // reset unused channels
						gBeamY[g][b][qfw][c] = -1;
					} // channels
				} // qfw

			} // boards
		} // grids


	// HERE IS THE GRID-MAPPING:
	// gbeamX[gridnumber ][boardnumber][qfwnumber][scaler-channelid]= X_wirenumber
	// gbeamY[gridnumber ][boardnumber][qfwnumber][scaler-channelid]= Y_wirenumber

	// to begin with we assume both boards have same grid mapping JAM
	for (int gb = 0; gb < VUPROM_QFWBOARDS; ++gb)
				{
			// [gridnumber ][boardnumber][qfwnumber][scaler-channelid]
			gBeamX[gb][gb][0][0] = 0;
			gBeamX[gb][gb][0][1] = 1;
			gBeamX[gb][gb][0][2] = 2;
			gBeamX[gb][gb][0][3] = 3;
			gBeamX[gb][gb][1][0] = 4;
			gBeamX[gb][gb][1][1] = 5;
			gBeamX[gb][gb][1][2] = 6;
			gBeamX[gb][gb][1][3] = 7;
			gBeamX[gb][gb][2][0] = 8;
			gBeamX[gb][gb][2][1] = 9;
			gBeamX[gb][gb][2][2] = 10;
			gBeamX[gb][gb][2][3] = 11;
			gBeamX[gb][gb][3][0] = 12;
			gBeamX[gb][gb][3][1] = 13;
			gBeamX[gb][gb][3][2] = 14;
			gBeamX[gb][gb][3][3] = 15;

			gBeamY[gb][gb][4][0] = 15;
			gBeamY[gb][gb][4][1] = 14;
			gBeamY[gb][gb][4][2] = 13;
			gBeamY[gb][gb][4][3] = 12;
			gBeamY[gb][gb][5][0] = 11;
			gBeamY[gb][gb][5][1] = 10;
			gBeamY[gb][gb][5][2] = 9;
			gBeamY[gb][gb][5][3] = 8;
			gBeamY[gb][gb][6][0] = 7;
			gBeamY[gb][gb][6][1] = 6;
			gBeamY[gb][gb][6][2] = 5;
			gBeamY[gb][gb][6][3] = 4;
			gBeamY[gb][gb][7][0] = 3;
			gBeamY[gb][gb][7][1] = 2;
			gBeamY[gb][gb][7][2] = 1;
			gBeamY[gb][gb][7][3] = 0;

				} // gb
}
