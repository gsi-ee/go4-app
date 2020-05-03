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
// V 0.91 03-May-2020
// Joern Adamczewski-Musch, CSEE, GSI Darmstadt
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


The parameter class TG4et4ppRawParam offers some switches to control processing:

 - Int_t fVerbosity; 
  	// degree of debug printouts. 
 	0 - mostly silent, 
 	1 - show data problems, like unfinished payload that causes to skip rest of message or event
 	2 - dump all data words, i.e. header content of messages. NOTE: use slow motion mode for this!
    
- Bool_t fSlowMotion; 
   // if set to true than only process one MBS event and stop.



Some more description will follow one day...



JAM 03-May-2020