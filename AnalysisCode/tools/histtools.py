########################################################################
# some small tools for working with histograms and lists of histograms #
########################################################################

import os
import numpy as np
from array import array
import ROOT

### histogram reading and loading ###

def loadallhistograms(histfile,mustcontain=[]):
    ### read a root file containing histograms and load all histograms to a list
    # if mustcontain is not empty, histogram names are required to contain all elements in it.
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        # check if histogram is readable
        try:
            nentries = hist.GetEntries() # maybe replace by more histogram-specific function
            nbins = hist.GetNbinsX()
            hist.SetDirectory(0)
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
            continue
        keep = True
        if len(mustcontain)>0:
            for tag in mustcontain:
                if not tag in hist.GetName(): keep = False; break
        if not keep: continue
        # add hist to dict
        histlist.append(hist)
    f.Close()
    return histlist

### histogram clipping ###

def cliphistogram(hist):
    ### clip a histogram to minimum zero
    for i in range(0,hist.GetNbinsX()+2):
        if hist.GetBinContent(i)<0:
            hist.SetBinContent(i,0)
            hist.SetBinError(i,0)
    # check if histogram is empty after clipping and if so, fill it with dummy value
    if hist.GetSumOfWeights()<1e-12: hist.SetBinContent(1,1e-6)

def cliphistograms(histlist):
    ### apply cliphistogram on all histograms in a list
    for hist in histlist: cliphistogram(hist)

def clipallhistograms(histfile,mustcontain=[]):
    ### apply cliphistogram on all histograms in a file
    histlist = loadallhistograms(histfile)
    if len(mustcontain)==0:
	cliphistograms(histlist)
    else:
	for hist in histlist:
	    clip = True
	    for tag in mustcontain:
		if not tag in hist.GetName():
		    clip = False
		    break
	    if clip: cliphistogram(hist)
    tempfilename = histfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
        hist.Write()
    f.Close()
    os.system('mv '+tempfilename+' '+histfile)

### histogram conversion ###

def tgraphtohist( graph ):

    # get list of x values and sort them
    xvals = []
    for i in range(graph.GetN()): xvals.append(graph.GetX()[i])
    xvals = np.array(xvals)
    sortedindices = np.argsort(xvals)
    # make bins
    bins = []
    for i in sortedindices: bins.append(graph.GetX()[i]-graph.GetErrorXlow(i))
    bins.append(graph.GetX()[i]+graph.GetErrorXhigh(i))
    # make histogram
    hist = ROOT.TH1D("","",len(bins)-1,array('f',bins))
    # set bin contents
    for i in range(1,hist.GetNbinsX()+1):
        bincontent = graph.GetY()[sortedindices[i-1]]
        binerror = max(graph.GetErrorYlow(sortedindices[i-1]),
                        graph.GetErrorYhigh(sortedindices[i-1]))
        hist.SetBinContent(i,bincontent)
        hist.SetBinError(i,binerror)
    hist.SetName(graph.GetName())
    hist.SetTitle(graph.GetTitle())
    return hist
