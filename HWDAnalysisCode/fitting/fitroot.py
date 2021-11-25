##########################################################################
# main function to perform a fit with a given histogram and fit function #
##########################################################################
# the fit method is the standard ROOT method TH1::Fit
# see fitscipy.py for an equivalent fitter using the scipy curve_fit method!

import sys
import os
import ROOT

def fitroot(histogram, fitfunction, parameters, 
	    fitrange=None, ignore_empty=False, optionstring='LQNR'):
    ### fit a function to a histogram
    # input arguments:
    # - histogram: a TH1 object to perform the fit on
    # - fitfunction: a function object that can be used to create a TF1,
    #                see funcdefs.py for examples
    # - parameters: a list of initial values for all function parameters;
    #               length and order must correspond to the chosen fitfunction!
    # - fitrange: tuple setting the range of the fitted funcion (default: entire histogram range)
    # - ignore_empty: boolean whether to ignore empty bins (zero content, zero error)
    #                 (effectively just sets the method to chi2)
    # - optionstring: string of options passed to ROOT.TH1.Fit
    #   see https://root.cern.ch/root/htmldoc/guides/users-guide/FittingHistograms.html
    #   default: Q = quiet (minimal printing)
    #            L = log likelihood instead of chi2 (when the histogram represents counts)
    #            N = do not plot the result
    #		 R = use the fitrange argument to determine fitting range
    #                (if not provided, fitrange is only the display range, not the actual fit range!)

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

    # ignore empty bins if requested
    if ignore_empty:
	if( 'WL' in optionstring ):
	    print('WARNING: weighted log-likelihood is replaced by chi2 method'
		    +' since ignore_empty was set to True!')
	    optionstring = optionstring.replace('WL','')
	elif( 'L' in optionstring ):
	    print('WARNING: log-likelihood is replaced by chi2 method'
                    +' since ignore_empty was set to True!')
            optionstring = optionstring.replace('L','')

    # do the fit
    fitresult = histogram.Fit(fitfunc, optionstring)
    return fitfunc
