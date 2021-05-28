#include "TQFWDisplay.h"

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

#include "TQFWRawEvent.h"
#include "TQFWRawParam.h"
#include "TQFWProfileEvent.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

TQFWDisplay::TQFWDisplay(Int_t deviceid) :
    fDisplayId(deviceid)
{
  ClearLoopDisplays();
}

TQFWDisplay::~TQFWDisplay()
{
  for (unsigned i = 0; i < fLoops.size(); ++i)
  {
    delete fLoops[i];
  }
}

void TQFWDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
  // if we have subdisplays for loops, initialize these with numbe of timeslices
  for (unsigned i = 0; i < fLoops.size(); ++i)
  {
    fLoops[i]->InitDisplay(timeslices, replace);
  }
}



TH1* TQFWDisplay::MakeVarbinsTH1(Bool_t replace, char type, const char* fullname, const char* title, Int_t nbinsx,
    Double_t* xbins, const char* xtitle, const char* ytitle)
{
  //fbObjMade = kFALSE;
  TString foldername, histoname;

  if ((fullname == 0) || (strlen(fullname) == 0))
  {
    TGo4Log::Error("Histogram name not specified, can be a hard error");
    return 0;
  }
  const char* separ = strrchr(fullname, '/');
  if (separ != 0)
  {
    histoname = separ + 1;
    foldername.Append(fullname, separ - fullname);
  }
  else
    histoname = fullname;

  int itype = 0;
  const char* sclass = "TH1I";
  switch (type)
  {
    case 'I':
    case 'i':
      itype = 0;
      sclass = "TH1I";
      break;
    case 'F':
    case 'f':
      itype = 1;
      sclass = "TH1F";
      break;
    case 'D':
    case 'd':
      itype = 2;
      sclass = "TH1D";
      break;
    case 'S':
    case 's':
      itype = 3;
      sclass = "TH1S";
      break;
    case 'C':
    case 'c':
      itype = 4;
      sclass = "TH1C";
      break;
    default:
      TGo4Log::Error("There is no histogram type: %c, use I instead", type);
      break;
  }

  TH1* oldh = GetHistogram(fullname);
  if (oldh != 0)
  {
    if (oldh->InheritsFrom(sclass) && !replace)
    {
      if (title)
        oldh->SetTitle(title);
      if (xtitle)
        oldh->GetXaxis()->SetTitle(xtitle);
      if (ytitle)
        oldh->GetYaxis()->SetTitle(ytitle);
      return oldh;
    }

    if (oldh->InheritsFrom(sclass))
      TGo4Log::Info("Rebuild existing histogram %s", fullname);
    else
      TGo4Log::Info("There is histogram %s with type %s other than specified %s, rebuild", fullname, oldh->ClassName(),
          sclass);
  // Adjusted with new code in go4 2020 JAM:
   // do not delete histogram immediately
      TGo4Analysis::Instance()->RemoveHistogram(fullname, kFALSE);
	// note JAM: not yet as forward sith both arguments in processor base class:
      // prevent ROOT to complain about same name
      oldh->SetName("___");
  
  }




  TH1* newh = 0;

  switch (itype)
  {
    case 0:
      newh = new TH1I(histoname, title, nbinsx, xbins);
      break;
    case 1:
      newh = new TH1F(histoname, title, nbinsx, xbins);
      break;
    case 2:
      newh = new TH1D(histoname, title, nbinsx, xbins);
      break;
    case 3:
      newh = new TH1S(histoname, title, nbinsx, xbins);
      break;
    case 4:
      newh = new TH1C(histoname, title, nbinsx, xbins);
      break;
  }

  newh->SetTitle(title);

  if (xtitle)
    newh->GetXaxis()->SetTitle(xtitle);
  if (ytitle)
    newh->GetYaxis()->SetTitle(ytitle);
    
// Adjusted with new code in go4 2020 JAM:
 if (oldh) {
      if ((oldh->GetDimension()==1) && !replace) newh->Add(oldh);
      delete oldh; oldh = 0;
   }

   if (foldername.Length() > 0)
      AddHistogram(newh, foldername.Data());
   else
      AddHistogram(newh);


  //if (oldh)
  //{
    //if ((oldh->GetDimension() == 1) && !replace)
      //newh->Add(oldh);
    //RemoveHistogram(fullname);
  //}

  //if (foldername.Length() > 0)
    //AddHistogram(newh, foldername.Data());
  //else
    //AddHistogram(newh);

  // fbObjMade = kTRUE;

  return newh;
}


// stolen from go4 mbs viewer
void TQFWDisplay::IncTrending( TH1 * histo, int value, bool forwards )
{
   if(histo==0) return;
   int bins=histo->GetNbinsX();
   //bool forwards=true;
   int j,dj;
   if(forwards)
      dj=-1;
   else
      dj=+1;
   for(int i=0;i<bins;i++)
   {
      if(forwards)
         j=bins-i;
      else
         j=i;
      int oldval=histo->GetBinContent(j+dj);
      histo->SetBinContent(j,oldval);
   }
   histo->SetBinContent(j+dj,value);
}




////////////////////////////////////////////////

TQFWLoopDisplay::TQFWLoopDisplay(Int_t deviceid, Int_t loopid) :
    TQFWDisplay(deviceid), fLoopId(loopid), fTimeSlices(0)
{
}

TQFWLoopDisplay::~TQFWLoopDisplay()
{

}

void TQFWLoopDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
  fTimeSlices = timeslices;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

TQFWBoardLoopDisplay::TQFWBoardLoopDisplay(Int_t boardid, Int_t loopid) :
    TQFWLoopDisplay(boardid, loopid), hQFWRaw(0), hQFWRawTrace(0)
{
  //TString loopname.Form("Board%02d/Loop%02d_", boardid, loopid);
  //InitDisplay(PEXOR_QFWSLICES);

}

void TQFWBoardLoopDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
  TQFWLoopDisplay::InitDisplay(timeslices, replace);
  if (replace)
    SetMakeWithAutosave(kFALSE);
  TGo4Log::Info("TQFWBoardLoopDisplay: Initializing histograms with %d timeslices for Board %d Loop %d", timeslices,
      GetDevId(), GetLoopId());

  // Creation of histograms (or take them from autosave)
  TString obname;
  TString obtitle;
  TString foldername;
  TString binlabel;
  Int_t brd = fDisplayId;
  Int_t loop = fLoopId;

  obname.Form("Board%d/Loop%d/Brd%d-Loop%d-Sum", brd, loop, brd, loop);
  obtitle.Form("QFW Board %d Loop %d Accumulated", brd, loop);
  int qbins = timeslices * PEXOR_QFWCHANS;
  //std::cout <<"*****qbins"<<qbins << std::endl;
  hQFWRaw = MakeTH1('I', obname.Data(), obtitle.Data(), qbins, 0, qbins, "QFW scaler sums");

  obname.Form("Board%d/Loop%d/Brd%d-Loop%d-Trace", brd, loop, brd, loop);
  obtitle.Form("QFW Board %d Loop %d Trace", brd, loop);
  hQFWRawTrace = MakeTH1('I', obname.Data(), obtitle.Data(), qbins, 0, qbins, "QFW scalers");

  if (replace)
  {    // avoid problems when taking it from autosave with different binning
    for (int sl = 0; sl < timeslices; ++sl)
    {
      for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
      {
        binlabel.Form("SL%d-%d", sl, ch);
        //cout <<"binlabel:"<<binlabel.Data() << endl;
        hQFWRaw->GetXaxis()->SetBinLabel(1 + sl * PEXOR_QFWCHANS + ch, binlabel.Data());
        hQFWRawTrace->GetXaxis()->SetBinLabel(1 + sl * PEXOR_QFWCHANS + ch, binlabel.Data());
      }
    }
  }
}

//***********************************************************

TQFWBoardDisplay::TQFWBoardDisplay(Int_t boardid) :
    TQFWDisplay(boardid), hQFWRaw2D(0), hQFWRaw2DTrace(0), hQFWRawErr(0), hQFWRawErrTr(0), pPexorQfws(0),
        pPexorQfwsTrace(0)
{
  //TString boardname.Form("Board%02d", boardid);"Wrong optic format - 0x34 are expected0-7 bits not as expected"
  for (int i = 0; i < PEXOR_QFWLOOPS; ++i)
  {
    AddLoopDisplay(new TQFWBoardLoopDisplay(boardid, i));
  }
  InitDisplay(PEXOR_QFWSLICES);
}

TQFWBoardDisplay::~TQFWBoardDisplay()
{

}

/* access to histogram set for current loop id*/
TQFWBoardLoopDisplay* TQFWBoardDisplay::GetLoopDisplay(Int_t index)
{
  return (TQFWBoardLoopDisplay*) GetSubDisplay(index);
}

void TQFWBoardDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
  unsigned numloops = GetNumLoops();
  
