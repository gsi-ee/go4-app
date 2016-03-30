#!/usr/bin/env python

from go4py import ExpMem


f = ExpMem("export.root")
#f = ExpMem("Go4AutoSave.root")

print "All objects:"
for obj in f.Objects():
    print obj

print
print "Something not in the object list:"
for obj in f.MatchingObjects("does_not_exist"):
    print obj

print
print "All objects containing \"Ch\":"
for obj in f.MatchingObjects("*Ch*"):
    print obj

print
print "All histograms ending on \"g\":"
for obj in f.MatchingObjects("*g", "Histograms"):
    print obj



