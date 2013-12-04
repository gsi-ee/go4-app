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

////////////////////////////////////////////////

TQFWLoopDisplay::TQFWLoopDisplay(Int_t deviceid, Int_t loopid) :
    TQFWDisplay(deviceid), fLoopId(loopid),fTimeSlices(0)
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
    TQFWLoopDisplay(boardid, loopid)
{
  //TString loopname.Form("Board%02d/Loop%02d_", boardid, loopid);
  InitDisplay(PEXOR_QFWSLICES);

}

void TQFWBoardLoopDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
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

  obname.Form("Board%d/Brd%d-Loop%d-Sum", brd, brd, loop);
  obtitle.Form("QFW Board %d Loop %d Accumulated", brd, loop);
  int qbins = timeslices * PEXOR_QFWCHANS;
  hQFWRaw = MakeTH1('I', obname.Data(), obtitle.Data(), qbins, 0, qbins, "QFW scaler sums");

  obname.Form("Board%d/Brd%d-Loop%d-Trace", brd, brd, loop);
  obtitle.Form("QFW Board %d Loop %d Trace", brd, loop);
  hQFWRawTrace = MakeTH1('I', obname.Data(), obtitle.Data(), qbins, 0, qbins, "QFW scalers");

  for (int sl = 0; sl < timeslices; ++sl)
    for (int ch = 0; ch < PEXOR_QFWCHANS; ++ch)
    {
      binlabel.Form("SL%d-%d", sl, ch);
      hQFWRaw->GetXaxis()->SetBinLabel(1 + sl * PEXOR_QFWCHANS + ch, binlabel.Data());
      hQFWRawTrace->GetXaxis()->SetBinLabel(1 + sl * PEXOR_QFWCHANS + ch, binlabel.Data());
    }

}

//***********************************************************

