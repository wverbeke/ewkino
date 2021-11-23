#############################################
# small test script for fit.py and plotting #
#############################################

import sys
import os
sys.path.append('../plotting')
import plotfit
sys.path.append('../tools/python')
import histtools as ht
import optiontools as opt

from fitroot import fitroot
from fitscipy import fitscipy
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
    parameters = [0,0,0]

    # get the inpu histogram
    histlist = ht.loadhistograms(options.inputfile, mustcontainall=[options.histname])
    hist = histlist[0]

    # do the fit with root
    fitfunc1 = fitroot( hist, fitfunctype, parameters )
    
    # do the fit with scipy
    fitfunc2 = fitscipy( hist, fitfunctype, parameters )

    # print fitted parameters
    #for i in range(len(parameters)):
    #	print('parameter {}: {}'.format(i, fitfunc1.GetParameter(i)))

    # make an example histogram to add on the plot
    exthist = hist.Clone()
    exthist.Scale(0.5)

    # make a plot without ratio
    figname = options.figname.replace('.png','_noratio.png')
    plotfit.plotfit( hist, [fitfunc1, fitfunc2], histlist=[exthist], figname=figname,
                        xaxtitle='a test x-axis', yaxtitle='a test y-axis',
                        dolegend=True,
                        datalabel='Pseudo-data',
                        funclabellist=['Fit (root)', 'Fit (scipy)'], funccolorlist=None,
			histlabellist=['Some random hist'], histcolorlist=None,
                        logy=False, ymaxlinfactor=1.8, yminlogfactor=0.2, ymaxlogfactor=100,
                        ylims=None, yminzero=False,
                        histdrawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=False, ratiorange=None )

    # make a plot with ratio
    figname = options.figname.replace('.png','_ratio.png')
    plotfit.plotfit( hist, [fitfunc1, fitfunc2], histlist=[exthist], figname=figname,
                        xaxtitle='a test x-axis', yaxtitle='a test y-axis',
                        dolegend=True,
                        datalabel='Pseudo-data',
                        funclabellist=['Fit (root)', 'Fit (scipy)'], funccolorlist=None,
                        histlabellist=['Some random hist'], histcolorlist=None,
                        logy=False, ymaxlinfactor=1.8, yminlogfactor=0.2, ymaxlogfactor=100,
                        ylims=None, yminzero=False,
                        histdrawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=True, ratiorange=None )
