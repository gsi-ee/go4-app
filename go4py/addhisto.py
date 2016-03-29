
import go4py


oldhist = next(go4py.MatchingObjects("His1", "Histograms"))

newhist = oldhist.Clone()
newhist.SetName("Cloned" + oldhist.GetName())

print "Old hist name:\t", oldhist.GetName()
print "New hist name:\t", newhist.GetName()

newhist.Rebin(5)
go4.AddHistogram(newhist, "Copies")



