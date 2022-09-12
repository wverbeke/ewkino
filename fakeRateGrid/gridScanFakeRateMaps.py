##########################################################################
# script to scan over parallel fake rate outputs and do some diagnostics #
##########################################################################

import sys
import os
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from copy import copy
import ROOT
# local imports
from gridMCFakeRateMeasurement import get_conf
from gridScanClosureTest import plotscan


### help functions ###

def histtoarray( hist ):
    ### convert a TH2D to a simple numpy array 
    # (ignoring uncertainties, outerflow and nonuniform bin widths)
    nxbins = hist.GetNbinsX()
    nybins = hist.GetNbinsY()
    res = np.zeros((nxbins,nybins))
    for i in range(nxbins):
	for j in range(nybins):
	    res[i,j] = hist.GetBinContent(i+1,j+1)
    return res

def getminmaxavg( hist ):
    ### get minimum, maximum, and average of a 2D hist
    arr = histtoarray(hist)
    hmin = np.amin(arr)
    hmax = np.amax(arr)
    havg = np.mean(arr)
    return (hmin, hmax, havg)


if __name__=='__main__':

    # global settings
    testrun = False
    colorbartitle = 'Average fake rate'
    xdimtitle = 'lepton ptRatio threshold'
    ydimtitle = 'lepton closest jet deepFlavor threshold'
    zdimtitle = 'electron MVA scenario'
    
    # get configuration
    conf = get_conf()
    years = conf['years']
    flavors = conf['flavors']
    ptRatioCuts = conf['ptRatioCuts']
    deepFlavorCuts = conf['deepFlavorCuts']
    extraCuts = conf['extraCuts']

    # modify configuration for testrun
    if testrun:
	years = [years[0]]
	flavors = [flavors[0]]

    # initializations
    outputfile = 'outputscan_fakerate.png'
    resmaps = {}
    restextmaps = {}
    for year in years:
	for flavor in flavors:
	    name = '{}_{}'.format(year,flavor)
	    resmaps[name] = np.zeros((len(ptRatioCuts),len(deepFlavorCuts),len(extraCuts)))
	    restextmaps[name] = np.empty((len(ptRatioCuts),len(deepFlavorCuts),len(extraCuts)),dtype='S16')

    # loop over cut values
    for i,ptRatioCut in enumerate(ptRatioCuts):
        for j,deepFlavorCut in enumerate(deepFlavorCuts):
	    for k,extraCut in enumerate(extraCuts):
		# check the  working directory
		ptrCutName = str(ptRatioCut).replace('.','p')
		dfCutName = str(deepFlavorCut).replace('.','p')
		exCutName = str(extraCut)
		wdirname = 'output_ptRatio{}_deepFlavor{}_extraCut{}'.format(
				    ptrCutName, dfCutName, exCutName)
		wdirname = os.path.join(wdirname, 'fakeRateMaps')
		if not os.path.exists(wdirname):
		    msg = 'ERROR: working directory {}'.format(wdirname)
		    msg += ' does not exist, skipping...'
		    print(msg)
		    continue
		# loop over years and flavours
		for year in years:
		    for flavor in flavors:
			name = '{}_{}'.format(year,flavor)
			# read the fake rate map
			fname = 'fakeRateMap_MC_{}_{}.root'.format(flavor,year)
			fname = os.path.join(wdirname,fname)
			if not os.path.exists(fname):
			    msg = 'ERROR: file {}'.format(fname)
			    msg += ' does not exist, skipping...'
			    print(msg)
			    restextmaps[name][i,j,k] = 'failed'
			    continue
			f = ROOT.TFile.Open(fname)
			# read the fake rate histogram
			frmap = f.Get('fakeRate_{}_{}'.format(flavor,year))
			# get minimum, maximum and average
			(frmin, frmax, fravg) = getminmaxavg(frmap)
			# add average to result
                        resmaps[name][i,j,k] = fravg
			# define text to write 
			# (note that the length must not be longer than initialized above!)
			restext = '{:.2f} ({:.2f}-{:.2f})'.format(fravg,frmin,frmax)
			restextmaps[name][i,j,k] = restext

    ### make plots
    for name,res in resmaps.items():
        print('making plot for {}'.format(name))
        title = 'Fake rate summary for '+name.replace('_',' / ')
        thisoutputfile = os.path.splitext(outputfile)[0]+'_{}.png'.format(name)
        plotscan(res, thisoutputfile,
            xdimtitle=xdimtitle, ydimtitle=ydimtitle, zdimtitle=zdimtitle,
            xdimvalues=ptRatioCuts, ydimvalues=deepFlavorCuts, zdimvalues=extraCuts,
            title=title, colorbartitle=colorbartitle,
            bintext=restextmaps[name])
