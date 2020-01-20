-------------------------------------------------------------
    The GSI Online Offline Object Oriented (Go4) Project
    Experiment Data Processing at EEL department, GSI
------------------------------------------------------------

***********************************************************
*   Go4 user defined event source and analysis example
*   for processing ROOT trees in NCSLDAQ/DDAS dumped format   
***********************************************************
        v 0.5 20-Jan-2020 by JAM, Joern Adamczewski-Musch (j.adamczewski@gsi.de)
---------------------------------------------------------------
Copyright (C) 2000- Gesellschaft f. Schwerionenforschung, GSI
                    Planckstr. 1, 64291 Darmstadt, Germany
Contact:            http://go4.gsi.de
---------------------------------------------------------------
This software can be used under the license agreements as stated
in Go4License.txt file which is part of the distribution.
---------------------------------------------------------------

This package contains some classes of NSCL DDAS Software and ddasdumper
by  Ron Fox, Jeromy Tompkins, and Sean LIddick (fox@scl.msu.edu).
The full code is available under https://git.code.sf.net/p/nscldaq/ddas
All code from nscl is collected in subfolder nscl. 

_________________________________________________________________________
OVERVIEW:
The TDDASEventSource class wraps existing code of FilterDDAS.cpp into the 
go4 environment. A raw event ROOT tree produced by ddasdumper
from a NCSLDAQ .evt file is read into the TDDASRawEvent which keeps 
the original DDASEvent structure as produced by ddasdumper.

The first Go4 Analysis step TDDASFilterProc will do some histogramming and may write
a tree with a filtered DDASEvent, also wrapped into a TDDASRawEvent.

A second analysis step TDDASAnalysisProc can continue on the TDDASRawEvent, or read it back from
the intermediate ROOT file. Since input and output structure of the first analysis step are the same class, 
it is also possible for the second analysis step to use the TDDASEventSource to read directly ROOT files produced
with the external FilterDDAS code (with first analysis step disabled).
The output of the TDDASAnalysisProc will be a Go4 TDDASAnalysisEvent that wraps 
the "rootout" class used in AnalysisDDAS example.

__________________________________________________________________________________
FIRST ANALYSIS STEP: TDDASFilterProc

This Go4 processor contains mostly the code of previous standanlone example FilterDDAS.cpp.

Function MapChannels() is kept here as a static method, i.e. it is also used in the second analysis step.

Constructor TDDASFilterProc() initializes the ddas library classes betadecay and betadecayvars as member objects
fBdecay and fBdecayVars, resp. Please note that the setup of the betadecayvars parameters is taken from local text
files DSSDInit.txt, Otherinit.txt, and SuNinit.txt in subfolder cal of current directory.
Additionally, some Go4 histograms are created in subfolders "Raw" and "Mapped"

Method DdasToEvent() is almost the same as the existing function ddastoevent() from original code.
It gets the channellist from the input event TDDASRawEvent and processes it to filter out the "good" events according 
to the criteria of the original code: only events that contain imax and dmax above a threshold (at the front or at the 
back of the DSSD) detector can pass, i.e. the Go4 output event (also a TDDASRawEvent that is a copy of the input data) 
is marked as valid. In Go4 framework, only valid events are stored into the ROOT Tree if the Eventstore of the analysis step 
is enabled.
In addition to the original code, one may control this filtering by values in the class TDDASFilterParameter:

  Bool_t fDoFiltering; // if true, output event will be filtered with given dssd thresholds, if false pass through everything
  
  Int_t fDSSD_Front_Imax; // lower threshold of front dssd imax value for filtering
  
  Int_t fDSSD_Front_Dmax; // lower threshold of front dssd dmax value for filtering
  
  Int_t fDSSD_Back_Imax; // lower threshold of back dssd imax value for filtering
  
  Int_t fDSSD_Back_Dmax; // lower threshold of back dssd dmax value for filtering
  
As usual, these values can be changed interactively from Go4 GUI with parameter editor, or by editing the setup script
set_DDASFilterParameter.C that is executed as default whenever a new "Submit" of analysis settings  is done.

For immediate monitoring, some histograms are filled containing time, energy and traces data of the ADC channels (Raw folder).
As example, also some mappped histograms are accumulatged for front and back strips of the DSSD, and for
the SuN pmts. Please feel free to add another plots if it is reasonable for your purpose.


______________________________________________________________________________________________
SECOND ANALYSIS STEP: TDDASAnalysisProc

