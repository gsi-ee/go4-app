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
// Analysis of readout of Awags @ sIS via FEBEX/MBS.
// V 0.20 02-Aug-2022
// Joern Adamczewski-Musch, EEL, GSI Darmstadt
// j.adamczewski@gsi.de
---------------------------------------------
// this code contains parts of the general MBS/FEBEX go4 analysis written by Nikolaus Kurz (n.kurz@gsi.de)
----------------------------------------------------------------------------------------------------------

____________________
BUILDING and STARTUP:

To build it just enable go4 environment by calling 

> . $GO4SYS/go4login
(bash dot script; with GO4SYS the go4 installation directory)
Then call
> make 

To start Go4 GUI just call
> go4
and launch analysis in local folder with libGo4UserAnalysis.so.
Do not forget to create a Go4 hotstart script after this!

As usual, 
> go4analysis 
will offer a batch mode (please see go4analysis -h)

This code has been compiled and tested with  Go4 v6.1.2 and root 6.22/08. 

____________________
FIRST ANALYSIS STEP:

The first Go4 Analysis step  of class TAwagsSosProc does unpacking of MBS data from FEBEX/GEM-csa setup, fills histograms and
copies data to the output event structure of class TAwagsSisBasicEvent.
All histograms and go4 objects are created dynamically in function TAwagsSisBasicProc::InitDisplay (), depending on
the header information of the connected front-ends send in the MBS data stream.
The actual unpacking and histograming is done in the TAwagsSisBasicProc::BuildEvent() function.

Here also an evaluation of average signal values of all channel traces is done within the region defined by two Go4 conditions:
-  fxBackgroundRegion ("BackgroundRegion"): initial part before signal is acquired
-  fxSignalRegion ("SignalRegion"): actual sampled signal
The computed average height in these user defined region at each event is used to accumulate for each channel following histograms
in subfolder "Baselines":
- average background values (subsubfolder "Background")
- average signal values (subsubfolder "Signals")
- signal to background ratio of it (subsubfolder "StoB")
- difference signal minus background(subsubfolder "S-B")

These histograms with the original trace and the conditions are contained together in Go4 pictures in folder "Pictures/Baselines"

The regions for background and signal can be changed interactively in the Go4 GUI condition editor and are kept in the autosave file.
They are initialized from compiled code by the defines CSA_BASE_START, CSA_BASE_SIZE, CSA_SIGNAL_START, CSA_SIGNAL_SIZE in TAwagsSisBasicProc.h

______________
Spill finding:

Since one SIS spill spans its samples over several MBS events, the begin of each spill is searched in the first analysis step.
The criterion is the ratio (or difference, resp.) between "signal" region (end of each febex trace, after awags reset) and "background" region 
(begin of each febex trace before awags reset).
Function HandleSignalToBackground() will evaluate this for all channels and return the value used for spill state discrimmination.
This value is then used in function EvaluateSpills() to check if the current MBS event is in or out of spill.
The window condition "SpillSelect" defines this state: if the test value "signal to background"  (or "signal minus background", resp) value is inside (above the lower threshold), 
the MBS event is regarded as in spill. If it is outside (below threshold), the spill ends and the next spill is searched. Begin of new spill is when test value is again
within condition window.

If "signal to background" or "signal minus background" should be checked for spill, is defined at compile time by  #define USE_SIGNALTOBACK_RATIO. Signal to background ratio is default.

The spill decision may be evaluated by the average of  "signal to background" of _all_ channels, or by setting a dedicated "trigger channel" that is known to be 
in the full beam all the time. The trigger channel is defined by indices fSpillTriggerSFP, fSpillTriggerSlave, and fSpillTriggerChan in the TAwagsSisParam setup (see below).
For fSpillTriggerSFP<0 the average off all channels is used. 

Histogram "Baselines/StoB/Signal_background_ratio_trigger channel" (or "Signal_background_ratio_average", resp.) accumulates the spill test values and can be used to tune 
the condition "SpillSelect". If USE_SIGNALTOBACK_RATIO is not active, these histograms are named "Signal_background_diff_trigger channel" or "Signal_background_diff_average". 

Overview of spill characteristics is available by histograms "Spills/EventScaler" and "Spills/SpillSize". The first displays number of complete spills found 
and the number of mbs events during spill. The second gives statistics of length of spills in mbs events.


