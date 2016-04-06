#!/usr/bin/env python

import go4py


f = go4py.ExpMem("example.root")

ho = next(f.MatchingObjects("*g", "Histograms"))
xo, yo = go4py.convert.hist2list(ho)

hn = go4py.convert.list2hist(xo, yo)
xn, yn = go4py.convert.hist2list(hn)


def overview(l):
    return "\t".join(str(i) for i in (type(l), len(l), min(l), max(l), sum(l)))

print hn
print "\t".join(i for i in ("type:\t", "len:", "min:", "max:", "sum:"))
print overview(xo)
print overview(xn)
print overview(yo)
print overview(yn)

assert xo == xn
assert yo == yn