This Go4 processor contains mostly the code of previous standanlone example AnalyzeDDAS.cpp.

The correlator class (defined in subfolder nscl) is kept here as a member component fCorrelator.

Constructor TDDASAnalysisProc() initializes the ddas library classes betadecay and betadecayvars as member objects
fBdecay and fBdecayVars, resp.  Please note that the setup of the betadecayvars parameters is taken from local text
files DSSDInit.txt, Otherinit.txt, and SuNinit.txt in subfolder cal of current directory. 
Additionally, some Go4 histograms are created in subfolder "Analyzed".

The analysis parameters (previously arguments of the main() function of AnalyzeDDAS.cpp) are now 
controled by Go4 class TDDASAnalysisParameter:

 Bool_t fUseGate; // if true, will use TCutG of fGateName for Correlator. If false, gate is neglected.
 
 TString fGateName; // name of existing TCutG for gating in correlator
 
 Double_t fClockMax; // Correlation time
 
 Double_t fMinImplantTime; // Minimum implant time
 
 Int_t fRandomFlag; // random flag for correlator
 
As usual, these values can be changed interactively from Go4 GUI with parameter editor, or by editing the setup script
set_DDASAnalysisParameter.C that is executed as default whenever a new "Submit" of analysis settings is done.

If fUseGate is true, Go4 will create a Go4PolygonCondition (container of TCutG) and assign its name to the correlator object
(please note: the correlator of original code had the cut feature completely commented out; this was enabled again).
TODO: This feature has still to be tested! Also missing: reasonable TH2 histogram to display the TCutG

Method DdasToEvent() is almost the same as the existing function ddastoevent() from original code.
It gets the channellist from the input event TDDASRawEvent.  
Note that here only the "valid" input events are processed, i.e. the ones
that the first filtering steps let pass.
Furthermore it does unpacking/mapping and uses the correlator fCorrelator
for further analysis. The results are copied to the component fData of the output event TDDASAnalysisEvent, which is the
same data structure rootdataout as defined in the original code (Parameters-ddas.h)
This may be written to an output tree if the eventstore of the analysis step is switched on.
Please note that original example AnalyzeDDAS write the rootdataout to a top level branch of name "data", while go4 
would embed the rootdataout into a TGo4EventElement with subbranch of name "fData".

Method FillHistograms() will accumulate some monitoring histograms of data of the rootdataout structure in subfolder "Analyzed". 
As example, the energies of the front and back strips of DSSD are provided (may be compared with the corresponding histograms in the 
"Mapped" folder before filtering). Additionally, other summary values are histogrammed. Please feel free to implement more things here
that you would like to see before the further analysis of the output tree!


______________________
BUILDING and RUNNING
NOTE: This Go4 user library requires the  Go4/ROOT framework only, since all relevant external classes from ddas are
contained as source code and will be compiled within the libGo4UserAnalysis.
It is not necessary to link against existing libraries of nscldaq or specTCL!
However, later the Makefile may be adjusted such that code in subfolder ddas is neglected, 
and such that include and link paths use an existing installation for consistency.

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

__________________________
ANALYSIS CONFIGURATION
The setup of the two analysis steps can be done from gui analysis configuration window.
A root file with a tree created from ddasdumper should be the input of first analysis step (Go4 event source "User Source").
One may also disable the first analysis step and define this Go4 user source as input of the second step "Analysis", 
if the input file contains already filtered data from FilterDDAS program.

Both Go4 steps "Filter" and "Analysis" can write outut trees of their event class if the "Store" option is enabled.
The name of the output file and also  some tree parameters may be defined for the Go4FileStore.
Step "Filter" will write tree of class TDDASRawEvent (contains DDASEvent with list of ddaschannel), 
step "Analysis" will write tree of class TDDASAnalysisEvent (contains rootdataout).

One may disable step "Filter" and set step "Analysis" event source to TGo4FileSource to process a tree 
that has been written before from Go4 analysis step "Filter.

___________________
FURTHER INFORMATION
All information about go4 are available at
https://go4.gsi.de

Download the go4 framework at
http://web-docs.gsi.de/~go4/download/go4.php

The Go4 source repository:
current version
https://subversion.gsi.de/go4/tags/600-00/
development version
https://subversion.gsi.de/go4/trunk/

Please have a look at the Go4 user manual available at
http://web-docs.gsi.de/~go4/go4V06/manuals/Go4introV6.pdf
(or in the Go4 GUI Help menu ;-))
______________________________________________________________________JAM 20-01-2020




