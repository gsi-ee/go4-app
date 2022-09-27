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
// V 0.99 27-September-2022
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





JAM 27-September-2022