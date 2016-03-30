
import go4py
from go4py.convert import hist2list, eval_func
import ROOT
import matplotlib.pyplot as plt


hist = next(go4py.MatchingObjects("His1", "Histograms"))
#hist.Rebin(5)


xlow, xhigh = 0, 10000
f = ROOT.TF1("simple_func", "[0]*x", xlow, xhigh)
f.SetParName(0, "slope")
f.SetParameter(0, 0.1)


xs, ys = hist2list(hist)
#xs, ys = zip(*[(x, y) for x, y in zip(xs, ys) if y > 35])

yfs = eval_func(xs, f)


plt.xlim([xlow, xhigh])
plt.xlabel('x-axis')
plt.ylabel('y-axis')

plt.step(xs, ys)
plt.step(xs, yfs)

plt.show()