//   std::cout <<"*************** TQFWBoardDisplay::InitDisplay with timeslices"<<timeslices << std::endl;
//   std::cout <<"*************** TQFWBoardDisplay::InitDisplay finds numloops="<<numloops << std::endl;
  
  Int_t subtimes[numloops];
  if (timeslices > 0)
  {
    TQFWDisplay::InitDisplay(timeslices, replace);    // handle initialization of subloops
    for (unsigned subix = 0; subix < numloops; ++subix)
    {
      subtimes[subix] = timeslices;
    }
  }
  else
  {
    timeslices = 0;    // otherwise do not specify timeslice of subloops, but take values from them (see below)!

    for (unsigned subix = 0; subix < numloops; ++subix)
    {
      subtimes[subix] = GetSubDisplay(subix)->GetTimeSlices();
      timeslices += subtimes[subix];
//          std::cout <<"*************** TQFWBoardDisplay::InitDisplay add "<< subix<<". timeslice"<< subtimes[subix] << std::endl;

    }
    numloops = 1;    // we sum up all timeslices, so need only factor 1 below!
//      std::cout <<"*************** TQFWBoardDisplay::InitDisplay uses true sum timeslices"<<timeslices << std::endl;
  }

  TString obname;
  TString obtitle;

  if (replace)
    SetMakeWithAutosave(kFALSE);
  Int_t brd = GetDevId();

  hQFWRaw2D = MakeTH2('I', Form("Board%d/Brd%d-2D-Sum", brd, brd), Form("QFW Board %d accumulated for all loops", brd),
      numloops * timeslices, 0, numloops * timeslices, PEXOR_QFWCHANS, 0, PEXOR_QFWCHANS, "loop", "ch");

  hQFWRaw2DTrace = MakeTH2('I', Form("Board%d/Brd%d-2D-Trace", brd, brd), Form("QFW Board %d trace for all loops", brd),
      numloops * timeslices, 0, numloops * timeslices, PEXOR_QFWCHANS, 0, PEXOR_QFWCHANS, "loop", "ch");

  if (replace)
  {    // avoid problems when taking it from autosave with different binning
    // set axis description:
    numloops = GetNumLoops();
    int loopoffset = 0;
    TString binlabel;
    for (unsigned loop = 0; loop < numloops; ++loop)
    {
      for (int t = 0; t < subtimes[loop]; ++t)
      {

        binlabel.Form("L%d-T%d", loop, t);
        //cout <<"binlabel:"<<binlabel.Data() << endl;
        hQFWRaw2D->GetXaxis()->SetBinLabel(1 + loopoffset + t, binlabel.Data());
        hQFWRaw2DTrace->GetXaxis()->SetBinLabel(1 + loopoffset + t, binlabel.Data());
      }
      loopoffset += subtimes[loop];
    }
  }

//

  hQFWRawErr = MakeTH1('I', Form("Board%d/Brd%d-Err", brd, brd), Form("QFW errors in board %d", brd), PEXOR_QFWNUM, 0,
      PEXOR_QFWNUM);

//  hQFWRawErrTr = MakeTH1('I', Form("Board%d/Brd%d-ErrTr", brd, brd), Form("QFW errors in board %d (snap shot)", brd),
//      PEXOR_QFWNUM, 0, PEXOR_QFWNUM);

  hQFWOffsets= MakeTH1('I', Form("Board%d/Brd%d-Offsets", brd, brd), Form("QFW frontend measured channel offsets in board %d", brd),
      PEXOR_QFWCHANS, 0, PEXOR_QFWCHANS);


  hEventDelta= MakeTH1('I', Form("Board%d/Brd%d-EventDelta", brd, brd), Form("Event sequence differences on board %d", brd),
      100, 0, 100);



  for(int i=0; i<PEXOR_NUMTHERMS;++i)
  {
  // temperatures of sensors in Celsius. accumulated distribution
   hTemps[i]= MakeTH1('I', Form("Board%d/Sensors/Distribution/TempDist_%d_%d", brd, brd,i),
       Form("Thermometer %d board %d distribution", i, brd), 3000, -150, 150, "Celsius","Events");

   hTempsTrend[i]=MakeTH1('D', Form("Board%d/Sensors/Trend/TempTrend_%d_%d", brd, brd,i),
       Form("Thermometer %d board %d trending", i, brd), 1000, 0, 1000, "Seconds","Celsius");

  }

  for(int f=0; f<PEXOR_NUMFANS;++f)
   {
   // temperatures of sensors in Celsius. accumulated distribution
    hFans[f]= MakeTH1('I', Form("Board%d/Sensors/Distribution/FanDist_%d_%d", brd, brd,f),
        Form("Fan %d board %d RPM distribution", f, brd), 1000, 0, 10000, "RPM","Events");

    hFansTrend[f]=MakeTH1('D', Form("Board%d/Sensors/Trend/FanTrend_%d_%d", brd, brd,f),
        Form("Fan %d board %d RPM trending", f, brd), 1000, 0, 1000, "Seconds","RPM");

   }





  obname.Form("QFW_Rawscalers_Brd%d", brd);
  pPexorQfws = GetPicture(obname.Data());
  if (pPexorQfws == 0)
  {
    obtitle.Form("Overview all QFW Accumulated Board%d", brd);
    pPexorQfws = new TGo4Picture(obname.Data(), obtitle.Data());

    pPexorQfws->SetDivision(fLoops.size(), 1);
    for (unsigned loop = 0; loop < fLoops.size(); ++loop)
    {
      TQFWBoardLoopDisplay* ldisp = dynamic_cast<TQFWBoardLoopDisplay*>(fLoops[loop]);
      if (ldisp == 0)
        continue;
      pPexorQfws->Pic(loop, 0)->AddObject(ldisp->hQFWRaw);
      pPexorQfws->Pic(loop, 0)->SetFillAtt(5, 3001);
    }
    AddPicture(pPexorQfws, Form("Board%d", brd));
  }

  obname.Form("QFW_Rawscalers_Brd%d_Tr", brd);
  pPexorQfwsTrace = GetPicture(obname.Data());
  if (pPexorQfwsTrace == 0)
  {
    obtitle.Form("Overview all QFW Trace Board%d", brd);
    pPexorQfwsTrace = new TGo4Picture(obname.Data(), obtitle.Data());

    pPexorQfwsTrace->SetDivision(fLoops.size(), 1);
    for (unsigned loop = 0; loop < fLoops.size(); ++loop)
    {
      TQFWBoardLoopDisplay* ldisp = GetLoopDisplay(loop);
      if (ldisp == 0)
        continue;
      pPexorQfwsTrace->Pic(loop, 0)->AddObject(ldisp->hQFWRawTrace);
      pPexorQfwsTrace->Pic(loop, 0)->SetFillAtt(5, 3001);
    }
    AddPicture(pPexorQfwsTrace, Form("Board%d", brd));
  }





}

///////////////////////////////////////////////////////////////7
// GRID and CUP mapped histograms below:

//***********************************************************

TQFWGridLoopDisplay::TQFWGridLoopDisplay(Int_t gridid, Int_t loopid) :
    TQFWLoopDisplay(gridid, loopid), hBeamXSlice(0), hBeamYSlice(0), hBeamXSliceOffs(0), hBeamYSliceOffs(0),
        hBeamAccXSlice(0), hBeamAccYSlice(0),
        hBeamChargeXSlice(0), hBeamChargeYSlice(0), hBeamAccChargeXSlice(0), hBeamAccChargeYSlice(0),
        hBeamCurrentXSlice(0), hBeamCurrentYSlice(0), hBeamAveCurrentXSlice(0), hBeamAveCurrentYSlice(0),
        hBeamLoopX(0), hBeamLoopY(0), hBeamAccLoopX(0), hBeamAccLoopY(0),
        hBeamTimeX(0), hBeamTimeY(0), hBeamAccTimeX(0),hBeamAccTimeY(0),
        hPosLoopX(0), hPosLoopY(0), hPosAccLoopX(0), hPosAccLoopY(0),
        hPosQLoopX(0), hPosQLoopY(0), hPosQAccLoopX(0), hPosQAccLoopY(0),
        hPosILoopX(0), hPosILoopY(0), hPosIAveLoopX(0), hPosIAveLoopY(0),
        hBeamMeanCountsX(0), hBeamMeanCountsY(0),
        hBeamRMSCountsX(0), hBeamRMSCountsY(0), cBeamXSliceCond(0), cBeamYSliceCond(0), fGridData(0), fParam(0)
{

}

//***********************************************************
TQFWGridLoopDisplay::~TQFWGridLoopDisplay()
{

}
void TQFWGridLoopDisplay::SetGridEvent(TQFWProfileEvent* out)
{
  fGridData = out->GetGrid(GetDevId());

}

