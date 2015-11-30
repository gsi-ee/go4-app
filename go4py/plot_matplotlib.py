
import go4py
import ROOT
import matplotlib.pyplot as plt


def hist2array(h):
	xs = []
	ys = []
	for i in range(h.GetNbinsX()):
		x = hist.GetBinLowEdge(i+1)
		y = hist.GetBinContent(i+1)
		xs.append(x)
		ys.append(y)
	return xs, ys


def eval_func(xs, tf):
	ys = []
	for x in xs:
		y = tf.Eval(x)
		ys.append(y)
	return ys



hist = next(go4py.MatchingObjects("TdcChan2", "Histograms"))
hist.Rebin(5)


xlow, xhigh = 0, 10000
f = ROOT.TF1("simple_func", "[0]*x", xlow, xhigh)
f.SetParName(0, "slope")
f.SetParameter(0, 0.1)


xs, ys = hist2array(hist)
#xs, ys = zip(*[(x, y) for x, y in zip(xs, ys) if y > 35])

yfs = eval_func(xs, f)


plt.xlim([xlow, xhigh])
plt.xlabel('x-axis')
plt.ylabel('y-axis')

plt.step(xs, ys)
plt.step(xs, yfs)

plt.show()



