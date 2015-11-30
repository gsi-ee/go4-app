#!/usr/bin/env python

from go4py import ExpMem


f = ExpMem("export.root")

print "All objects:"
for obj in f.Objects():
    print obj

print
print "Something not in the object list:"
for obj in f.MatchingObjects("does_not_exist"):
    print obj

print
print "All objects containing \"x\":"
for obj in f.MatchingObjects("*x*"):
    print obj

print
print "All histograms ending on \"_ele\":"
for obj in f.MatchingObjects("*_ele", "Histograms"):
    print obj



