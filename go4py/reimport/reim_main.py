
# Allow files in this folder to be found:
from go4py import addthispath
addthispath()



# If a module is regularly imported it will be cached.
# Changes to the module will not be visible until a interpreter restart 
# or an explicit reload.

# This can be confirmed by uncommenting the second definition of somefunc 
# in reim_lib.py and running this script again.

# Using go4py.reimport instead of or additionally to the import shows 
# changes immediately...


import reim_lib

#from go4py import reimport
#reim_lib = reimport("reim_lib")

reim_lib.somefunc()



