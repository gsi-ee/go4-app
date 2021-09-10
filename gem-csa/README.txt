//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at CSEE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------


---------------------------------------------
// Analysis of readout of GEM CSA via FEBEX/MBS.
// V 0.20 10-Sep-2021
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

The first Go4 Analysis step  of class TGemCSABasicProc does unpacking of MBS data from FEBEX/GEM-csa setup, fills histograms and
copies data to the output event structure of class TGemCSABasicEvent.
All histograms and go4 objects are created dynamically in function TGemCSABasicProc::f_make_histo(), depending on
the header information of the connected front-ends send in the MBS data stream.
The actual unpacking and histograming is done in the TGemCSABasicProc::BuildEvent() function.

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
They are initialized from compiled code by the defines CSA_BASE_START, CSA_BASE_SIZE, CSA_SIGNAL_START, CSA_SIGNAL_SIZE in TGemCSABasicProc.h

---
For interactive control the parameter container TGemCSABasicParam has following switches:

 Bool_t fDoBaselineFits; 
 - if true, polynomial fit is done for traces within the range of Go4 condition fxBackgroundRegion for all channel traces.
   The fit result values (A0,A1,chi2, model curve)  are accumulated for each channel into histograms in subfolder BaselineFits. 
   There are also go4 pictures at Pictures/BaselineFits that show all of this in a predefined canvas.
   NOTE: such fit slows down analysis very much, so usually it is not used unless the simple baseline estimation is not sufficient
   
Bool_t fMapTraces; 
- if true, full traces are copied to the output event TGemCSABasicEvent to be mapped to detector wires or pads. However, since this is
quite time consuming, it can also be disabled here if not needed

Bool_t fSlowMotion; 
- if true than only process one MBS event and stop. This can be helpful to debug data by printout in analysis terminal.


The script set_GemCSABasocParam.C offers possibility to change the setup of this mapping without recompiling or using the paramter editor with the
autosave file. If existing in the working directory, it will override both previous setups.

____________________________________
SECOND ANALYSIS STEP:

The second analysis step of class TGemCSAMapProc can continue on the TGemCSABasicEvent, or read it back from intermediate root tree. 
It is intended to map frontend channels to detector chambers and do higher level correlations. It may fill additional root histograms
and put result data to output event structure of class TGemCSAMapEvent (TODO!).
Again the main work is done in the TGemCSAMapProc::BuildEvent().

---
The setup of the second analysis step is defined in parameter container TGemCSAMapParam by a number of arrays that relate the frontend id numbers
(SFP, slave, channel) to wire numbers of detector chambers;

 Int_t fSFP [chamber][wire]; 
- number of kinpex sfp that produces data for wire id  of chamber id

Int_t fSlave [chamber][wire];
- number of febex slave that produces data for wire id  of chamber id
      
Int_t fChannel [chamber][wire];
-febex channel that produces data for wire id of chamber id

The maximum configurable number of wires is defined at compile time with values CSA_MAXCHAMBERS and CSA_MAXWIRES in TGemCSAMapEvent.h

The script set_GemCSAMapParam.C offers possibility to change the setup of this mapping without recompiling or using the parameter editor with the
autosave file. If existing in the working directory, it will override both previous setups.


___________________
FURTHER INFORMATION
All information about go4 are available at
https://go4.gsi.de

Download the go4 framework at
http://web-docs.gsi.de/~go4/download/go4.php

The Go4 source repository:
current version
https://subversion.gsi.de/go4/tags/601-02/
development version
https://subversion.gsi.de/go4/trunk/

Please have a look at the Go4 user manual available at
http://web-docs.gsi.de/~go4/go4V06/manuals/Go4introV6.pdf
(or in the Go4 GUI Help menu ;-))

___________________________________________________________________JAM 10-09-2021




