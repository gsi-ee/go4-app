
print "Entering Python script"

print
print "before wrapper import:"
print " go4 ==", go4
print " no NextMatchingObject gives:", go4.NextMatchingObject("does_not_exist", "*", True)


from go4py import go4
#import go4py.go4 as go4


print
print "after wrapper import:"
print " go4 ==", go4
print " go4.analysis ==", go4.analysis
print " no NextMatchingObject gives:", go4.NextMatchingObject("does_not_exist", "*", True)



import go4py

for h in go4py.MatchingObjects("*3", "Histograms"):
    print h.GetName(), h.GetEntries()



# reset go4 object to the PyROOT-bound go4
go4 = go4.analysis
print "Leaving Python script"



