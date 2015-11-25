-----------------------------------------------------------------------------------
    The GSI Online Offline Object Oriented (Go4) Project
    Experiment Data Processing at EE department, GSI
-----------------------------------------------------------------------------------

This project folder is dedicated to Go4 analysis framework related Python code.
This may cover wrappers for TGo4Analysis and example application scripts.
JAM (j.adamczewski@gsi.de) 18-Nov-2015

So far, only a small example can be found here, demonstrating:
a) how to use the wrapper around the PyROOT-bound go4-object
b) how the pythonistic library is supposed to work

In case an "ImportError: No module named go4" is received, check whether go4login
contains a line that adds $GO4SYS/python to the PYTHONPATH.
If this line is missing, your Go4 is either too old (a revision >= r1803 is needed)
or needs to be rebuilt (make clean; make) such that go4login is recreated.

-----------------------------------------------------------------------------------
To be continued...

