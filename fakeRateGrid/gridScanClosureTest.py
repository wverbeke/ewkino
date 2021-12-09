###################################################################################
# script to scan over parallel closure test outputs and do some automatic ranking #
###################################################################################

import sys
import os
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from copy import copy
import ROOT
from gridMCFakeRateMeasurement import get_conf

def histtoarray( hist ):
    ### convert a TH1D to a simple numpy array 
    # (ignoring uncertainties, outerflow and nonuniform bin widths)
    nbins = hist.GetNbinsX()
    res = np.zeros(nbins)
    for i in range(nbins):
	res[i] = hist.GetBinContent(i+1)
    return res

def kldivergence( hist1, hist2 ):
    ### calculate Kullback-Leibler divergence between two histograms
    # note: bins with negative or zero contents in one of both histograms will be skipped
    # to do: probably normalization is needed to make the result always positive (?)
    arr1 = histtoarray(hist1)
    arr2 = histtoarray(hist2)
    pos_ids = np.nonzero( (arr1>0) & (arr2>0) )
    arr1 = arr1[pos_ids]
    arr2 = arr2[pos_ids]
    temp = np.divide( arr1, arr2 )
    temp = np.log( temp )
    temp = np.multiply( arr1, temp )
    return np.sum(temp)

def chi2divergence( hist1, hist2 ):
    ### calculate chi2 divergence between two histograms
    # note: bins with negative or zero contents in one of both histograms will be skipped
    arr1 = histtoarray(hist1)
    arr2 = histtoarray(hist2)
    pos_ids = np.nonzero( (arr1>0) & (arr2>0) )
    arr1 = arr1[pos_ids]
    arr2 = arr2[pos_ids]
    temp = np.power( arr1-arr2, 2 )
    temp = np.divide( temp, arr2 )
    return np.sum(temp)

def normchi2divergence( hist1, hist2 ):
    ### calculate normalized chi2 divergence between two histograms
    # note: same remarks as above
    chi2 = chi2divergence( hist1, hist2 )
    ndof = hist1.GetNbinsX()-1
    return chi2/ndof


