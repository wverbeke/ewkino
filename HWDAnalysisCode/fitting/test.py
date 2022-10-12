#############################################
# small test script for fit.py and plotting #
#############################################

import sys
import os
import ROOT
sys.path.append('../plotting')
import plotfit
sys.path.append('../tools/python')
import histtools as ht
import optiontools as opt

from fitroot import fitroot
from fitscipy import fitscipy
from fitroofit import fitroofit, fitroofit_alternative, fitroofit_alternative2
import funcdefs as fd


if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path', required=True) )
    options.append( opt.Option('histname', required=True) )
    options.append( opt.Option('figname', vtype='path', required=True) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # set fit function properties
    fitfunctype = fd.polynomial
    rffitfunctype = fd.RooFitPolynomialPdf
    parameters = [1.,1.]

    # get the input histogram
    histlist = ht.loadhistograms(options.inputfile, mustcontainall=[options.histname])
    hist = histlist[0]
    nbins = hist.GetNbinsX()
    xlow = hist.GetBinLowEdge(1)
    xhigh = hist.GetBinLowEdge(nbins)+hist.GetBinWidth(nbins)

    # optional: set the histogram to a simple shape
    for i in range(0, hist.GetNbinsX()+2):
    	xval = hist.GetBinCenter(i)
    	yval = 15
    	yerror = 0.1
    	hist.SetBinContent(i,yval)
    	hist.SetBinError(i,yerror)

    # optional: set a region to zero
    #for i in range(0, hist.GetNbinsX()/2):
    #	hist.SetBinContent(i,0)
    #	hist.SetBinError(i,0)

    # initialize lists of functions and labels
    funclist = []
    labellist = []

    ### fitting with default Root ###

    # do the fit with root
    #funclist.append( fitroot( hist, fitfunctype, parameters, ignore_empty=True ) )
    #labellist.append('Fit (root)')
    
    # do the fit with root in limited range
    #funclist.append( fitroot( hist, fitfunctype, parameters, fitrange=(130,150) ) )
    #labellist.append('Fit (root, limited)')

    ### fitting with scipy ### 
    
    # do the fit with scipy
    #funclist.append( fitscipy( hist, fitfunctype, parameters, ignore_empty=True ) )
    #labellist.append('Fit (scipy)')
    
    # do the fit with scipy in limited range
    #funclist.append( fitscipy( hist, fitfunctype, parameters, fitrange=(110,120) ) )
    #labellist.append('Fit (scipy, limited)')

    ### fitting with RooFit ###
    bounds = [(0.,5.),(0.,5.)]
    #(roofitfunc,otherstuff) = fitroofit(hist, rffitfunctype, parameters, bounds=bounds)
    (roofitfunc,otherstuff) = fitroofit_alternative(hist, 
    				'RooPolynomial::poly(x,{a1[10,0,15]})', 'poly')
    #(roofitfunc,otherstuff) = fitroofit_alternative2(hist, rffitfunctype, parameters, bounds=bounds)
    print(roofitfunc)
    print(roofitfunc.Eval(150.))
    funclist.append( roofitfunc )
    labellist.append('Fit (RooFit)')
    

    # make an example histogram to add on the plot
    exthist = hist.Clone()
    exthist.Scale(0.5)

    # make a plot without ratio
    figname = options.figname.replace('.png','_noratio.png')
    plotfit.plotfit( hist, funclist, histlist=[exthist], figname=figname,
                        xaxtitle='a test x-axis', yaxtitle='a test y-axis',
                        dolegend=True,
                        datalabel='Pseudo-data',
                        funclabellist=labellist, funccolorlist=None,
			histlabellist=['Some random hist'], histcolorlist=None,
                        logy=False, ymaxlinfactor=1.8, yminlogfactor=0.2, ymaxlogfactor=100,
                        ylims=None, yminzero=False,
                        histdrawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=False, ratiorange=None )

    # make a plot with ratio
    figname = options.figname.replace('.png','_ratio.png')
    plotfit.plotfit( hist, funclist, histlist=[exthist], figname=figname,
                        xaxtitle='a test x-axis', yaxtitle='a test y-axis',
                        dolegend=True,
                        datalabel='Pseudo-data',
                        funclabellist=labellist, funccolorlist=None,
                        histlabellist=['Some random hist'], histcolorlist=None,
                        logy=False, ymaxlinfactor=1.8, yminlogfactor=0.2, ymaxlogfactor=100,
                        ylims=None, yminzero=False,
                        histdrawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=True, ratiorange=None )
