-------------------------------------------------------------
    The GSI Online Offline Object Oriented (Go4) Project
    Experiment Data Processing at EEL department, GSI
------------------------------------------------------------

***********************************************************
*   Go4 user defined event source 
*   for processing ROOT trees in NCSLDAQ/DDAS dumped format   
***********************************************************
        13-Jan-2020, J. Adamczewski-Musch (j.adamczewski@gsi.de)
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




