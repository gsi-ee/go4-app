
import go4py
import ROOT
import matplotlib.pyplot as plt


def hist2list(h):
    """
    Read points from histogram h into two lists (x and y coordinates)
    Format is chosen such that matplotlib.pyplot.step() can be used for plotting
    hence (x, y) == (low edge, content)
    """
    xs = []
    ys = []
    for i in range(h.GetNbinsX()):
        x = h.GetBinLowEdge(i+1)
        y = h.GetBinContent(i+1)
        xs.append(x)
        ys.append(y)
    return xs, ys


def eval_func(xs, tf):
    """
    Evaluate the TF1 tf for all values in xs, return list of y values
    """
    ys = []
    for x in xs:
        y = tf.Eval(x)
        ys.append(y)
    return ys



hist = next(go4py.MatchingObjects("a_1d_hist", "Histograms"))
hist.Rebin(5)


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



