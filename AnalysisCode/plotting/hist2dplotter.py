######################################################################
# a Python translation of ewkino/plotting/plotCode.cc/plot2DHistogam #
######################################################################

import ROOT
import sys
import numpy as np
import json
import os
import plottools as tools

def loadhistograms(histfile,mustcontain=[],mustnotcontain=[]):
    # load histograms from a root file.
    # 'histfile' is a string containing the path to the input root file.
    # the output is a list of histograms
    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        # check if histogram is readable
        try:
            bincontent = hist.GetBin(1,1)
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
	    continue
        # check if histogram needs to be included
        keep = True
        for el in mustcontain:
            if el not in hist.GetTitle(): keep = False; break;
        for el in mustnotcontain:
            if el in hist.GetTitle(): keep = False; break;
        if not keep: continue;
        # add hist to list
	hist.SetDirectory(0)
        histlist.append(hist)
	# make sure the name corresponds to the one displayed in rootbrowser
	hist.SetName(key.GetName())
    return histlist

def plot2dhistogram(hist, outfilepath, drawoptions='colztexte'):
    
    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    
    # set global properties
    cheight = 500 # height of canvas
    cwidth = 500 # width of canvas
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
    # title offset
    ytitleoffset = 1.5
    xtitleoffset = 1.5
    # margins:
    topmargin = 0.05
    bottommargin = 0.15
    leftmargin = 0.15
    rightmargin = 0.15


    # create canvas
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    c1.SetTopMargin(topmargin)
    c1.SetBottomMargin(bottommargin)
    c1.SetLeftMargin(leftmargin)
    c1.SetRightMargin(rightmargin)
    
    # set offset and label size
    hist.GetXaxis().SetTitleOffset(xtitleoffset)
    hist.GetXaxis().SetTitleFont(axtitlefont)
    hist.GetXaxis().SetTitleSize(axtitlesize)
    hist.GetYaxis().SetTitleOffset(ytitleoffset)
    hist.GetYaxis().SetTitleFont(axtitlefont)
    hist.GetYaxis().SetTitleSize(axtitlesize)

    # draw
    hist.Draw( drawoptions )

    # save the plot
    c1.SaveAs(outfilepath.rstrip('.png')+'.png') 

if __name__=='__main__':
    
    histfile = 'egammaEffi.txt_EGM2D.root'
    outfolder = 'test'

    if len(sys.argv)==3:
	histfile = sys.argv[1]
	outfolder = sys.argv[2]

    else:
	print('### WARNING ###: hist2plotter needs 2 command line arguments.')
	print('			Normal usage: python hist2dplotter.py <input file> <output dir>')
	print('                 Run with default args instead? (y/n)')
	go = raw_input()
	if not go == 'y': sys.exit()

    histfile = os.path.abspath(histfile)
    if not os.path.exists(outfolder):
	os.makedirs(outfolder)
    outfolder = os.path.abspath(outfolder)

    histlist = loadhistograms(histfile) 
    for hist in histlist:
	plot2dhistogram(hist,os.path.join(outfolder,hist.GetName()),'col z cjust')
