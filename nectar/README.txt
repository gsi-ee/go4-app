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
// Display readout of MDPP and VMMR data  for Nectar setup
// V 0.1 09-Sep-2021
// V 0.2 19-Oct-2021 - added README and small fixes.
// Joern Adamczewski-Musch, EEL, GSI Darmstadt
// j.adamczewski@gsi.de
---------------------------------------------

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

_____________
USER ANALYSIS:

The main analysis structure is set up in constructor of class TNectarAnalysis.
Here different subsequent "analysis steps" can be defined by means of a TGo4StepFactory. This
stepfactory will tell the framework which processor class and event class shall be created from the user
library with functions DefEventProcessor() and DefOutputEvent(). The analysis step is then created using
this factory and registered by calling AddAnalysisStep().
Currently there is only one analysis step provided for the raw unpacker, but others may be added later.

Please note that all initial definitions of input/output file etc specified here can still be redefined at runtime
by go4analysis command line argumens, or by the analysis configuration GUI. However,
analysis steps and their classes must be defined here at compile time.



____________________
FIRST ANALYSIS STEP:

.....................................
Event processor class TNectarRawProc:
The first Go4 Analysis step  with class TNectarRawProc does unpacking of MBS data from Mdpp and/or Vmmr setup, fills histograms and
copies data to the output event structure of class TNectarRawEvent. The main function which is executed for each input event is
BuildEvent(). Depending on the type of payload data, it will call functions UnpackMdpp(), and UnpackVmmr() 
to treat either Mdpp or Vmmr frontend, resp.
Function UpdateDisplays() is called after the complete MBS event has been unpacked to do additional analysis
on the data kept in TNectarRawEvent. Here it will evaluate the time difference histograms to 
the defined reference channel of each front-end.

.....................................
Event structure class TNectarRawEvent:
The whole output event object will keep the complete data of one MBS input event, 
unpacked and sorted into the different frontend components MDPP and VMMR.
Therefore it is set up as TGo4CompositeEvent which can keep various subobjects of classes
TMdppBoard and TVmmrBoard that are accessed by a unique id number. Since the VMMR may read out
several slave front-ends via fiber chains, each TVmmrBoard keeps a collection of TVmmrSlave 
objects which are identified by their chain id.
The actual data is kept in std::vectors of message objects:
From class TVmmrAdcData within TVmmrSlave, and from classes TMdppAdcData and TMdppTdcData in TMdppBoard, resp.

This event structure may be saved as a ROOT tree, or can be used
for further analysis in additional analysis steps, e.g. to map the electronic channels into
displays of detector components (to be developed).

....................................
Histogram containers in TNectarDisplay.h:
To handle the partitioning of the frontends into alike units, the histograms filled by the TNectarRawProc
are organized in so called "display" classes. The base class TNectarBoardDisplay is implemented
as TMdppDisplay, TVmmrDisplay, and TVmmrSlaveDisplay, reflecting the structure of the event data.
Each display class contains their appropriate histogram pointers which are defined respectively in the 
InitDisplay() function. The id numbers of the event components are used to register these histograms
with unique names in the Go4 object management.
The event processor TNectarRawProc has vectors of TMdppDisplay and TVmmrDisplay which are created
at startup according to the front-end configuration. The TVmmrSlaveDisplay is subpart of each 
TVmmrDisplay and added automatically whenever a VMMR frontend of a chain id sends its first data message.

...............................................
Configuration and control with TNectarRawParam:
The class TNectarRawParam is a TGo4Parameter that defines both the setup of the unpacker and 
frontend event structure, and may offer additional run control flags.
The configuration is done by means of some arrays of id numbers and flags:

Int_t fVMMR_BoardID[VMMR_MAXBOARDS]; // mapping of VMMR board numbers to (slot?) indices
<- The id number must match the id that is part of the data header from the VMMR frontends and is used to identify
the event structure element and the display object (histogram name indices). Up to VMMR_MAXBOARDS=10 different
boards can be treated by default. Id -1 means that the VMMR board is not present for this slot.

Int_t fMDPP_BoardID[MDPP_MAXBOARDS]; // mapping of MDPP board numbers to (slot?) indices
<- The id number must match the id that is part of the data header from the MDPP frontends and is used to identify
the event structure element and the display object (histogram name indices). Up to MDPP_MAXBOARDS=10 different
boards can be treated by default. Id -1 means that the MDPP board is not present for this slot.

Bool_t fMDPP_is16Channels[MDPP_MAXBOARDS]; // true if MDPP of that index has 16 channels. otherwise assume 32 channels
<- For each id slot in the fMDPP_BoardID array, one can specify if this is a 16 or 32 channel device. This
is important for the unpacker, since MDPP data formats for these implementations differ.


Int_t fMDPP_ReferenceChannel[MDPP_MAXBOARDS]; // TDC reference channel for DeltaT of each MDPP board
<- For each id slot in the fMDPP_BoardID array, one can specify the reference channel to be used for the dt
evaluation and histogramming


The following flag is an example of interactive control:

Bool_t fSlowMotion; 
<- if true than only process one MBS event and stop. 
This can be helpful to debug data by printout in analysis terminal. When pressing the "start" button in Go4 gui,
the next event is processed. Please note that #define NECTAR_VERBOSE_PRINT in TNectarRawProc.cxx would switch on
a verbose printout of all data (recompile/"make" required).

Like all Go4 parameters, one can inspect (and change) the current contents of TNectarRawParam in the GUI
with the parameter editor.


.........................................
Configuration script set_NectarRawParam.C:
When Go4 is initialized (Submit button), the board ids of the parameter arrays are copied to 
static vectors TNectarRawEvent::fgConfigVmmrBoards  and TNectarRawEvent::fgConfigMdppBoards. 
This is done in the constructor of TNectarRawProc by calling function TNectarRawParam::SetConfigBoards(). 
Such vectors of TNectarRawEvent are then used in its constructor to define the subcomponents of the 
Go4 composite event. Some entries of TNectarRawParam are also accessed directly in the TNectarRawProc during
unpacking, like the reference channel.

By default, the values as specified in the source code of TNectarRawParam.cxx are used, so one could change the setup
here and recompile the user analysis.
By means of the interactive parameter editor and the autosave file, one could override this though (however, old
histgrams of deactivated modules will still be kept in the autosave file then)

The script set_NectarRawParam.C offers the possibility to change the setup without recompiling or using the parameter editor. 
If existing in the working directory, it will override both previous setups.
To use it, please edit the values assigned to the parameter members in the script 
(e.g. param0->fVMMR_BoardID[0]=1;). Then just "submit and start" again the analysis from gui, or rerun go4analysis batch job.

 
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

___________________________________________________________________JAM 19-10-2021









