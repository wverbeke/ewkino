##########################################################################
# main function to perform a fit with a given histogram and fit function #
##########################################################################
# the fit method is scipy curve_fit.
# see fitroot.py for an equivalent function using the ROOT TH1::Fit method!
#   note: both functions have the same signature and are intended to be fully compatible,
#         all needed conversions are done "under the hood".

# to do: find a way to take into account the fit range (currently ignored)
# to do: find a way to take into account optionstring (currently ignored)

import sys
import os
import numpy as np
import scipy.optimize as scopt
import ROOT

### help functions

def hist_to_arrays( histogram ):
    ### convert a TH1 into arrays of x- and y-data
    # todo: also return the bin errors and see how it can be propagated to the fit
    nbins = histogram.GetNbinsX()
    xdata = np.zeros(nbins)
    ydata = np.zeros(nbins)
    for i in range(1,nbins+1):
	xdata[i-1] = histogram.GetBinCenter(i)
	ydata[i-1] = histogram.GetBinContent(i)
    return (xdata,ydata)

### main function 

def fitscipy( histogram, fitfunction, parameters,
	      fitrange=None, optionstring='LQ0' ):
    ### fit a function to a histogram
    # input arguments:
    # - histogram: a TH1 object to perform the fit on
    # - fitfunction: a function object that can be used to create a TF1,
    #                see funcdefs.py for examples
    # - parameters: a list of initial values for all function parameters;
    #               length and order must correspond to the chosen fitfunction!
    # - fitrange: tuple setting the range to be fitted (default: entire histogram range)
    # - optionstring: string of options passed to ROOT.TH1.Fit

    # get some histogram info
    nbins = histogram.GetNbinsX()
    xlow = histogram.GetBinLowEdge(1)
    xhigh = histogram.GetBinLowEdge(nbins)+histogram.GetBinWidth(nbins)

    # parse arguments
    if fitrange is None: fitrange = (xlow, xhigh)

    # get input data from histogram
    (xdata,ydata) = hist_to_arrays(histogram)

    # create the fit function automatially based on the implementation in root format
    # root format: name(x, params) where x is a list of length 1 and params a list of parameters
    # scipy format: name(x, param1, param2, ...) where all arguments are numbers.
    paramnames = []
    for i in range(len(parameters)): paramnames.append('param{}'.format(i))
    def objective(x, *paramnames):
	thisparameters = []
	for i in range(len(paramnames)): thisparameters.append(paramnames[i])
	return fitfunction([x], thisparameters)

    # do the fit
    popt,_ = scopt.curve_fit(objective, xdata, ydata, p0=parameters)

    # translate the result to a TF1
    fitfunc = ROOT.TF1("fitfunc", fitfunction, fitrange[0], fitrange[1], len(parameters))
    for i in range(len(parameters)):
        fitfunc.SetParameter(i, popt[i])

    return fitfunc
