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
    # - bounds: a list of tuples with lower and upper bound for all function parameters;
    #           lenght and order must correspond to the chosen fitfunction!

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
    (roofitfunc,otherstuff) = fitfunction(x, parameters, bounds=bounds)
    norm = otherstuff['norm']

    # do the fit
    # options: 
    # - Extended add extended likelihood term
    #   (can use false since already explicitly in RooExtendPdf?)
    # - Save: return a RooFitResult object
    #   (not used for now but keep)
    # - SumW2Error: take weights into account for error calculation
    # - Hesse: whether to run Hesse after Migrad (switchin off does not seem to have any effect)
    roofitresult = roofitfunc.fitTo(roofithist, ROOT.RooFit.Extended(False), 
    				 ROOT.RooFit.Save(True),
    				 ROOT.RooFit.SumW2Error(True) )
    roofitresult.Print()

    # make a plot
    # -> plot seems to be ok at this stage!
    c = ROOT.TCanvas('c','c',800,600)
    xframe = x.frame()
    roofithist.plotOn(xframe)
    roofitfunc.plotOn(xframe)
    xframe.Draw()
    c.Draw()
    c.Update()
    c.SaveAs('test_roofit.png')

    # transform to a TF1
    #params = roofitfunc.getParameters(ROOT.RooArgSet(x))
    params = roofitresult.floatParsFinal()
    #params = roofitresult.floatParsInit()
    func = roofitfunc.asTF(ROOT.RooArgList(x),ROOT.RooArgList(params))

    # extend the contents of otherstuff
    otherstuff['fittedfunction'] = roofitfunc
    otherstuff['norm'] = norm
    otherstuff['xaxvar'] = x
    otherstuff['roofithist'] = roofithist
    otherstuff['params'] = params

    return (func,otherstuff)


def fitroofit_alternative(histogram, fitfuncstr, fitfuncname):
    ### alternative attempt, based on
    # http://dpnc.unige.ch/~sfyrla/teaching/Statistics/handsOn1.html

    # get some histogram info
    nbins = histogram.GetNbinsX()
    xlow = histogram.GetBinLowEdge(1)
    xhigh = histogram.GetBinLowEdge(nbins)+histogram.GetBinWidth(nbins)

    # make a workspace and x-axis variable
    workspace = ROOT.RooWorkspace()
    workspace.factory('x[{},{}]'.format(xlow,xhigh))
    x = workspace.var('x')
    roofithist = ROOT.RooDataHist('data','data',ROOT.RooArgList(x),histogram)

    # make the fit function
    workspace.factory(fitfuncstr)
    workspace.factory('SUM::fitfunc(norm[1.0,0,1000]*{})'.format(fitfuncname))
    roofitfunc = workspace.pdf('fitfunc')

    # do the fit
    roofitresult = roofitfunc.fitTo(roofithist, ROOT.RooFit.Extended(False),
					 ROOT.RooFit.Save(True),
					 ROOT.RooFit.SumW2Error(True) )
    roofitresult.Print()
    
    # make a plot
    # -> plot seems to be ok at this stage!
    c = ROOT.TCanvas('c','c',800,600)
    xframe = x.frame()
    roofithist.plotOn(xframe)
    roofitfunc.plotOn(xframe)
    xframe.Draw()
    c.Draw()
    c.Update()
    c.SaveAs('test_roofit.png')

    # transform to a TF1
    params = roofitfunc.getParameters(ROOT.RooArgSet(x))
    prodset = ROOT.RooArgList(roofitfunc)
    fitproduct = ROOT.RooProduct("fitproduct","fitproduct",prodset)
    func = fitproduct.asTF(ROOT.RooArgList(x),ROOT.RooArgList(params))
    print(func)
    print(func.Eval(120.))

    # extend the contents of otherstuff
    otherstuff = {}
    otherstuff['workspace'] = workspace
    otherstuff['fittedfunction'] = roofitfunc
    otherstuff['fitproduct'] = fitproduct
    otherstuff['xaxvar'] = x
    otherstuff['roofithist'] = roofithist
    otherstuff['params'] = params
    otherstuff['prodset'] = prodset

    return (func,otherstuff)


def fitroofit_alternative2(histogram, fitfunction, parameters, bounds=None):
    ### alternative attempt, based on

    # get some histogram info
    nbins = histogram.GetNbinsX()
    xlow = histogram.GetBinLowEdge(1)
    xhigh = histogram.GetBinLowEdge(nbins)+histogram.GetBinWidth(nbins)

    # make a workspace and x-axis variable
    workspace = ROOT.RooWorkspace()
    workspace.factory('x[{},{}]'.format(xlow,xhigh))
    x = workspace.var('x')
    roofithist = ROOT.RooDataHist('data','data',ROOT.RooArgList(x),histogram)

    # make the fit function
    (roofitpdf,otherstuff) = fitfunction(x, parameters, bounds=bounds)
    roofitpdfname = otherstuff['name']
    getattr(workspace,'import')(roofitpdf)
    workspace.factory('SUM::fitfunc(norm[1.0,0.0,20.0]*{})'.format(roofitpdfname))
    roofitfunc = workspace.pdf('fitfunc')

    # do the fit
    roofitresult = roofitfunc.fitTo(roofithist, ROOT.RooFit.Extended(True),
                                         ROOT.RooFit.Save(True),
                                         ROOT.RooFit.SumW2Error(True) )
    roofitresult.Print()

    # make a plot
    c = ROOT.TCanvas('c','c',800,600)
    xframe = x.frame()
    roofithist.plotOn(xframe)
    roofitfunc.plotOn(xframe)
    xframe.Draw()
    c.Draw()
    c.Update()
    c.SaveAs('test_roofit.png')

    # transform to a TF1
    params = roofitfunc.getParameters(ROOT.RooArgSet(x))
    print(params['a1'])
    print(params['norm'])
    func = roofitfunc.asTF(ROOT.RooArgList(x),ROOT.RooArgList(params))
    print(func)
    print(func.Eval(120.))

    # extend the contents of otherstuff
    otherstuff['workspace'] = workspace
    otherstuff['fittedfunction'] = roofitfunc
    otherstuff['xaxvar'] = x
    otherstuff['roofithist'] = roofithist
    otherstuff['params'] = params

    return (func,otherstuff)
