-----------------------------------------------------------------------------------
    The GSI Online Offline Object Oriented (Go4) Project
    Experiment Data Processing at EE department, GSI
-----------------------------------------------------------------------------------

This project folder is dedicated to Go4 analysis framework related Python code.
This may cover wrappers for TGo4Analysis and example application scripts.
JAM (j.adamczewski@gsi.de) 18-Nov-2015

-----------------------------------------------------------------------------------

A few small examples can be found in this directory. Below, an overview is given, 
see the individual scripts for more detailed descriptions.

All scripts should be usable in the Go4 example analyses, e.g., Go4ExampleSimple, 
using $GO4SYS/data/test.lmd as input or with example.root in this directory, 
which has been saved from that example analysis.

If you have questions, ideas, or comments, feel free to send them my way:
https://www.mpi-hd.mpg.de/personalhomes/augustin/


Descriptions:
-------------

addhisto.py
Send a new histogram from Python back to the Go4 analysis.

basic.py
Overview on how the go4 wrapper and go4py work, and how to retrieve a histogram.

converters.py
Convert from ROOT histogram to Python list, and vice-versa.

expmem.py
Demonstrates the interface to exported memory or auto-saved .root files.

fit_pyroot.py
Get a ROOT histogram from the analysis and apply a fit using PyROOT.

interactive.py
Start an interactive python shell within Go4 (needs an installed PyShell).

plot_matplotlib.py
Get a ROOT histogram from the analysis and plot it using matplotlib.pyplot.

reimport/*.py
Demonstrates when/why go4py.reimport might be useful.

-----------------------------------------------------------------------------------

For the PyROOT (a.k.a. "import ROOT") manual, see here:
https://root.cern.ch/pyroot/

It's worth pointing out that there is also rootpy available:
https://rootpy.github.io/rootpy/

In particular their root_numpy package might be of interest:
https://rootpy.github.io/root_numpy/

-----------------------------------------------------------------------------------

In case an "ImportError: No module named go4" is received, check whether go4login 
contains a line that adds $GO4SYS/python to the PYTHONPATH.
If this line is missing, your Go4 is either too old (a svn revision >= r1803 or 
release version >= 5.1.0 is needed) or needs to be rebuilt (make clean; make) such 
that go4login is recreated.

-----------------------------------------------------------------------------------

To be continued...



