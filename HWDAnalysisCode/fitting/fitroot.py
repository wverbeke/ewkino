##########################################################################
# main function to perform a fit with a given histogram and fit function #
##########################################################################
# the fit method is the standard ROOT method TH1::Fit
# see fitscipy.py for an equivalent fitter using the scipy curve_fit method!

import sys
import os
import ROOT

def fitroot(histogram, fitfunction, parameters, 
	    fitrange=None, optionstring='LQ0'):
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
	
    # create the fit function
    fitfunc = ROOT.TF1("fitfunc", fitfunction, fitrange[0], fitrange[1], len(parameters))
    for i, initialvalue in enumerate(parameters):
	fitfunc.SetParameter(i, initialvalue)

    # do the fit
    fitresult = histogram.Fit(fitfunc, optionstring)
    return fitfunc