Note that data is passed to second analysis step only when current spill is complete. Then the intermediate data structure TAwagsSisBasicEvent 
is flagged as "valid" and may be processed by TAwagsSisMapProc for mapped chamber displays.

The vectors fChargeTrend contain the estimated charge of each channel for any MBS event sample in most recent spill. 
A figure of merit for the accumulated charge is the difference of the average baseline in the "signal" region and the "background" region, as set with the same conditions
"SignalRegion" and "BackgroundRegion" used for spill evaluation. This is motivated by the fact that "signal" contains the febex trace directly after awags reset, and 
background is the remaining trace which was started top be sampled by the previous mbs event. 

Additionaly, the stitched channel traces of the complete spill may be passed to the second analysis step if #define AWAGS_STORE_TRACES is set at compile time.
This is currently not used.

Overview of stitched channel traces and charge trend per spill is given anyway by channelwise histograms under the "Spill" folder.
These are best displayed together with go4 pictures, e.g. "Pictures/Spills/SFP0 0_Slave 1/Picture_Spills_ 0_ 1_ 0".

For future beam frequency analysis, there is histogram "Histograms/Spills/Traces/AllSignalTrace" that contains the stitch trace of all channels and 
all mbs event samples within the "signal" region.
This may be used for a quick fft (see available macro in the "Go4 command line" tool), or more elaborate analysis in future versions of this code.



-------------------------
Parameter TAwagsSisParam: 
  Int_t fNumEventsPerSpill; 
  // estimated number of MBS event for each spill - for display. If changed, histograms will be recreated at next resubmit of settings!
  Int_t fMaxSpillEvent;    // maximum MBS event index for each spill. After this we are out of spill for sure. 
  Bool_t fMapSpills; // if true copy spill traces to output event for mapping step.
  Int_t fSpillTriggerSFP; // sfp for spill trigger decision channel. set to -1 to use average of all channels
  UInt_t fSpillTriggerSlave; // slave for spill trigger decision channel
  UInt_t fSpillTriggerChan; // slave for spill trigger decision channel


The script set_AwagsSisParam.C offers possibility to change the setup of this mapping without recompiling or using the paramter editor with the
autosave file. If existing in the working directory, it will override both previous setups.

____________________________________
SECOND ANALYSIS STEP:

The second analysis step of class TAwagsSisMapProc can continue on the TAwagsSisBasicEvent, or read it back from intermediate root tree. 
It is intended to map frontend channels to detector chambers and do higher level correlations. It may fill additional root histograms
and put result data to output event structure of class TAwagsSisMapEvent (TODO!).
Again the main work is done in the TAwagsSisMapProc::BuildEvent().

---
The setup of the second analysis step is defined in parameter container TAwagsSisMapParam by a number of arrays that relate the frontend id numbers
(SFP, slave, channel) to wire numbers of detector chambers;

Int_t fMaxChamber; // number of chambers in use

Int_t fMaxWire[chamber]; // number of wires for each chamber


Int_t fSFP [chamber][wire]; 
- number of kinpex sfp that produces data for wire id  of chamber id

Int_t fSlave [chamber][wire];
- number of febex slave that produces data for wire id  of chamber id
      
Int_t fChannel [chamber][wire];
-febex channel that produces data for wire id of chamber id

The maximum configurable number of wires is defined at compile time with values CSA_MAXCHAMBERS and CSA_MAXWIRES in TAwagsSisMapEvent.h

Additonally, there are switches to set analysis to slow control for monitoring and debugging:Ä

Bool_t fSlowMotion; // if true than only process one MBS event and stop.

Bool_t fStopAtEachSpill; // if true stop processing after each complete spill
     
The script set_AwagsSisMapParam.C offers possibility to change the setup of this mapping without recompiling or using the parameter editor with the
autosave file. If existing in the working directory, it will override both previous setups.

_______________
Mapped displays:

Using the channel to wire mapping as defined in the paramter, the second analysis step offers histogram in the "Histograms/Mapped" folder.



___________________
FURTHER INFORMATION
All information about go4 are available at
https://go4.gsi.de


The Go4 source repository:
current version
https://subversion.gsi.de/go4/tags/602-00/
development version
https://subversion.gsi.de/go4/trunk/

Please have a look at the Go4 user manual available at
http://web-docs.gsi.de/~go4/go4V06/manuals/Go4introV6.pdf
(or in the Go4 GUI Help menu ;-))

___________________________________________________________________JAM 02-08-2022




