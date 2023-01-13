//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at CSEE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------


---------------------------------------------
// Readout of Get4++ ASIC via VULOM/MBS.
// V 0.991 13-January-2022
// Joern Adamczewski-Musch, EEL, GSI Darmstadt
// j.adamczewski@gsi.de
---------------------------------------------

Processor class TGet4ppRawProc will unpack MBS formatted data from readout of Get4++ Test system.
This readout is done with VULOM in VMEbus crate System driven by MBS data acquisition.
 
With the special user event source, such readout container may be emulated using ASCII data from chip simulations.
Note that real chip data from MBS will use *.lmd file or stream server as event source.
Simulation data can use Go4 user event source with *.dat files as input. The unpacker code is identical for both data sources.

Unpacked TDC messages are histogrammed into several subfolders, sorted by channel id of messages. 
Messages without meaningfull channel, such as some error messages, are accounted for channel 0.

All time counters are accumulated, also the full time stamp in fine counter units and in seconds.

Time over threshold between subsequent leading and trailing edges is evaluated. Here also the kind of edge and missing edges are accounted.
Scalers for all kind of state bits (errors, sync, etc) are provided.

Wishbone messages (if existing in data stream) are treated and their contents histogrammed. 
Other (impossible?) message types are also counted and may indicate some problem with the readout.


The parameter class TGet4ppRawParam offers some switches to control processing:

 - Int_t fVerbosity; 
  	// degree of debug printouts. 
 	0 - mostly silent, 
 	1 - show data problems, like unfinished payload that causes to skip rest of message or event
 	2 - dump all data words, i.e. header content of messages. NOTE: use slow motion mode for this!
    
- Bool_t fSlowMotion; 
   // if set to true than only process one MBS event and stop.

--------
For evaluation of measurement series files, it has been implemented to write the bins of leading and trailing fine time histograms 
for each file into the output event. If file store is enabled, this will be written to an output tree. Each tree entry corresponds to one input file.
This feature can be enabled/disabled at compile time with #define Get4pp_DOFINETIMSAMPLES (in TGet4ppRawEvent.h). By default it is on.

Example root macro readGet4ppTree.C shows how to read back the fine time bins into histograms from the written Go4 eventstore tree.

Control of this fine time samples can be done with additional parameters in TGet4ppRawParam:

  Int_t fFineTimeSampleBoard; 
   		// id of board for fine time evaluation - default is 42 (the only board for lab setup :))
  Int_t fFineTimeStatsLimit; 
        // number of event threshold for writing fine times
        - after this number of events is reached, the fine time bin entries of leading and trailing edge histograms are copied to output event for tree storage.
          All other event in the current input lmd file are skipped. When the next file of the lml input list/name wildcard is opened, the event counter is reset and another
          measuerment result will be storead after this number of events, etc.

----------------
Additional Fine time calibration 

To check possible improvements by a software calibration of fine time counters like in FPGA TDCS, some functionality was added:

-Additional histograms to keep the fine time calibration of each channel:
In folder "ChannelN/Calibration": For each edge leading/trailing, the calibration curve (bins->fine time value) is kept as histogram
"Time_fine_calibration_leading_i_j" and "Time_fine_calibration_trailing_i_j". 
These histograms are persistent against the interactive "clear" button of all other histograms!
The only way to clear old calibrations kept in this histogram is either to remove the autosave file, or to initiate a new calibration procedure
by means of fResetCalibration flag (see below).

-Additional histograms for channelwise time difference of leading edges:

In "DeltaTime" folder, new subfolders "raw_calib" and "seconds_calib" containing the corresponding dt histograms calculated
from the calibrated times. These are filled only when the fUseSoftwareCalibration mode in parameter is enabled, and a valid calibration exists for
_all_ channels of all boards under test.
The uncalibrated dt histograms are filled anyway with uncalibrated time differences, as before.


In parameter TGet4ppRawParam, as controllable from GUI:

    Bool_t fUseSoftwareCalibration; 
    	// if true (default), evaluate delta times with software calibration of fine time bins. 
    	Then the delta time values (mu, sigma) as stored to the TTree are used with the calibrated fine times.
    	Otherwise, the raw fine time is used as before
  
   Bool_t fResetCalibration; 
   		// set true to begin new software calibration procedure. Usually, if calibration histograms  are is existing in the Go4 autosave file 
   		from a previous calibration run, this calibration is used for all newly unpacked messages. To start new calibration, put this switch
   		to true (also possible on the fly with online monitor).
   		Note that this is the only way to reset calibration histograms, since they are write protected and are kept persistent when
   		user presses the clear button for other histograms!
 
   Int_t  fCalibrationCounts; 
         // minimum required statistics for each channel (leading/trailing) to complete calibration
         When initial calibration histograms are empty, or if user switches the fResetCalibration flag, the accumulated data
         of the fine time (box) histograms "FineTimeLeading*" and "FineTimeTrailing" is used to evaluate a calibration curve
         when the number of MBS events reaches the fCalibrationCounts value.

-----------
Suggested procedure to use fine time calibration:
	1) select an lmd file as input with enough event statistics. Probably 5000 MBS events are sufficient for calibration, but the more the better. 
	   The desired statistics for calibration may be set by changing the fCalibrationCounts in the parameter. Instead of an lmd input file, 
	   calibration may be done directly from MBS stream server (online mode)
	
	2) start data processing with blank calibration (no autosave file before), or after setting fResetCalibration in the parameter editor on GUI. Note
	   that you should clear (fine time box histograms) before running new calibration.
	
	3) When calibration is ready, this will be printed on analysis terminal. 
	
	4) Close the calibration run. This calibration is stored in autosave file
	
	5) Start evaluating lmd data under test with this calibration from autosave file.
	
	6) Comparison between calibrated and non-calibrated dt scan can be done by changing flag fUseSoftwareCalibration in parameter. Note
	that existing calibration histogram is not discarded by setting fUseSoftwareCalibration=false, so one may switch to and fro as long as
	autosave file is kept.

---------

__
JAM 13-January-2023