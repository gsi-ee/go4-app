//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at CSEE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------


---------------------------------------------
// Readout of ASIC CTR16 via VULOM/MBS.
// V 0.3 1-Feb-2023
// V 0.4 27-Mar-2023: prepared for optional GOSIP readout
// Joern Adamczewski-Musch, EEL, GSI Darmstadt
// j.adamczewski@gsi.de
---------------------------------------------


################## Important: ################################### 
The unpacker can be switched between lab readout with VULOM and GOSIP readout with KILOM
by definition in  TCtr16RawEvent.h:
#define Ctr16_USE_VULOM 1


############### Parameters:

The parameters to control the monitoring are collected in class TCtr16RawParam.
They can be changed without recompiling in setup script: 
set_Ctr16RawParam.c


1) GOSIP Setup:

The wiring of the gosip slaves to board ids is done with array 
fBoardID[sfp][dev] in parameter container TCtr16RawParam
    sfp: sfp line at KINPEX (0,..,3)
    dev: device nummer at each sfp line (0,...,255?)
    fBoardID: an arbitray id number of the slave board. This is used in further analysis mapping etc.

2) other parameters in TCtr16RawParam:

  Int_t fNumSnapshots; // number of trace snapshots per mbs event
  
  Int_t fTraceLength; // length of trace to visualize (16, 32, 64)
  
  Bool_t fDoCalibrate; // switch on calibrate mode for adc correction
  
  Bool_t fSlowMotion; // if true than only process one MBS event and stop.
  
  Int_t fStopAtEvent; // mbs event sequence number to stop at.
  
  Int_t fVerbosity; // degree of debug printouts. 0 - mostly silent, 1- show data problems, 2 - dump all data words






JAM 27-Mar-2023