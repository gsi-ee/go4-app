
import go4py
import ROOT


# Produce a profile from a 2D histogram
h = next(go4py.MatchingObjects("Cr1Ch1x2", "Histograms"))

ylow  = h.GetYaxis().FindBin(500)
yhigh = h.GetYaxis().FindBin(1500)

p = h.ProfileX(h.GetName() + "_pfx", ylow, yhigh)


# Fit a 1D histogram
h = next(go4py.MatchingObjects("His1", "Histograms"))
h = h.Clone()
h.Rebin(10)

xlow, xhigh = 400, 1300
sinus = ROOT.TF1("sinus", "[0]*sin(TMath::TwoPi()*x/[1] + [2]) + [3]", xlow, xhigh)

sinus.SetParName(0, "amplitude")
sinus.SetParName(1, "period")
sinus.SetParName(2, "phase")
sinus.SetParName(3, "offset")

sinus.SetParameter(0, 15)
sinus.SetParameter(1, 500)
sinus.SetParameter(2, 5)
sinus.SetParameter(3, 10)

sinus.SetParLimits(0, 10, 30);
sinus.SetParLimits(1, 350, 550);
sinus.SetParLimits(2, 0, 10);
sinus.SetParLimits(3, 0, 20);

h.Fit("sinus", "", "", xlow, xhigh)


xs, ys = go4py.convert.hist2list(h)
yfs = go4py.convert.eval_func(xs, sinus)

import matplotlib.pyplot as plt
plt.step(xs, ys)
plt.step(xs, yfs)
plt.show()



