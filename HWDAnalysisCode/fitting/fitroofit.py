##########################################################################
# main function to perform a fit with a given histogram and fit function #
##########################################################################
# the fit method is using RooFit.
# see fitroot.py for an equivalent function using the ROOT TH1::Fit method!
# see fitscipy.py for an equivalent function using the scipy curve_fit method!

import sys
import os
import ROOT

def fitroofit(histogram, fitfunction, parameters, bounds=None):
    ### fit a function to a histogram
    # input arguments:
    # - histogram: a TH1 object to perform the fit on
    # - fitfunction: a function object that can be used to create a RooFit RooExtendPdf object,
    #                see funcdefs.py for examples
    # - parameters: a list of initial values for all function parameters;
    #               length and order must correspond to the chosen fitfunction!

    # get some histogram info
    nbins = histogram.GetNbinsX()
    xlow = histogram.GetBinLowEdge(1)
    xhigh = histogram.GetBinLowEdge(nbins)+histogram.GetBinWidth(nbins)

    # convert the histogram to a RooDataHist
    x = ROOT.RooRealVar("x","x",xlow,xhigh)
    roofithist = ROOT.RooDataHist("roofithist","roofithist",ROOT.RooArgList(x),histogram)

    # get a fit function instance
    # note: otherstuff contains all objects that are not needed
    #       but that need to be returned in order to be kept in memory 
    #       and avoid nullptr exceptions
    (roofitfunc,norm,otherstuff) = fitfunction(x, parameters, bounds=bounds)

    # do the fit
    # options: 
    # - Extended add extended likelihood term
    #   (can use false since already explicitly in RooExtendPdf?)
    # - Save: return a RooFitResult object
    #   (not used for now but keep)
    # - SumW2Error: take weights into account for error calculation
    # - Hesse: whether to run Hesse after Migrad (switchin off does not seem to have any effect)
    roofitfunc.fitTo(roofithist, ROOT.RooFit.Extended(False), 
    				 ROOT.RooFit.Save(True),
    				 ROOT.RooFit.SumW2Error(True) )


    # transform to a TF1
    params = roofitfunc.getParameters(ROOT.RooArgSet(x))
    prodset = ROOT.RooArgList(roofitfunc)
    prodset.add(norm)
    fitproduct = ROOT.RooProduct("fitproduct","fitproduct",prodset)
    func = fitproduct.asTF(ROOT.RooArgList(x),ROOT.RooArgList(params))

    # extend the contents of otherstuff
    otherstuff['fittedfunction'] = roofitfunc
    otherstuff['fitproduct'] = fitproduct
    otherstuff['norm'] = norm
    otherstuff['xaxvar'] = x
    otherstuff['roofithist'] = roofithist
    otherstuff['params'] = params
    otherstuff['prodset'] = prodset

    return (func,otherstuff)