void TQFWGridLoopDisplay::InitDisplay(int timeslices, Bool_t replace)
{
  TGo4Log::Info("TQFWGridLoopDisplay: Initializing histograms with %d timeslices for Grid %d Loop %d", timeslices,
      GetDevId(), GetLoopId());
  TString obname;
  TString obtitle;
  TString foldername;
  if (replace)    //TGo4Analysis::Instance()->
    SetMakeWithAutosave(kFALSE);
  TQFWLoopDisplay::InitDisplay(timeslices, replace);

  Int_t grid = GetDevId();
  Int_t loop = GetLoopId();
  Int_t wiresX = PEXOR_QFW_WIRES;
  Int_t wiresY = PEXOR_QFW_WIRES;
  Int_t gix = -1;
  if (fGridData)
  {
    // take real number of wires from event object
    wiresX = fGridData->GetNumXWires();
    wiresY = fGridData->GetNumYWires();
  }

// here evaluate visible range from parameter
// note that first existing wire has index 0, last wire index (wiresX-1)
// means maxX/maxY is always upper excluded boundary
  Int_t minX = 0;
  Int_t maxX = wiresX;
  Int_t minY = 0;
  Int_t maxY = wiresY;
  if (fParam)
  {
    gix = fParam->FindGridIndex(grid);
    if (gix >= 0)
    {
      //printf("GGGGGG Using index %d of grid uid %d in loop %d \n", gix, grid, loop);

      if (fParam->fGridMinWire_X[gix] >= 0)
        minX = fParam->fGridMinWire_X[gix];
      if (fParam->fGridMaxWire_X[gix] >= 0)
        maxX = fParam->fGridMaxWire_X[gix];
      if (maxX > wiresX)
        maxX = wiresX;
      if (minX > maxX)
        minX = maxX;

      if (fParam->fGridMinWire_Y[gix] >= 0)
        minY = fParam->fGridMinWire_Y[gix];
      if (fParam->fGridMaxWire_Y[gix] >= 0)
        maxY = fParam->fGridMaxWire_Y[gix];
      if (maxY > wiresY)
        maxY = wiresY;
      if (minY > maxY)
        minY = maxY;
    }
  }

  Int_t binsX = maxX - minX;
  Int_t binsY = maxY - minY;

  /* xy beam display*/

  foldername.Form("Beam/Grid%2d/Raw/Loop_%d", grid, loop);
  //printf("JAM - grid:%d loop:%d binsX:%d binsY:%d minX:%d maxX:%d minY:%d maxY:%d timeslices:%d\n",
  //grid, loop, binsX, binsY, minX, maxX, minY, maxY, timeslices);

  hBeamXSlice = MakeTH2('D', Form("%s/Profile_X_Time_G%d_L%d", foldername.Data(), grid, loop),
      Form("X Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0, timeslices,
      "Wire", "Time Slice");
  hBeamYSlice = MakeTH2('D', Form("%s/Profile_Y_Time_G%d_L%d", foldername.Data(), grid, loop),
      Form("Y Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0, timeslices,
      "Wire", "Time Slice");

  hBeamXSliceOffs = MakeTH2('D', Form("%s/Profile_X_Time_Offset_G%d_L%d", foldername.Data(), grid, loop),
      Form("X Profile vs Time slices average offset Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0,
      timeslices, "Wire", "Time Slice");
  hBeamYSliceOffs = MakeTH2('D', Form("%s/Profile_Y_Time_Offset_G%d_L%d", foldername.Data(), grid, loop),
      Form("Y Profile vs Time slices average offset Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0,
      timeslices, "Wire", "Time Slice");

  hBeamAccXSlice = MakeTH2('D', Form("%s/ProfileSum_X_Time_G%d_L%d", foldername.Data(), grid, loop),
      Form("X Profile vs Time slices accum Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0, timeslices,
      "Wire", "Time Slice");
  hBeamAccYSlice = MakeTH2('D', Form("%s/ProfileSum_Y_Time_G%d_L%d", foldername.Data(), grid, loop),
      Form("Y Profile vs Time slices accum Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0, timeslices,
      "Wire", "Time Slice");

  hBeamLoopX = MakeTH1('D', Form("%s/Profile_X_G%d_L%d", foldername.Data(), grid, loop),
      Form("X Profile Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, "Wire");

  hBeamLoopY = MakeTH1('D', Form("%s/Profile_Y_G%d_L%d", foldername.Data(), grid, loop),
      Form("Y Profile Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, "Wire");

  hBeamAccLoopX = MakeTH1('D', Form("%s/ProfileSum_X_G%d_L%d", foldername.Data(), grid, loop),
      Form("X Profile accumulated Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, "Wire");
  hBeamAccLoopY = MakeTH1('D', Form("%s/ProfileSum_Y_G%d_L%d", foldername.Data(), grid, loop),
      Form("Y Profile accumulated Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, "Wire");
#ifdef       QFW_FILL_RMS_PROFILES
  hBeamLoopRMSX = MakeTH1('D', Form("%s/ProfileRMS_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Profile RMS Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, "Wire");

    hBeamLoopRMSY = MakeTH1('D', Form("%s/ProfileRMS_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Profile RMS Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, "Wire");

    hBeamAccLoopRMSX = MakeTH1('D', Form("%s/ProfileRMSSum_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Profile RMS accumulated Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, "Wire");
    hBeamAccLoopRMSY = MakeTH1('D', Form("%s/ProfileRMSSum_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Profile RMS accumulated Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, "Wire");
#endif




  hBeamTimeX = MakeTH1('D', Form("%s/Time_X_G%d_L%d", foldername.Data(), grid, loop),
       Form("Time Profile X Grid%2d Loop%2d", grid, loop), timeslices, 0, timeslices, "Time Slice");
  hBeamTimeY = MakeTH1('D', Form("%s/Time_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Time Profile Y Grid%2d Loop%2d", grid, loop), timeslices, 0, timeslices, "Time Slice");
  hBeamAccTimeX = MakeTH1('D', Form("%s/TimeSum_X_G%d_L%d", foldername.Data(), grid, loop),
         Form("Time Profile accumulated X Grid%2d Loop%2d", grid, loop), timeslices, 0, timeslices, "Time Slice");
  hBeamAccTimeY = MakeTH1('D', Form("%s/TimeSum_Y_G%d_L%d", foldername.Data(), grid, loop),
          Form("Time Profile accumulated Y Grid%2d Loop%2d", grid, loop), timeslices, 0, timeslices, "Time Slice");

  /* mean count of beam profile part: */

  cBeamXSliceCond = MakeWinCond(Form("%s/XSliceCond_G%d_L%d", foldername.Data(), grid, loop), minX, maxX, 0, timeslices,
      hBeamXSlice->GetName());
  cBeamYSliceCond = MakeWinCond(Form("%s/YSliceCond_G%d_L%d", foldername.Data(), grid, loop), minY, maxY, 0, timeslices,
      hBeamYSlice->GetName());


  if(fParam && fParam->fDoCountStatistics)
  {
    // may enable/disable these by parameter:
    hBeamMeanCountsX = MakeTH1('D', Form("%s/Mean_Counts_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Counts Mean Grid%2d Loop%2d", grid, loop), 80000, -100, 40000, "Mean counts");

    hBeamMeanCountsY = MakeTH1('D', Form("%s/Mean_Counts_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Counts Mean Grid%2d Loop%2d", grid, loop), 80000, -100, 40000, "Mean counts");

    hBeamRMSCountsX = MakeTH1('D', Form("%s/RMS_Counts_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Counts RMS Grid%2d Loop%2d", grid, loop), 12000, 0, 6000, "RMS counts");

    hBeamRMSCountsY = MakeTH1('D', Form("%s/RMS_Counts_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Counts RMS Grid%2d Loop%2d", grid, loop), 12000, 0, 6000, "RMS counts");


    // mean/rms count histogram per wire/qfw channel:
      for(int w=0; w< PEXOR_QFW_WIRES; ++w)
      {
        hBeamMeanCountsGridX[w] = MakeTH1('D', Form("%s/Wire_%d/Mean_Counts_X_G%d_L%d_W%d", foldername.Data(), w, grid, loop, w),
             Form("X Counts Mean Grid%2d Loop%2d Wire%2d", grid, loop, w), 80000, -100, 40000, "Mean counts");
        hBeamMeanCountsGridY[w] = MakeTH1('D', Form("%s/Wire_%d/Mean_Counts_Y_G%d_L%d_W%d", foldername.Data(), w, grid, loop, w),
             Form("Y Counts Mean Grid%2d Loop%2d Wire%2d", grid, loop, w), 80000, -100, 40000, "Mean counts");
        hBeamRMSCountsGridX[w] = MakeTH1('D', Form("%s/Wire_%d/RMS_Counts_X_G%d_L%d_W%d", foldername.Data(), w,grid, loop, w),
              Form("X Counts RMS Grid%2d Loop%2d Wire%2d", grid, loop, w), 12000, 0, 6000, "RMS counts");
        hBeamRMSCountsGridY[w] = MakeTH1('D', Form("%s/Wire_%d/RMS_Counts_Y_G%d_L%d_W%d", foldername.Data(), w,grid, loop, w),
                   Form("Y Counts RMS Grid%2d Loop%2d Wire%2d", grid, loop, w), 12000, 0, 6000, "RMS counts");
      }

  }

#ifdef   QFW_FILL_POSITION_PROFILES

  if (fParam && (gix >= 0))
  {
    // these only make sense with position map from parameter is there
    Double_t xposition[PEXOR_QFW_WIRES];
    for (Int_t ix = 0, jx = minX; (ix < binsX) && (jx + 1 < PEXOR_QFW_WIRES); ++ix, ++jx)
    {
      Double_t binwidth = fParam->fGridPosition_X[gix][jx + 1] - fParam->fGridPosition_X[gix][jx];
      if (ix == 0)
        xposition[ix] = fParam->fGridPosition_X[gix][jx] - binwidth / 2;
      xposition[ix + 1] = fParam->fGridPosition_X[gix][jx] + binwidth / 2;
      //printf("QFWQFWQFW grid %d loop%d  index %d:lowedge:%f centre:%f upedge:%f \n",grid, loop, ix,xposition[ix], fParam->fGridPosition_X[gix][jx], xposition[ix+1]);
    }

    Double_t yposition[PEXOR_QFW_WIRES];
    for (Int_t ix = 0, jx = minY; (ix < binsY) && (jx + 1 < PEXOR_QFW_WIRES); ++ix, ++jx)
    {
      Double_t binwidth = fParam->fGridPosition_Y[gix][jx + 1] - fParam->fGridPosition_Y[gix][jx];
      if (ix == 0)
        yposition[ix] = fParam->fGridPosition_Y[gix][jx] - binwidth / 2;
      yposition[ix + 1] = fParam->fGridPosition_Y[gix][jx] + binwidth / 2;
      //printf("YYYYYYYY grid %d loop%d index %d:lowedge:%f centre:%f upedge:%f \n",grid, loop,ix,yposition[ix], fParam->fGridPosition_Y[gix][jx], yposition[ix+1]);
    }

    // histograms for counts profiles versus calibrated positions:
    foldername.Form("Beam/Grid%2d/Counts/Loop_%d", grid, loop);

    hPosLoopX = MakeVarbinsTH1(replace, 'I', Form("%s/N_Position_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Position Grid%2d Loop%2d", grid, loop), binsX, xposition, "X-Position [mm]");

    hPosAccLoopX = MakeVarbinsTH1(replace, 'I', Form("%s/N_PositionSum_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Position accumulated Grid%2d Loop%2d", grid, loop), binsX, xposition, "X-Position [mm]");

    hPosLoopY = MakeVarbinsTH1(replace, 'I', Form("%s/N_Position_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Position Grid%2d Loop%2d", grid, loop), binsY, yposition, "Y-Position [mm]");

    hPosAccLoopY = MakeVarbinsTH1(replace, 'I', Form("%s/N_PositionSum_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Position accumulated Grid%2d Loop%2d", grid, loop), binsY, yposition, "Y-Position [mm]");

    //  here calibrated charge versus position profiles, trace and accumulated
    foldername.Form("Beam/Grid%2d/Charge/Loop_%d", grid, loop);

    hPosQLoopX = MakeVarbinsTH1(replace, 'D', Form("%s/Q_Position_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Charge profile Grid%2d Loop%2d", grid, loop), binsX, xposition, "X-Position [mm]", "Q [C]");

    hPosQAccLoopX = MakeVarbinsTH1(replace, 'D', Form("%s/Q_PositionSum_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X  Charge profile accumulated Grid%2d Loop%2d", grid, loop), binsX, xposition, "X-Position [mm]", "Q [C]");

    hPosQLoopY = MakeVarbinsTH1(replace, 'D', Form("%s/Q_Position_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Charge profile Grid%2d Loop%2d", grid, loop), binsY, yposition, "Y-Position [mm]", "Q [C]");

    hPosQAccLoopY = MakeVarbinsTH1(replace, 'D', Form("%s/Q_PositionSum_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y  Charge profile accumulated Grid%2d Loop%2d", grid, loop), binsY, yposition, "Y-Position [mm]", "Q [C]");

    obname.Form("Beam Position Charge Display Grid%2d Loop %2d", grid, loop);
    TGo4Picture* pic = GetPicture(obname.Data());
    if (pic == 0)
    {
      obtitle.Form("Overview beam charge position projections Grid%2d Loop %2d", grid, loop);
      //foldername.Form("Beam/Grid%2d", grid); // use same picture subfolders as for histograms
      pic = new TGo4Picture(obname.Data(), obtitle.Data());
      pic->SetDivision(2, 2);
      pic->Pic(0, 0)->AddObject(hPosQLoopX);
      pic->Pic(0, 0)->SetDrawOption("LP");
      pic->Pic(0, 0)->SetLineAtt(3, 1, 2);    // solid line
      pic->Pic(0, 0)->SetMarkerAtt(3, 2, 4);    // circle marker
      //pic->Pic(0, 0)->SetFillAtt(3, 3001);
      pic->Pic(0, 1)->AddObject(hPosQLoopY);
      pic->Pic(0, 1)->SetDrawOption("LP");
      pic->Pic(0, 1)->SetLineAtt(4, 1, 2);
      pic->Pic(0, 1)->SetMarkerAtt(4, 2, 4);    // circle marker
      //pic->Pic(0, 1)->SetFillAtt(4, 3001);
      pic->Pic(1, 0)->AddObject(hPosQAccLoopX);
      pic->Pic(1, 0)->SetDrawOption("LP");
      pic->Pic(1, 0)->SetLineAtt(3, 1, 2);
      pic->Pic(1, 0)->SetMarkerAtt(3, 2, 4);    // circle marker
      pic->Pic(1, 0)->SetFillAtt(3, 3001);
      pic->Pic(1, 1)->AddObject(hPosQAccLoopY);
      pic->Pic(1, 1)->SetDrawOption("LP");
      pic->Pic(1, 1)->SetLineAtt(4, 1, 2);
      pic->Pic(1, 1)->SetMarkerAtt(4, 2, 4);    // circle marker
      pic->Pic(1, 1)->SetFillAtt(4, 3001);
      AddPicture(pic, foldername.Data());

    }


    hBeamChargeXSlice = MakeTH2('D', Form("%s/Charge_X_Time_G%d_L%d", foldername.Data(), grid, loop),
          Form("X Charge Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0, timeslices,
          "Wire", "Time Slice", "Q [C]");
    hBeamChargeYSlice = MakeTH2('D', Form("%s/Charge_Y_Time_G%d_L%d", foldername.Data(), grid, loop),
          Form("Y Charge Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0, timeslices,
          "Wire", "Time Slice", "Q [C]");

    hBeamAccChargeXSlice = MakeTH2('D', Form("%s/ChargeSum_X_Time_G%d_L%d", foldername.Data(), grid, loop),
              Form("X Accumulated Charge Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0, timeslices,
              "Wire", "Time Slice", "Q [C]");
    hBeamAccChargeYSlice = MakeTH2('D', Form("%s/ChargeSum_Y_Time_G%d_L%d", foldername.Data(), grid, loop),
              Form("Y Accumulated Charge Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0, timeslices,
              "Wire", "Time Slice", "Q [C]");





    // here trace current profile, maybe also average current accumulated?
    foldername.Form("Beam/Grid%2d/Current/Loop_%d", grid, loop);

    hPosILoopX = MakeVarbinsTH1(replace, 'D', Form("%s/I_Position_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Current profile Grid%2d Loop%2d", grid, loop), binsX, xposition, "X-Position [mm]", "I [A]");

    hPosIAveLoopX = MakeVarbinsTH1(replace, 'D', Form("%s/I_PositionAverage_X_G%d_L%d", foldername.Data(), grid, loop),
        Form("X  Average current profile accumulated Grid%2d Loop%2d", grid, loop), binsX, xposition, "X-Position [mm]",
        "I [A]");

    hPosILoopY = MakeVarbinsTH1(replace, 'D', Form("%s/I_Position_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Current profile Grid%2d Loop%2d", grid, loop), binsY, yposition, "Y-Position [mm]", "I [A]");

    hPosIAveLoopY = MakeVarbinsTH1(replace, 'D', Form("%s/I_PositionAverage_Y_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y  Average current profileCharge profile accumulated Grid%2d Loop%2d", grid, loop), binsY, yposition,
        "Y-Position [mm]", "I [A]");

    obname.Form("Beam Position Current Display Grid%2d Loop %2d", grid, loop);
    pic = GetPicture(obname.Data());
    if (pic == 0)
    {
      obtitle.Form("Overview beam current position projections Grid%2d Loop %2d", grid, loop);
      //foldername.Form("Beam/Grid%2d", grid); // use same picture subfolders as for histograms
      pic = new TGo4Picture(obname.Data(), obtitle.Data());
      pic->SetDivision(2, 2);
      pic->Pic(0, 0)->AddObject(hPosILoopX);
      pic->Pic(0, 0)->SetDrawOption("LP");
      pic->Pic(0, 0)->SetLineAtt(3, 1, 2);    // solid line
      pic->Pic(0, 0)->SetMarkerAtt(3, 2, 4);    // circle marker
      //pic->Pic(0, 0)->SetFillAtt(3, 3001);
      pic->Pic(0, 1)->AddObject(hPosILoopY);
      pic->Pic(0, 1)->SetDrawOption("LP");
      pic->Pic(0, 1)->SetLineAtt(4, 1, 2);
      pic->Pic(0, 1)->SetMarkerAtt(4, 2, 4);    // circle marker
      //pic->Pic(0, 1)->SetFillAtt(4, 3001);
      pic->Pic(1, 0)->AddObject(hPosIAveLoopX);
      pic->Pic(1, 0)->SetDrawOption("LP");
      pic->Pic(1, 0)->SetLineAtt(3, 1, 2);
      pic->Pic(1, 0)->SetMarkerAtt(3, 2, 4);    // circle marker
      pic->Pic(1, 0)->SetFillAtt(3, 3001);
      pic->Pic(1, 1)->AddObject(hPosIAveLoopY);
      pic->Pic(1, 1)->SetDrawOption("LP");
      pic->Pic(1, 1)->SetLineAtt(4, 1, 2);
      pic->Pic(1, 1)->SetMarkerAtt(4, 2, 4);    // circle marker
      pic->Pic(1, 1)->SetFillAtt(4, 3001);
      AddPicture(pic, foldername.Data());
    }

    hBeamCurrentXSlice = MakeTH2('D', Form("%s/Current_X_Time_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Current Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0,
        timeslices, "Wire", "Time Slice", "I [A]");
    hBeamCurrentYSlice = MakeTH2('D', Form("%s/Current_Y_Time_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Current Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0,
        timeslices, "Wire", "Time Slice", "I [A]");

    hBeamAveCurrentXSlice = MakeTH2('D', Form("%s/CurrentAverage_X_Time_G%d_L%d", foldername.Data(), grid, loop),
        Form("X Average Current Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsX, minX, maxX, timeslices, 0,
        timeslices, "Wire", "Time Slice", "I [A]");
    hBeamAveCurrentYSlice = MakeTH2('D', Form("%s/CurrentAverage_Y_Time_G%d_L%d", foldername.Data(), grid, loop),
        Form("Y Average Current Profile vs Time slices Grid%2d Loop%2d", grid, loop), binsY, minY, maxY, timeslices, 0,
        timeslices, "Wire", "Time Slice", "I [A]");



  }    //  if (fParam && (gix >= 0))
#endif


}

void TQFWGridLoopDisplay::AdjustDisplay(TQFWLoop* loopdata)
{
  // check if we have different timeslices:
  if (loopdata->fHasData && (loopdata->fQfwLoopSize != GetTimeSlices()))
  {
    //printf("TQFWGridLoopDisplay::AdjustDisplay with InitDisplay for loopdata %s with new timeslizes %d\n",
    //    loopdata->GetName(),
    //    loopdata->fQfwLoopSize);

    InitDisplay(loopdata->fQfwLoopSize, kTRUE);

  }

//  printf("TQFWGridLoopDisplay::AdjustDisplay for loopdata %s \n",
//       loopdata->GetName());


// change histogram titels according setup:
  //Double_t mtime = loopdata->fQfwLoopTime * 20 / 1000;    // measurement time in us
  Double_t mtime = loopdata->GetMicroSecsPerTimeSlice();
  /* evaluate measurement setup*/
  TString setup = loopdata->GetSetupString();
//      GetSetupString(loopdata->fQfwSetup);

// APPEND TIME RANGES:
  TString mtitle;
  mtitle.Form("%s dt=%.2E us", setup.Data(), mtime);

// clear trace histograms:

  hBeamLoopX->Reset("");
  hBeamLoopY->Reset("");

#ifdef       QFW_FILL_RMS_PROFILES
  hBeamLoopRMSX->Reset("");
  hBeamLoopRMSY->Reset("");
#endif
#ifdef  QFW_FILL_POSITION_PROFILES
  hPosLoopX->Reset("");
  hPosLoopY->Reset("");
  hPosQLoopX->Reset("");
  hPosQLoopY->Reset("");
  hPosILoopX->Reset("");
  hPosILoopY->Reset("");
  hPosIAveLoopX->Reset("");
  hPosIAveLoopY->Reset("");
#endif

  hBeamXSlice->Reset("");
  hBeamYSlice->Reset("");
  hBeamXSliceOffs->Reset("");
  hBeamYSliceOffs->Reset("");
  hBeamXSlice->SetTitle(mtitle.Data());
  hBeamYSlice->SetTitle(mtitle.Data());

  hBeamTimeX->Reset("");
  hBeamTimeY->Reset("");
  hBeamTimeX->SetTitle(mtitle.Data());
  hBeamTimeY->SetTitle(mtitle.Data());

#ifdef  QFW_FILL_POSITION_PROFILES
  hBeamChargeXSlice->Reset("");
  hBeamChargeYSlice->Reset("");
  hBeamCurrentXSlice->Reset("");
  hBeamCurrentYSlice->Reset("");
  hBeamAveCurrentXSlice->Reset("");
  hBeamAveCurrentYSlice->Reset("");
#endif

}

///*********************************************

TQFWGridDisplay::TQFWGridDisplay(Int_t gridid) :
    TQFWDisplay(gridid), hBeamX(0), hBeamY(0), hBeamAccX(0), hBeamAccY(0), hPosX(0), hPosY(0), hPosAccX(0), hPosAccY(0),
        hPosQ_X(0), hPosQ_Y(0), hPosQAcc_X(0), hPosQAcc_Y(0), hPosI_X(0), hPosI_Y(0), hPosIAve_X(0), hPosIAve_Y(0),
        pBeamProfiles(0), pPosProfiles(0), pChargeProfiles(0), pCurrentProfiles(0), hBeamMeanXY(0), hBeamRMSX(0),
        hBeamRMSY(0), hPosMeanXY(0), hPosRMSX(0), hPosRMSY(0), pBeamRMS(0), pPosRMS(0), fGridData(0), fParam(0)
{

  for (int i = 0; i < PEXOR_QFWLOOPS; ++i)
  {
    AddLoopDisplay(new TQFWGridLoopDisplay(gridid, i));
  }
  InitDisplay(PEXOR_QFWSLICES);
}

//***********************************************************
TQFWGridDisplay::~TQFWGridDisplay()
{

}

TQFWGridLoopDisplay* TQFWGridDisplay::GetLoopDisplay(Int_t index)
{
  return (TQFWGridLoopDisplay*) GetSubDisplay(index);
}

void TQFWGridDisplay::SetGridEvent(TQFWProfileEvent* out)
{
  fGridData = out->GetGrid(GetDevId());
  for (unsigned i = 0; i < fLoops.size(); ++i)
  {
    TQFWGridLoopDisplay* cuploop = dynamic_cast<TQFWGridLoopDisplay*>(fLoops[i]);
    cuploop->SetGridEvent(out);
  }

}

void TQFWGridDisplay::SetProfileParam(TQFWProfileParam* par)
{
  fParam = par;
  for (unsigned i = 0; i < fLoops.size(); ++i)
  {
    TQFWGridLoopDisplay* loop = dynamic_cast<TQFWGridLoopDisplay*>(fLoops[i]);
    loop->SetProfileParam(par);
  }
}

void TQFWGridDisplay::InitDisplay(int timeslices, Bool_t replace)
{
  TQFWDisplay::InitDisplay(timeslices, replace);    // handle initialization of subloops
  if (replace)
    SetMakeWithAutosave(kFALSE);
  TString obname;
  TString obtitle;
  TString foldername;
  Int_t grid = GetDevId();
  Int_t wiresX = PEXOR_QFW_WIRES;
  Int_t wiresY = PEXOR_QFW_WIRES;

  if (fGridData)
  {
    // take real number of wires from event object
    wiresX = fGridData->GetNumXWires();
    wiresY = fGridData->GetNumYWires();
  }

// here evaluate visible range from parameter
// note that first existing wire has index 0, last wire index (wiresX-1)
// means maxX/maxY is always upper excluded boundary
  Int_t minX = 0;
  Int_t maxX = wiresX;
  Int_t minY = 0;
  Int_t maxY = wiresY;
  Int_t gix = -1;
  if (fParam)
  {
    gix = fParam->FindGridIndex(grid);
    if (gix >= 0)
    {
      //printf("GGGGGG Using index %d of grid uid%d \n", gix, grid);
      if (fParam->fGridMinWire_X >= 0)
        minX = fParam->fGridMinWire_X[gix];
      if (fParam->fGridMaxWire_X[gix] >= 0)
        maxX = fParam->fGridMaxWire_X[gix];
      if (maxX > wiresX)
        maxX = wiresX;
      if (minX > maxX)
        minX = maxX;

      if (fParam->fGridMinWire_Y[gix] >= 0)
        minY = fParam->fGridMinWire_Y[gix];
      if (fParam->fGridMaxWire_Y[gix] >= 0)
        maxY = fParam->fGridMaxWire_Y[gix];
      if (maxY > wiresY)
        maxY = wiresY;
      if (minY > maxY)
        minX = maxX;
    }
  }

  Int_t binsX = maxX - minX;
  Int_t binsY = maxY - minY;

  /* xy beam display*/

  foldername.Form("Beam/Grid%2d/Raw", grid);
  hBeamX = MakeTH1('D', Form("%s/Profile_X_G%d", foldername.Data(), grid), Form("X Profile Grid%2d", grid), binsX, minX,
      maxX, "Wire");
  hBeamY = MakeTH1('D', Form("%s/Profile_Y_G%d", foldername.Data(), grid), Form("Y Profile Grid%2d", grid), binsY, minY,
      maxY, "Wire");

  hBeamAccX = MakeTH1('D', Form("%s/ProfileSum_X_G%d", foldername.Data(), grid),
      Form("X Profile accumulated Grid%2d", grid), binsX, minX, maxX, "Wire");
  hBeamAccY = MakeTH1('D', Form("%s/ProfileSum_Y_G%d", foldername.Data(), grid),
      Form("Y Profile accumulated Grid%2d", grid), binsY, minY, maxY, "Wire");

  obname.Form("Beam Display Grid%2d", grid);
  pBeamProfiles = GetPicture(obname.Data());
  if (pBeamProfiles == 0)
  {
    obtitle.Form("Overview beam projections Grid%2d", grid);
    //foldername.Form("Beam/Grid%2d", grid);
    pBeamProfiles = new TGo4Picture(obname.Data(), obtitle.Data());
    pBeamProfiles->SetDivision(2, 2);
    pBeamProfiles->Pic(0, 0)->AddObject(hBeamX);
    pBeamProfiles->Pic(0, 0)->SetFillAtt(3, 3001);
    pBeamProfiles->Pic(0, 1)->AddObject(hBeamY);
    pBeamProfiles->Pic(0, 1)->SetFillAtt(4, 3001);
    pBeamProfiles->Pic(1, 0)->AddObject(hBeamAccX);
    pBeamProfiles->Pic(1, 0)->SetFillAtt(3, 3001);
    pBeamProfiles->Pic(1, 1)->AddObject(hBeamAccY);
    pBeamProfiles->Pic(1, 1)->SetFillAtt(4, 3001);
    AddPicture(pBeamProfiles, foldername.Data());

  }

  hBeamMeanXY = MakeTH2('I', Form("%s/Meanpos_G%d", foldername.Data(), grid),
      Form("Beam Mean position scaler Grid%2d", grid), binsX, minX, maxX, binsY, minY, maxY, "X", "Y");

  hBeamRMSX = MakeTH1('I', Form("%s/RMS_X_G%d", foldername.Data(), grid), Form("X Profile RMS Grid%2d", grid),
      10 * binsX, minX, maxX, "Wire spacings");

  hBeamRMSY = MakeTH1('I', Form("%s/RMS_Y_G%d", foldername.Data(), grid), Form("Y Profile RMS Grid%2d", grid),
      10 * binsY, minY, maxY, "Wire spacings");

  obname.Form("Beam RMS Grid%2d", grid);
  pBeamRMS = GetPicture(obname.Data());
  if (pBeamRMS == 0)
  {
    obtitle.Form("Beam RMS distribution Grid%2d", grid);
    //foldername.Form("Beam/Grid%2d", grid);
    pBeamRMS = new TGo4Picture(obname.Data(), obtitle.Data());
    pBeamRMS->SetDivision(1, 2);
    pBeamRMS->Pic(0, 0)->AddObject(hBeamRMSX);
    pBeamRMS->Pic(0, 0)->SetFillAtt(3, 3002);
    pBeamRMS->Pic(0, 1)->AddObject(hBeamRMSY);
    pBeamRMS->Pic(0, 1)->SetFillAtt(4, 3002);
    AddPicture(pBeamRMS, foldername.Data());

  }

#ifdef  QFW_FILL_POSITION_PROFILES

  // position histogram with non equidistant bins
  if (fParam && gix >= 0)
  {
    // these only make sense with position map from parameter is there
    Double_t xposition[PEXOR_QFW_WIRES];
    for (Int_t ix = 0, jx = minX; (ix < binsX) && (jx + 1 < PEXOR_QFW_WIRES); ++ix, ++jx)
    {
      Double_t binwidth = fParam->fGridPosition_X[gix][jx + 1] - fParam->fGridPosition_X[gix][jx];
      if (ix == 0)
        xposition[ix] = fParam->fGridPosition_X[gix][jx] - binwidth / 2;
      xposition[ix + 1] = fParam->fGridPosition_X[gix][jx] + binwidth / 2;
      //printf("QFWQFWQFW grid %d index i=%d: j=%d: lowedge:%f centre:%f upedge:%f \n",grid, ix, jx, xposition[ix], fParam->fGridPosition_X[gix][jx], xposition[ix+1]);
    }

    foldername.Form("Beam/Grid%2d/Counts", grid);

    hPosX = MakeVarbinsTH1(replace, 'I', Form("%s/Position_X_G%d", foldername.Data(), grid),
        Form("X Position Grid%2d", grid), binsX, xposition, "X-Position [mm]");

    hPosAccX = MakeVarbinsTH1(replace, 'I', Form("%s/PositionSum_X_G%d", foldername.Data(), grid),
        Form("X Position accumulated Grid%2d", grid), binsX, xposition, "X-Position [mm]");

    Double_t yposition[PEXOR_QFW_WIRES];
    for (Int_t ix = 0, jx = minY; (ix < binsY) && (jx + 1 < PEXOR_QFW_WIRES); ++ix, ++jx)
    {
      Double_t binwidth = fParam->fGridPosition_Y[gix][jx + 1] - fParam->fGridPosition_Y[gix][jx];
      if (ix == 0)
        yposition[ix] = fParam->fGridPosition_Y[gix][jx] - binwidth / 2;
      yposition[ix + 1] = fParam->fGridPosition_Y[gix][jx] + binwidth / 2;
      //printf("YYYYYYYY grid %d index %d:lowedge:%f centre:%f upedge:%f \n",grid,ix,yposition[ix], fParam->fGridPosition_Y[gix][jx], yposition[ix+1]);
    }
    hPosY = MakeVarbinsTH1(replace, 'I', Form("%s/Position_Y_G%d", foldername.Data(), grid),
        Form("Y Position Grid%2d", grid), binsY, yposition, "Y-Position [mm]");
    hPosAccY = MakeVarbinsTH1(replace, 'I', Form("%s/PositionSum_Y_G%d", foldername.Data(), grid),
        Form("Y Position accumulated Grid%2d", grid), binsY, yposition, "Y-Position [mm]");

    obname.Form("Beam Position Display Grid%2d", grid);
    pPosProfiles = GetPicture(obname.Data());
    if (pPosProfiles == 0)
    {
      obtitle.Form("Overview beam position projections Grid%2d", grid);
      //foldername.Form("Beam/Grid%2d", grid); // use same picture subfolders as for histograms
      pPosProfiles = new TGo4Picture(obname.Data(), obtitle.Data());
      pPosProfiles->SetDivision(2, 2);
      pPosProfiles->Pic(0, 0)->AddObject(hPosX);
      pPosProfiles->Pic(0, 0)->SetFillAtt(3, 3001);
      pPosProfiles->Pic(0, 1)->AddObject(hPosY);
      pPosProfiles->Pic(0, 1)->SetFillAtt(4, 3001);
      pPosProfiles->Pic(1, 0)->AddObject(hPosAccX);
      pPosProfiles->Pic(1, 0)->SetFillAtt(3, 3001);
      pPosProfiles->Pic(1, 1)->AddObject(hPosAccY);
      pPosProfiles->Pic(1, 1)->SetFillAtt(4, 3001);
      AddPicture(pPosProfiles, foldername.Data());

    }

    // put 2d mean positions hBeamMeanXY#
    Double_t minX_mm = fParam->fGridPosition_X[gix][minX];
    Double_t maxX_mm = fParam->fGridPosition_X[gix][maxX];
    Double_t minY_mm = fParam->fGridPosition_Y[gix][minY];
    Double_t maxY_mm = fParam->fGridPosition_Y[gix][maxY];

    hPosMeanXY = MakeTH2('I', Form("%s/MeanposMM_G%d", foldername.Data(), grid),
        Form("Beam Mean position [mm] Grid%2d", grid), 10 * binsX, minX_mm, maxX_mm, 10 * binsY, minY_mm, maxY_mm,
        "X [mm]", "Y[mm]");

    hPosRMSX = MakeTH1('I', Form("%s/RMSMM_X_G%d", foldername.Data(), grid), Form("X Profile RMS [mm] Grid%2d", grid),
        10 * binsX, minX_mm, maxX_mm, "mm");

    hPosRMSY = MakeTH1('I', Form("%s/RMSMM_Y_G%d", foldername.Data(), grid), Form("Y Profile RMS [mm]Grid%2d", grid),
        10 * binsY, minY_mm, maxY_mm, "mm");

    obname.Form("Beam RMS [mm] Grid%2d", grid);
    pPosRMS = GetPicture(obname.Data());
    if (pPosRMS == 0)
    {
      obtitle.Form("Beam RMS distribution [mm] Grid%2d", grid);
      //foldername.Form("Beam/Grid%2d", grid); // same as for histograms
      pPosRMS = new TGo4Picture(obname.Data(), obtitle.Data());
      pPosRMS->SetDivision(1, 2);
      pPosRMS->Pic(0, 0)->AddObject(hPosRMSX);
      pPosRMS->Pic(0, 0)->SetFillAtt(3, 3002);
      pPosRMS->Pic(0, 1)->AddObject(hPosRMSY);
      pPosRMS->Pic(0, 1)->SetFillAtt(4, 3002);
      AddPicture(pPosRMS, foldername.Data());

    }

    // TODO here profiles for summed up charges of all loops
    foldername.Form("Beam/Grid%2d/Charge", grid);
    hPosQ_X = MakeVarbinsTH1(replace, 'D', Form("%s/Q_Position_X_G%d", foldername.Data(), grid),
        Form("X Charge profile Grid%2d", grid), binsX, xposition, "X-Position [mm]", "Q [C]");

    hPosQAcc_X = MakeVarbinsTH1(replace, 'D', Form("%s/Q_PositionSum_X_G%d", foldername.Data(), grid),
        Form("X  Charge profile accumulated Grid%2d", grid), binsX, xposition, "X-Position [mm]", "Q [C]");

    hPosQ_Y = MakeVarbinsTH1(replace, 'D', Form("%s/Q_Position_Y_G%d", foldername.Data(), grid),
        Form("Y Charge profile Grid%2d ", grid), binsY, yposition, "Y-Position [mm]", "Q [C]");

    hPosQAcc_Y = MakeVarbinsTH1(replace, 'D', Form("%s/Q_PositionSum_Y_G%d", foldername.Data(), grid),
        Form("Y  Charge profile accumulated Grid%2d", grid), binsY, yposition, "Y-Position [mm]", "Q [C]");

    obname.Form("Beam Position Charge Display Grid%2d", grid);
    pChargeProfiles = GetPicture(obname.Data());
    if (pChargeProfiles == 0)
    {
      obtitle.Form("Overview beam charge position projections Grid%2d", grid);
      //foldername.Form("Beam/Grid%2d", grid); // use same picture subfolders as for histograms
      pChargeProfiles = new TGo4Picture(obname.Data(), obtitle.Data());
      pChargeProfiles->SetDivision(2, 2);
      pChargeProfiles->Pic(0, 0)->AddObject(hPosQ_X);
      pChargeProfiles->Pic(0, 0)->SetDrawOption("LP");
      pChargeProfiles->Pic(0, 0)->SetLineAtt(3, 1, 2);    // solid line
      pChargeProfiles->Pic(0, 0)->SetMarkerAtt(3, 2, 4);    // circle marker
      //pChargeProfiles->Pic(0, 0)->SetFillAtt(3, 3001);
      pChargeProfiles->Pic(0, 1)->AddObject(hPosQ_Y);
      pChargeProfiles->Pic(0, 1)->SetDrawOption("LP");
      pChargeProfiles->Pic(0, 1)->SetLineAtt(4, 1, 2);
      pChargeProfiles->Pic(0, 1)->SetMarkerAtt(4, 2, 4);    // circle marker
      //pChargeProfiles->Pic(0, 1)->SetFillAtt(4, 3001);
      pChargeProfiles->Pic(1, 0)->AddObject(hPosQAcc_X);
      pChargeProfiles->Pic(1, 0)->SetDrawOption("LP");
      pChargeProfiles->Pic(1, 0)->SetLineAtt(3, 1, 2);
      pChargeProfiles->Pic(1, 0)->SetMarkerAtt(3, 2, 4);    // circle marker
      pChargeProfiles->Pic(1, 0)->SetFillAtt(3, 3001);
      pChargeProfiles->Pic(1, 1)->AddObject(hPosQAcc_Y);
      pChargeProfiles->Pic(1, 1)->SetDrawOption("LP");
      pChargeProfiles->Pic(1, 1)->SetLineAtt(4, 1, 2);
      pChargeProfiles->Pic(1, 1)->SetMarkerAtt(4, 2, 4);    // circle marker
      pChargeProfiles->Pic(1, 1)->SetFillAtt(4, 3001);
      AddPicture(pChargeProfiles, foldername.Data());

    }

    // TODO here profiles for summed up currents of all loops maybe also average of loop averages
    foldername.Form("Beam/Grid%2d/Current", grid);
//    hPosI_X = MakeVarbinsTH1(replace, 'D', Form("%s/I_Position_X_G%d", foldername.Data(), grid),
//        Form("X Current profile Grid%2d", grid), binsX, xposition, "Position [mm]", "I [A]");
//
//    hPosIAve_X = MakeVarbinsTH1(replace, 'D', Form("%s/I_PositionAverage_X_G%d", foldername.Data(), grid),
//        Form("X Current profile average Grid%2d", grid), binsX, xposition, "Position [mm]", "I [A]");
//
//    hPosI_Y = MakeVarbinsTH1(replace, 'D', Form("%s/I_Position_Y_G%d", foldername.Data(), grid),
//        Form("Y Current profile Grid%2d ", grid), binsY, yposition, "Position [mm]", "I [A]");
//
//    hPosIAve_Y = MakeVarbinsTH1(replace, 'D', Form("%s/I_PositionAverage_Y_G%d", foldername.Data(), grid),
//        Form("Y Current profile average Grid%2d", grid), binsY, yposition, "Position [mm]", "I [A]");

  }    // if (fParam && gix >= 0)
#endif

}

void TQFWGridDisplay::AdjustDisplay(TQFWBoard* boarddata)
{

//   Bool_t dostop=kFALSE;
  //Double_t mtime=boarddata->fQfwLoopTime * 20 / 1000; // measurement time in us

  /* evaluate measurement setup*/
  TString setup = boarddata->GetSetupString();

  // APPEND TIME RANGES:
  TString mtitle;
  mtitle.Form("%s", setup.Data());
  hBeamX->Reset("");
  hBeamY->Reset("");
  hBeamX->SetTitle(mtitle.Data());
  hBeamY->SetTitle(mtitle.Data());
#ifdef   QFW_FILL_POSITION_PROFILES
  hPosX->Reset("");
  hPosY->Reset("");
  hPosX->SetTitle(mtitle.Data());
  hPosY->SetTitle(mtitle.Data());
  hPosQ_X->Reset("");
  hPosQ_Y->Reset("");
//  hPosI_X->Reset("");
//  hPosI_Y->Reset("");
//  hPosIAve_X->Reset("");
//  hPosIAve_Y->Reset("");

#endif

//      mtitle.Form("%s dt=%.2E us", setup.Data(),premtime);

}

TQFWCupLoopDisplay::TQFWCupLoopDisplay(Int_t cupid, Int_t loopid) :
    TQFWLoopDisplay(cupid, loopid), fCupData(0), hCupSlice(0), hCupSliceOffs(0), hAccCupSlice(0),
    hCupChargeSlice(0), hCupAccChargeSlice(0), hCupCurrentSlice(0), hCupAveCurrentSlice(0), hCupLoopScaler(0),
        hCupAccLoopScaler(0), hCupLoopCharge(0), hCupAccLoopCharge(0),hCupLoopCurrent(0), hCupAveLoopCurrent(0)

{

}
TQFWCupLoopDisplay::~TQFWCupLoopDisplay()
{

}
void TQFWCupLoopDisplay::SetCupEvent(TQFWProfileEvent* out)
{
  fCupData = out->GetCup(GetDevId());

}

void TQFWCupLoopDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
// TODO: put some scaler histograms here

  TGo4Log::Info("TQFWCupLoopDisplay: Initializing histograms with %d timeslices for Cup %d Loop %d", timeslices,
      GetDevId(), GetLoopId());
  if (replace)    //TGo4Analysis::Instance()->
    SetMakeWithAutosave(kFALSE);
  TQFWLoopDisplay::InitDisplay(timeslices, replace);    // important to remember new timeslices!
  Int_t cup = GetDevId();
  Int_t loop = GetLoopId();
  Int_t segments = PEXOR_QFW_CUPSEGMENTS;
  TString foldername;
  if (fCupData)
  {
    // take real number of segments from event object
    segments = fCupData->GetNumSegments();
  }

  foldername.Form("Beam/Cup%2d/Raw/Loop%2d/", cup,loop);

  hCupSlice = MakeTH2('D', Form("%s/Scaler_Time_C%d_L%d", foldername.Data(), cup, loop),
      Form("Segment Scaler vs Time slices Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices, 0, timeslices,
      "Segment", "Time Slice");

  /* helper histogram showing current offset*/
  hCupSliceOffs = MakeTH2('D', Form("%s/Scaler_Time_Offset_C%d_L%d", foldername.Data(), cup, loop),
      Form("Segment Scaler vs Time slices average offset Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices,
      0, timeslices, "Segment", "Time Slice");

  hAccCupSlice = MakeTH2('D', Form("%s/ScalerSum_Time_C%d_L%d", foldername.Data(), cup, loop),
      Form("Segment Scaler vs Time slices accum Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices, 0,
      timeslices, "Segment", "Time Slice");

  hCupLoopScaler = MakeTH1('I', Form("%s/Scaler_C%d_L%d", foldername.Data(), cup, loop),
      Form("Segment scaler Cup%2d Loop%2d", cup, loop), segments, 0, segments, "Segment", "Counts");

  hCupAccLoopScaler = MakeTH1('I', Form("%s/ScalerSum_C%d_L%d", foldername.Data(), cup, loop),
      Form("Segment scaler accumulated Cup%2d Loop%2d", cup, loop), segments, 0, segments, "Segment", "Counts");


  foldername.Form("Beam/Cup%2d/Charge/Loop%2d/", cup,loop);

  hCupLoopCharge = MakeTH1('D', Form("%s/Charge_C%d_L%d", foldername.Data(), cup, loop),
       Form("Segment charge Cup%2d Loop%2d", cup, loop), segments, 0, segments, "Segment", "Q [C]");

  hCupAccLoopCharge = MakeTH1('D', Form("%s/ChargeSum_C%d_L%d", foldername.Data(), cup, loop),
        Form("Segment charge accumulated Cup%2d Loop%2d", cup, loop), segments, 0, segments, "Segment", "Q [C]");

  hCupChargeSlice = MakeTH2('D', Form("%s/Charge_Time_C%d_L%d", foldername.Data(), cup, loop),
         Form("Segment Charge vs Time slices Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices, 0, timeslices,
         "Segment", "Time Slice", "Q [C]");

  hCupAccChargeSlice = MakeTH2('D', Form("%s/ChargeSum_Time_C%d_L%d", foldername.Data(), cup, loop),
        Form("Segment Charge Sum vs Time slices accum Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices, 0,
        timeslices, "Segment", "Time Slice", "Q [C]");


  hSegmentRatio.clear();    // we do not delete histograms here, is handled by framework when recreating
  hAccSegmentRatio.clear();
  for (int seg = 0; seg < segments; ++seg)
  {
    hSegmentRatio.push_back(
        MakeTH1('D', Form("%s/SegmentRatio_C%d_L%d_S%d", foldername.Data(), cup, loop, seg),
            Form("Charge Ratio Cup%2d Loop%2d Segment %2d", cup, loop, seg), 500, 0, 1, "Relative Charge"));

    hAccSegmentRatio.push_back(
        MakeTH1('D', Form("%s/SegmentRatioSum_C%d_L%d_S%d", foldername.Data(), cup, loop, seg),
            Form("Charge Ratio Cup%2d Loop%2d Segment %2d Accumulated", cup, loop, seg), 500, 0, 1, "Relative Charge"));
  }

  foldername.Form("Beam/Cup%2d/Current/Loop%2d/", cup,loop);

  hCupLoopCurrent = MakeTH1('D', Form("%s/Current_C%d_L%d", foldername.Data(), cup, loop),
       Form("Segment current Cup%2d Loop%2d", cup, loop), segments, 0, segments, "Segment", "I [A]");

  hCupAveLoopCurrent = MakeTH1('D', Form("%s/CurrentAverage_C%d_L%d", foldername.Data(), cup, loop),
        Form("Segment current average Cup%2d Loop%2d", cup, loop), segments, 0, segments, "Segment", "I [A]");


  hCupCurrentSlice = MakeTH2('D', Form("%s/Current_Time_C%d_L%d", foldername.Data(), cup, loop),
        Form("Segment Current vs Time slices Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices, 0, timeslices,
        "Segment", "Time Slice", "I [A]");

  hCupAveCurrentSlice = MakeTH2('D', Form("%s/CurrentAverage_Time_C%d_L%d", foldername.Data(), cup, loop),
       Form("Segment Average Current vs Time slices accum Cup%2d Loop%2d", cup, loop), segments, 0, segments, timeslices, 0,
       timeslices, "Segment", "Time Slice", "I [A]");

}

void TQFWCupLoopDisplay::AdjustDisplay(TQFWLoop* loopdata)
{

  // check if we have different timeslices:

  if (loopdata->fQfwLoopSize != GetTimeSlices())
  {
    InitDisplay(loopdata->fQfwLoopSize, kTRUE);
  }

// change histogram titels according setup:

  Double_t mtime = loopdata->fQfwLoopTime * 20 / 1000;    // measurement time in us

  /* evaluate measurement setup*/
  TString setup = loopdata->GetSetupString();

  // APPEND TIME RANGES:
  TString mtitle;
  mtitle.Form("%s dt=%.2E us", setup.Data(), mtime);

// clear trace histograms:
  hCupSlice->Reset("");
  hCupSliceOffs->Reset("");
  hCupChargeSlice->Reset("");
  hCupCurrentSlice->Reset("");
  hCupAveCurrentSlice->Reset("");
  hCupLoopScaler->Reset("");
  hCupLoopCharge->Reset("");
  hCupLoopCurrent->Reset("");
  hCupAveLoopCurrent->Reset("");

  hCupSlice->SetTitle(mtitle.Data());
  for (unsigned seg = 0; seg < hSegmentRatio.size(); ++seg)
  {
    hSegmentRatio[seg]->Reset("");
  }

}

/////////////////////////////////////////////////////////////////////////////7

TQFWCupDisplay::TQFWCupDisplay(Int_t cupid) :
    TQFWDisplay(cupid), hCupScaler(0), hCupAccScaler(0), fCupData(0)
{
  for (unsigned i = 0; i < PEXOR_QFWLOOPS; ++i)
  {
    AddLoopDisplay(new TQFWCupLoopDisplay(cupid, i));
  }
  InitDisplay(PEXOR_QFWSLICES);
}
TQFWCupDisplay::~TQFWCupDisplay()
{

}
void TQFWCupDisplay::SetCupEvent(TQFWProfileEvent* out)
{
  fCupData = out->GetCup(GetDevId());
  for (unsigned i = 0; i < fLoops.size(); ++i)
  {
    TQFWCupLoopDisplay* cuploop = dynamic_cast<TQFWCupLoopDisplay*>(fLoops[i]);
    cuploop->SetCupEvent(out);
  }

}
TQFWCupLoopDisplay* TQFWCupDisplay::GetLoopDisplay(Int_t index)
{
  return (TQFWCupLoopDisplay*) GetSubDisplay(index);
}

void TQFWCupDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
  TQFWDisplay::InitDisplay(timeslices, replace);
// TODO: put some scaler histograms here
TString foldername;
  Int_t cup = GetDevId();
  Int_t segs = PEXOR_QFW_CUPSEGMENTS;
  if (fCupData)
  {
    // take real number of segments from event object
    segs = fCupData->GetNumSegments();
  }

  foldername.Form("Beam/Cup%2d/Raw", cup);

  hCupScaler = MakeTH1('I', Form("%s/Scaler_C%d", foldername.Data(), cup), Form("Segment scaler Cup%2d", cup), segs, 0, segs,
      "Segment");

  hCupAccScaler = MakeTH1('I', Form("%s/ScalerSum_C%d", foldername.Data(), cup),
      Form("Segment scaler accumulated Cup%2d", cup), segs, 0, segs, "Segment");

}

void TQFWCupDisplay::AdjustDisplay(TQFWBoard* boarddata)
{

//   Bool_t dostop=kFALSE;
  //Double_t mtime=boarddata->fQfwLoopTime * 20 / 1000; // measurement time in us

  /* evaluate measurement setup*/
  TString setup = boarddata->GetSetupString();
  // APPEND TIME RANGES:
  TString mtitle;
//      mtitle.Form("%s dt=%.2E us", setup.Data(),mtime);
  mtitle.Form("%s", setup.Data());

  hCupScaler->Reset("");
  hCupScaler->SetTitle(mtitle.Data());

}

