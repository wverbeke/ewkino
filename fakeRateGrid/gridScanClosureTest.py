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

if __name__=='__main__':

    # global settings
    variable = 'leptonPtLeading'
    divfunc = chi2divergence
    
    # get configuration
    conf = get_conf()
    years = conf['years']
    flavors = conf['flavors']
    ptRatioCuts = conf['ptRatioCuts']
    deepFlavorCuts = conf['deepFlavorCuts']
    processes = ['TT','DY']

    # initializations
    outputfile = 'output_{}_{}.png'.format(variable,divfunc.__name__)
    divmaps = {}
    for year in years:
	for flavor in flavors:
	    for process in processes:
		name = '{}_{}_{}'.format(year,flavor,process)
		divmaps[name] = np.zeros((len(ptRatioCuts),len(deepFlavorCuts)))

    # loop over cut values
    condorclustercmds = []
    for i,ptRatioCut in enumerate(ptRatioCuts):
        for j,deepFlavorCut in enumerate(deepFlavorCuts):
            # check the  working directory
            ptrCutName = str(ptRatioCut).replace('.','p')
            dfCutName = str(deepFlavorCut).replace('.','p')
            wdirname = 'output_ptRatio{}_deepFlavor{}'.format(ptrCutName, dfCutName)
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
			divmaps[name][i,j] = div
			# printouts for testing
			print('--------')
			print(ptRatioCut)
			print(deepFlavorCut)
			print(year)
			print(flavor)
			print(process)
			print(div)

    # make plots
    for name,div in divmaps.items():
	print('making plot for {}'.format(name))
	fig,ax = plt.subplots()
	# make the color scale
	my_norm = mpl.colors.Normalize(vmin=1e-12, vmax=np.amax(div), clip=False)
	my_cmap = copy(mpl.cm.get_cmap('cool'))
	my_cmap.set_under(color='white')
	cobject = mpl.cm.ScalarMappable(norm=my_norm, cmap=my_cmap)
	cobject.set_array([]) # ad-hoc bug fix
	# make the plot
	ax.imshow(div.T, interpolation='none', cmap=my_cmap, norm=my_norm)
	# make a color bar
        cbar = fig.colorbar(cobject, ax=ax, fraction=0.035, pad=0.04)
	cbar.set_ticks([])
        cbar.set_label('Chi2 divergence', rotation=270, labelpad=15)
	# set the tick marks and axis titles
	xtickpos = np.linspace(0,len(ptRatioCuts)-1,num=len(ptRatioCuts))
	xtickvalues = ptRatioCuts
	ytickpos = np.linspace(0,len(deepFlavorCuts)-1,num=len(deepFlavorCuts))
        ytickvalues = deepFlavorCuts
	ax.set_xlabel('ptRatio threshold')
	ax.set_ylabel('deepFlavor threshold')
	ax.set_xticks(xtickpos)
	ax.set_xticklabels(xtickvalues)
	ax.set_yticks(ytickpos)
	ax.set_yticklabels(ytickvalues)
	# make a title
	ax.set_title('Closure for '+name.replace('_',' / '))
	# write the bin values
	for i in range(len(ptRatioCuts)):
	    for j in range(len(deepFlavorCuts)):
		if div[i,j] < 1e-12: continue
		txt = '{:.2E}'.format(div[i,j])
		ax.text(i, j, txt, horizontalalignment='center', verticalalignment='center', fontsize=8)
	# save the figure
	thisoutputfile = os.path.splitext(outputfile)[0]+'_{}.png'.format(name)
	fig.savefig(thisoutputfile)
