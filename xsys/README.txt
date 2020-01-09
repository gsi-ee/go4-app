//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------


---------------------------------------------
// Event Source for XSYS data format (.evt)
// V 1.0 24-Jan-2011
// Joern Adamczewski-Musch, EE, GSI Darmstadt
// j.adamczewski@gsi.de
// V 1.1 09-Jan-2020 - update code to compile with Go4 V6 / ROOT V.6,  moved to public go4-app svn

---------------------------------------------
Contents:
---------
README.txt
TXSYSAnalysis.h  
TXSYSEventSource.h  
TXSYSUnpackEvent.h  
TXSYSUnpackProc.h  
XSYSLinkDef.h
TXSYSAnalysis.cxx  
TXSYSEventSource.cxx  
TXSYSUnpackEvent.cxx  
TXSYSUnpackProc.cxx

Required:
---------
-GO4 analysis framework

Usage:
------
unpack tarball "tar zxvf *.gz" to working directory
Set Go4 environment ". go4login" (at gsi).
Compile user analysis "make".
Run analysis with input file name as "user source", e.g.:
"go4analysis -user /data/bkg2d215.evt"
In GUI, specify "usersource" as input of first step.

Details:
--------
The class TXSYSEventsource will read file in xsys/*.evt format and provide
each event as mbs subevent. Information of BEGIN and HALT records is print out when found. 
NOTE that specifications of XSYS data format from previous manuals
are copied for reference into comments of TXSYSEventSource.h

The mbs subevent ids are mapped to the flags found in the XSYS event headers:
Control  <- mbd flag (polarization);
Subcrate <- mbd channel (event stream number);
Procid   <- 0x55 (arbitrary identifier);

The example unpack analyis TXSYSUnpacProc.cxx will use only subevents with specific xsys Procid 0x55.
The example payload here is expected to be of 16bit words, each representing a spectrum. These are
put to histograms and to an output event structure TXSYSUnpackEvent and may be stored to ROOT tree by
Go4 file store.



--------------------JAM 24-Jan-2011----------------------------------------------------