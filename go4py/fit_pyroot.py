
import go4py
import ROOT


h = next(go4py.MatchingObjects("a_2d_hist", "Histograms"))

ylow  = h.GetYaxis().FindBin(5)
yhigh = h.GetYaxis().FindBin(500)

p = h.ProfileX(h.GetName() + "_pfx", ylow, yhigh)
p.Rebin(5)


xlow, xhigh = 100, 1500
sinus = ROOT.TF1("sinus", "[0]*sin(TMath::TwoPi()*x/[1] + [2]) + [3]", xlow, xhigh)

sinus.SetParName(0, "amplitude")
sinus.SetParName(1, "period")
sinus.SetParName(2, "phase")
sinus.SetParName(3, "offset")

sinus.SetParameter(0, 0.5)
sinus.SetParameter(1, 500)
sinus.SetParameter(2, 1.2)
sinus.SetParameter(3, 1.5)

sinus.SetParLimits(0, 0.4, 0.7);
sinus.SetParLimits(1, 450, 550);
sinus.SetParLimits(2, 1, 2);
sinus.SetParLimits(3, 0, 2);


p.Fit("sinus", "", "", xlow, xhigh)