TQFWBoardDisplay::TQFWBoardDisplay(Int_t boardid) :
    TQFWDisplay(boardid)
{
  //TString boardname.Form("Board%02d", boardid);
  for (int i = 0; i < PEXOR_QFWLOOPS; ++i)
  {
    AddLoopDisplay(new TQFWBoardLoopDisplay(boardid, i));
  }

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
  TQFWDisplay::InitDisplay(timeslices, replace);    // handle initialization of subloops
  TString obname;
  TString obtitle;

  if (replace)
    SetMakeWithAutosave(kFALSE);
  Int_t brd = GetDevId();

  hQFWRaw2D = MakeTH2('I', Form("Board%d/Brd%d-2D-Sum", brd, brd), Form("QFW Board %d accumulated for all loops", brd),
      PEXOR_QFWLOOPS * timeslices, 0, PEXOR_QFWLOOPS * timeslices, PEXOR_QFWCHANS, 0, PEXOR_QFWCHANS, "loop", "ch");

  hQFWRaw2DTrace = MakeTH2('I', Form("Board%d/Brd%d-2D-Trace", brd, brd), Form("QFW Board %d trace for all loops", brd),
      PEXOR_QFWLOOPS * timeslices, 0, PEXOR_QFWLOOPS * timeslices, PEXOR_QFWCHANS, 0, PEXOR_QFWCHANS, "loop", "ch");

  hQFWRawErr = MakeTH1('I', Form("Board%d/Brd%dErr", brd, brd), Form("QFW errors in board %d", brd), PEXOR_QFWNUM, 0,
      PEXOR_QFWNUM);

  hQFWRawErrTr = MakeTH1('I', Form("Board%d/Brd%dErrTr", brd, brd), Form("QFW errors in board %d (snap shot)", brd),
      PEXOR_QFWNUM, 0, PEXOR_QFWNUM);

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
  TQFWLoopDisplay(gridid, loopid),fGridData(0)
{
InitDisplay(PEXOR_QFWSLICES);
}

//***********************************************************
TQFWGridLoopDisplay::~TQFWGridLoopDisplay()
{

}
void TQFWGridLoopDisplay::SetGridEvent(TQFWProfileEvent* out)
{
  fGridData=out->GetGrid(GetDevId());

}

void TQFWGridLoopDisplay::InitDisplay(int timeslices, Bool_t replace)
{
TGo4Log::Info("TQFWGridLoopDisplay: Initializing histograms with %d timeslices for Grid %d Loop %d", timeslices,
    GetDevId(), GetLoopId());
TString obname;
TString obtitle;
if (replace)    //TGo4Analysis::Instance()->
  SetMakeWithAutosave(kFALSE);

Int_t grid = GetDevId();
Int_t loop = GetLoopId();
Int_t wiresX=PEXOR_QFW_WIRES;
Int_t wiresY=PEXOR_QFW_WIRES;
if(fGridData)
{
  // take real number of wires from event object
    wiresX=fGridData->GetNumXWires();
    wiresY=fGridData->GetNumYWires();
}

/* xy beam display*/
obname.Form("Beam/GridAccu");
obtitle.Form("Beam grid current accumulate");

hBeamXSlice = MakeTH2('I', Form("Beam/Grid%2d/Profile_X_Time_G%d_L%d", grid, grid, loop),
    Form("X Profile vs Time slices Grid%2d Loop%2d", grid, loop), wiresX, 0, wiresX, timeslices, 0,
    timeslices, "Wire", "Time Slice");
hBeamYSlice = MakeTH2('I', Form("Beam/Grid%2d/Profile_Y_Time_G%d_L%d", grid, grid, loop),
    Form("Y Profile vs Time slices Grid%2d Loop%2d", grid, loop), wiresY, 0, wiresY, timeslices, 0,
    timeslices, "Wire", "Time Slice");

hBeamXSliceOffs = MakeTH2('I', Form("Beam/Grid%2d/Profile_X_Time_Offset_G%d_L%d", grid, grid, loop),
    Form("X Profile vs Time slices average offset Grid%2d Loop%2d", grid, loop), wiresX, 0, wiresX,
    timeslices, 0, timeslices, "Wire", "Time Slice");
hBeamYSliceOffs = MakeTH2('I', Form("Beam/Grid%2d/Profile_Y_Time_Offset_G%d_L%d", grid, grid, loop),
    Form("Y Profile vs Time slices average offset Grid%2d Loop%2d", grid, loop), wiresY, 0, wiresY,
    timeslices, 0, timeslices, "Wire", "Time Slice");

hBeamAccXSlice = MakeTH2('I', Form("Beam/Grid%2d/ProfileSum_X_Time_G%d_L%d", grid, grid, loop),
    Form("X Profile vs Time slices accum Grid%2d Loop%2d", grid, loop), wiresX, 0, wiresX, timeslices, 0,
    timeslices, "Wire", "Time Slice");
hBeamAccYSlice = MakeTH2('I', Form("Beam/Grid%2d/ProfileSum_Y_Time_G%d_L%d", grid, grid, loop),
    Form("Y Profile vs Time slices accum Grid%2d Loop%2d", grid, loop), wiresY, 0, wiresY, timeslices, 0,
    timeslices, "Wire", "Time Slice");

/* mean count of beam profile part: */

cBeamXSliceCond = MakeWinCond(Form("Beam/Grid%2d/XSliceCond_G%d_L%d", grid, grid,loop), 0, wiresX, 0, timeslices,
    hBeamXSlice->GetName());
cBeamYSliceCond = MakeWinCond(Form("Beam/Grid%2d/YSliceCond_G%d_L%d", grid, grid,loop), 0, wiresY, 0, timeslices,
    hBeamYSlice->GetName());

hBeamMeanCountsX = MakeTH1('I', Form("Beam/Grid%2d/Mean_Counts_X_G%d_L%d", grid, grid,loop), Form("X Counts Mean Grid%2d Loop%2d", grid,loop),
    1000, 0, 100, "Mean counts");

hBeamMeanCountsY = MakeTH1('I', Form("Beam/Grid%2d/Mean_Counts_Y_G%d_L%d", grid, grid,loop), Form("Y Counts Mean Grid%2d Loop%2d", grid,loop),
    1000, 0, 100, "Mean counts");

hBeamRMSCountsX = MakeTH1('I', Form("Beam/Grid%2d/RMS_Counts_X_G%d_L%d", grid, grid,loop), Form("X Counts RMS Grid%2d Loop%2d", grid,loop),
    1000, 0, 100, "RMS counts");

hBeamRMSCountsY = MakeTH1('I', Form("Beam/Grid%2d/RMS_Counts_Y_G%d_L%d", grid, grid,loop), Form("Y Counts RMS Grid%2d Loop%2d", grid,loop),
    1000, 0, 100, "RMS counts");




}

///*********************************************

TQFWGridDisplay::TQFWGridDisplay(Int_t gridid) :
  TQFWDisplay(gridid),fGridData(0)
{

for (int i = 0; i < PEXOR_QFWLOOPS; ++i)
{
  AddLoopDisplay(new TQFWGridLoopDisplay(gridid, i));
}

}

//***********************************************************
TQFWGridDisplay::~TQFWGridDisplay()
{

}

void TQFWGridDisplay::SetGridEvent(TQFWProfileEvent* out)
{
  fGridData=out->GetGrid(GetDevId());
  for (unsigned i = 0; i < fLoops.size(); ++i)
    {TQFWGridLoopDisplay* cuploop=dynamic_cast<TQFWGridLoopDisplay*>(fLoops[i]);
      cuploop->SetGridEvent(out);
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
Int_t wiresX=PEXOR_QFW_WIRES;
Int_t wiresY=PEXOR_QFW_WIRES;
if(fGridData)
{
  // take real number of wires from event object
    wiresX=fGridData->GetNumXWires();
    wiresY=fGridData->GetNumYWires();
}

/* xy beam display*/
obname.Form("Beam/GridAccu");
obtitle.Form("Beam grid current accumulate");

hBeamX = MakeTH1('I', Form("Beam/Grid%2d/Profile_X_G%d", grid, grid), Form("X Profile Grid%2d", grid), wiresX, 0,
    wiresX, "Wire");
hBeamY = MakeTH1('I', Form("Beam/Grid%2d/Profile_Y_G%d", grid, grid), Form("Y Profile Grid%2d", grid), wiresY, 0,
    wiresY, "Wire");

hBeamAccX = MakeTH1('I', Form("Beam/Grid%2d/ProfileSum_X_G%d", grid, grid), Form("X Profile accumulated Grid%2d", grid),
    wiresX, 0, wiresX, "Wire");
hBeamAccY = MakeTH1('I', Form("Beam/Grid%2d/ProfileSum_Y_G%d", grid, grid), Form("Y Profile accumulated Grid%2d", grid),
    wiresY, 0, wiresY, "Wire");

obname.Form("Beam Display Grid%2d", grid);
pBeamProfiles = GetPicture(obname.Data());
if (pBeamProfiles == 0)
{
  obtitle.Form("Overview beam projections Grid%2d", grid);
  foldername.Form("Beam/Grid%2d", grid);
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

hBeamMeanXY = MakeTH2('I', Form("Beam/Grid%2d/Meanpos_G%d", grid, grid),
    Form("Beam Mean position scaler Grid%2d", grid), wiresX, 0, wiresX, wiresY, 0, wiresY, "X",
    "Y");

hBeamRMSX = MakeTH1('I', Form("Beam/Grid%2d/RMS_X_G%d", grid, grid), Form("X Profile RMS Grid%2d", grid),
    10 * wiresX, 0, wiresX, "Wire spacings");

hBeamRMSY = MakeTH1('I', Form("Beam/Grid%2d/RMS_Y_G%d", grid, grid), Form("Y Profile RMS Grid%2d", grid),
    10 * wiresX, 0, wiresY, "Wire spacings");

obname.Form("Beam RMS Grid%2d", grid);
pBeamRMS = GetPicture(obname.Data());
if (pBeamRMS == 0)
{
  obtitle.Form("Beam RMS distribution Grid%2d", grid);
  foldername.Form("Beam/Grid%2d", grid);
  pBeamRMS = new TGo4Picture(obname.Data(), obtitle.Data());
  pBeamRMS->SetDivision(1, 2);
  pBeamRMS->Pic(0, 0)->AddObject(hBeamRMSX);
  pBeamRMS->Pic(0, 0)->SetFillAtt(3, 3002);
  pBeamRMS->Pic(0, 1)->AddObject(hBeamRMSY);
  pBeamRMS->Pic(0, 1)->SetFillAtt(4, 3002);
  AddPicture(pBeamRMS, foldername.Data());

}



}






TQFWCupLoopDisplay::TQFWCupLoopDisplay(Int_t cupid, Int_t loopid)

{

}
TQFWCupLoopDisplay::~TQFWCupLoopDisplay()
{

}
void TQFWCupLoopDisplay::SetCupEvent(TQFWProfileEvent* out)
{
  fCupData=out->GetCup(GetDevId());

}

void TQFWCupLoopDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
// TODO: put some scaler histograms here

}





TQFWCupDisplay::TQFWCupDisplay(Int_t cupid): fCupData(0)
{
  for (unsigned i = 0; i < PEXOR_QFWLOOPS; ++i)
  {
    AddLoopDisplay(new TQFWCupLoopDisplay(cupid, i));
  }
}
TQFWCupDisplay::~TQFWCupDisplay()
{

}
void TQFWCupDisplay::SetCupEvent(TQFWProfileEvent* out)
{
  fCupData=out->GetCup(GetDevId());
  for (unsigned i = 0; i < fLoops.size(); ++i)
   {TQFWCupLoopDisplay* cuploop=dynamic_cast<TQFWCupLoopDisplay*>(fLoops[i]);
     cuploop->SetCupEvent(out);
   }


}

void TQFWCupDisplay::InitDisplay(Int_t timeslices, Bool_t replace)
{
  TQFWDisplay::InitDisplay(timeslices, replace);
// TODO: put some scaler histograms here

//  TH1* hCupScaler;
//   TH1* hCupAccScaler;
  Int_t cup = GetDevId();
  Int_t segs=PEXOR_QFW_CUPSEGMENTS;
 if(fCupData)
  {
    // take real number of segments from event object
      segs=fCupData->GetNumSegments();
  }

  hCupScaler = MakeTH1('I', Form("Beam/Cup%2d/Scaler_C%d", cup, cup), Form("Segment scaler Cup%2d", cup), segs, 0,
      segs, "Wire");

  hCupAccScaler = MakeTH1('I', Form("Beam/Cup%2d/ScalerSum_C%d", cup, cup), Form("Segment scaler accumulated Cup%2d", cup), segs, 0,
        segs, "Wire");


}