if __name__=='__main__':

    # global settings
    testrun = True
    variable = 'leptonPtLeading'
    divfunc = chi2divergence
    
    # get configuration
    conf = get_conf()
    years = conf['years']
    flavors = conf['flavors']
    ptRatioCuts = conf['ptRatioCuts']
    deepFlavorCuts = conf['deepFlavorCuts']
    extraCuts = conf['extraCuts']
    processes = ['TT','DY']

    # modify configuration for testrun
    if testrun:
	years = [years[0]]
	flavors = [flavors[0]]
	processes = [processes[0]]

    # initializations
    outputfile = 'output_{}_{}.png'.format(variable,divfunc.__name__)
    divmaps = {}
    for year in years:
	for flavor in flavors:
	    for process in processes:
		name = '{}_{}_{}'.format(year,flavor,process)
		divmaps[name] = np.zeros((len(ptRatioCuts),len(deepFlavorCuts),len(extraCuts)))

    # loop over cut values
    condorclustercmds = []
    for i,ptRatioCut in enumerate(ptRatioCuts):
        for j,deepFlavorCut in enumerate(deepFlavorCuts):
	    for k,extraCut in enumerate(extraCuts):
		# check the  working directory
		ptrCutName = str(ptRatioCut).replace('.','p')
		dfCutName = str(deepFlavorCut).replace('.','p')
		exCutName = str(extraCut)
		wdirname = 'output_ptRatio{}_deepFlavor{}_extraCut{}'.format(
				    ptrCutName, dfCutName, exCutName)
		if not os.path.exists(wdirname):
		    msg = 'ERROR: working directory {}'.format(wdirname)
		    msg += ' does not exist, skipping...'
		    print(msg)
		    continue
		# loop over years and flavours and processes
		for year in years:
		    for flavor in flavors:
			for process in processes:
			    # read the histograms
			    fname = 'closurePlots_MC_{}_{}_{}.root'.format(process,year,flavor)
			    fname = os.path.join(wdirname,fname)
			    if not os.path.exists(fname):
				msg = 'ERROR: file {}'.format(fname)
				msg += ' does not exist, skipping...'
				print(msg)
				continue
			    f = ROOT.TFile.Open(fname)
			    pname = '{}_{}_{}_{}_predicted'.format(variable,process,year,flavor)
			    oname = '{}_{}_{}_{}_observed'.format(variable,process,year,flavor)
			    predicted = f.Get(pname)
			    observed = f.Get(oname)
			    # do similariy check
			    div = divfunc( predicted, observed )
			    name = '{}_{}_{}'.format(year,flavor,process)
			    divmaps[name][i,j,k] = div
			    # printouts for testing
			    print('--------')
			    print(ptRatioCut)
			    print(deepFlavorCut)
			    print(extraCut)
			    print(year)
			    print(flavor)
			    print(process)
			    print(div)

    ### make plots

    # initializations
    vmin = 1e-12
    vmax = 100
    colormap = 'cool'

    for name,div in divmaps.items():
	print('making plot for {}'.format(name))
	# transform div from 3D array into 2D array
	(nx,ny,nz) = div.shape
	div2d = np.zeros((nx,ny*nz))
	for k in range(nz):
	    div2d[:,k*ny:(k+1)*ny] = div[:,:,k]
	# determine aspect ratio and figure size
	aspect_ratio = div2d.shape[1]/div2d.shape[0]
	#figsize = (6,6*aspect_ratio) # figure too stretched
	figsize = (8,12)
	# initialize the figure
	fig,ax = plt.subplots(figsize=figsize)
	# make the color scale
	if vmin is None: vmin = np.amin(div2d)
	if vmax is None: vmax = np.amax(div2d)
	print('setting vmin to {}'.format(vmin))
	print('setting vmax to {}'.format(vmax))
	my_norm = mpl.colors.Normalize(vmin=vmin, vmax=vmax, clip=False)
	my_cmap = copy(mpl.cm.get_cmap(colormap))
	my_cmap.set_under(color='white')
	cobject = mpl.cm.ScalarMappable(norm=my_norm, cmap=my_cmap)
	cobject.set_array([]) # ad-hoc bug fix
	# make the plot
	ax.imshow(div2d.T, interpolation='none', cmap=my_cmap, norm=my_norm, aspect='auto')
	# add rectangles
	for k in range(nz):
	    rec = mpl.patches.Rectangle( (-0.5,k*ny-0.5), nx, ny, 
		    linewidth=2, edgecolor='black', facecolor='none' )
	    ax.add_patch(rec)
	# make a color bar
        cbar = fig.colorbar(cobject, ax=ax, fraction=0.035, pad=0.04)
	cbar.set_ticks([])
        cbar.set_label('Chi2 divergence', rotation=270, labelpad=15)
	# set the tick marks and axis titles
	xtickpos = np.linspace(0,len(ptRatioCuts)-1,num=len(ptRatioCuts))
	xtickvalues = ptRatioCuts
	ytickpos = np.linspace(0,nz*len(deepFlavorCuts)-1,num=nz*len(deepFlavorCuts))
        ytickvalues = nz*deepFlavorCuts
	ax.set_xlabel('ptRatio threshold')
	ax.set_ylabel('deepFlavor threshold')
	ax.set_xticks(xtickpos)
	ax.set_xticklabels(xtickvalues)
	ax.set_yticks(ytickpos)
	ax.set_yticklabels(ytickvalues)
	# add the labels for the third dimension
	for k in range(nz):
	    txt = 'electron MVA scenario {}'.format(extraCuts[k])
	    temp = ax.text( nx-0.6, k*ny, txt, horizontalalignment='right', 
				verticalalignment='top', fontsize=10)
	    temp.set_bbox(dict(facecolor='white', edgecolor='black', alpha=0.7))
	# make a title
	ax.set_title('Closure for '+name.replace('_',' / '))
	# write the bin values
	for i in range(nx):
	    for j in range(ny*nz):
		if div2d[i,j] < 1e-12: continue
		txt = '{:.2E}'.format(div2d[i,j])
		ax.text(i, j, txt, horizontalalignment='center', verticalalignment='center', fontsize=8)
	# save the figure
	thisoutputfile = os.path.splitext(outputfile)[0]+'_{}.png'.format(name)
	fig.savefig(thisoutputfile)
