#!/usr/bin/env python

"""
Run an interactive PyShell from within Go4:

Thanks to TPython's interpreter state being preserved in between calls, 
closing/reopening PyShell allows to continue a session. Note that the 
analysis is paused for each command-line input, and opening a PyShell 
counts as one call.

Under Ubuntu, PyShell is part of the "python-wxtools" package:
sudo apt-get install python-wxtools
"""

#This is a slightly modified version of the PyShell startup script.
#The original segfaults on the second execution from within Go4.

#In principle, PyCrust should be usable like this, but it segfaults on 
#the second execution despite the changed startup...


from wx.py import PyShell as shell
shell.App().MainLoop()



