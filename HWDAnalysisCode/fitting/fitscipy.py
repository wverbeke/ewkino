##########################################################################
# main function to perform a fit with a given histogram and fit function #
##########################################################################
# the fit method is scipy curve_fit.
# see fitroot.py for an equivalent function using the ROOT TH1::Fit method!

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
    yerrorup = np.zeros(nbins)
    yerrordown = np.zeros(nbins)
    for i in range(1,nbins+1):
	xdata[i-1] = histogram.GetBinCenter(i)
	ydata[i-1] = histogram.GetBinContent(i)
	yerrorup[i-1] = histogram.GetBinErrorUp(i)
	yerrordown[i-1] = histogram.GetBinErrorLow(i)
	
    return (xdata,ydata,yerrorup,yerrordown)

### main function 

def fitscipy( histogram, fitfunction, parameters,
	      fitrange=None, ignore_empty=False, **kwargs ):
    ### fit a function to a histogram
    # input arguments:
    # - histogram: a TH1 object to perform the fit on
    # - fitfunction: a function object that can be used to create a TF1,
    #                see funcdefs.py for examples
    # - parameters: a list of initial values for all function parameters;
    #               length and order must correspond to the chosen fitfunction!
    # - fitrange: tuple setting the range to be fitted (default: entire histogram range)
    # - ignore_empty: boolean whether to ignore empty bins (zero content, zero error) in the fit
    # - kwargs: arguments passed down to scipy.curve_fit
    #   see https://docs.scipy.org/doc/scipy/reference/generated/scipy.optimize.curve_fit.html

    # get some histogram info
    nbins = histogram.GetNbinsX()
    xlow = histogram.GetBinLowEdge(1)
    xhigh = histogram.GetBinLowEdge(nbins)+histogram.GetBinWidth(nbins)

    # parse arguments
    if fitrange is None: fitrange = (xlow, xhigh)

    # get input data from histogram
    (xdata,ydata,yerrorup,yerrordown) = hist_to_arrays(histogram)

    # take into account fit range
    mask = (xdata>fitrange[0]) & (xdata<fitrange[1])
    xdata = xdata[mask]
    ydata = ydata[mask]
    yerrorup = yerrorup[mask]
    yerrordown = yerrordown[mask]

    # exclude empty bins if requested
    if ignore_empty:
	mask = (ydata!=0) | (yerrorup!=0) | (yerrordown!=0)
	xdata = xdata[mask]
	ydata = ydata[mask]
	yerrorup = yerrorup[mask]
	yerrordown = yerrordown[mask]

    # for now, unclear whether it is possible or desirable to allow asymmetric errors,
    # does not seem to be an option in scipy.curve_fit;
    # so choose largest one for each bin
    # (does not make a difference for data where yerrorup = yerrordown = sqrt(ydata))
    yerror = np.maximum(yerrorup, yerrordown)
    print(ydata)
    print(yerror)

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
    popt,_ = scopt.curve_fit(objective, xdata, ydata, p0=parameters, sigma=yerror)

    # translate the result to a TF1
    fitfunc = ROOT.TF1("fitfunc", fitfunction, fitrange[0], fitrange[1], len(parameters))
    for i in range(len(parameters)):
        fitfunc.SetParameter(i, popt[i])

    return fitfunc
