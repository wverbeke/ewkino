######################################################################
# a Python translation of ewkino/plotting/plotCode.cc/plot2DHistogam #
######################################################################

import ROOT
import sys
import numpy as np
import json
import os
import plottools as tools
from array import array

def swapaxes( hist ):
    # switch x and y axes
    nbinsx = hist.GetNbinsX()
    nbinsy = hist.GetNbinsY()
    xbins = list(hist.GetXaxis().GetXbins())
    ybins = list(hist.GetYaxis().GetXbins())

    swappedhist = ROOT.TH2F("","",nbinsy,array('f',ybins),nbinsx,array('f',xbins))
    swappedhist.SetName(hist.GetName())
    swappedhist.SetTitle(hist.GetTitle())
    swappedhist.GetXaxis().SetTitle(hist.GetYaxis().GetTitle())
    swappedhist.GetYaxis().SetTitle(hist.GetXaxis().GetTitle())

    for i in range(1,nbinsx+1):
	for j in range(1,nbinsy+1):
	    c = hist.GetBinContent(i,j)
	    e = hist.GetBinError(i,j)
	    swappedhist.SetBinContent(j,i,c)
	    swappedhist.SetBinError(j,i,e)
    return swappedhist

def geterrorhist( hist ):
    # get the errors on a given histogram as bin contents in a separate histogram
    errorhist = hist.Clone()
    errorhist.Reset()
    for i in range(0,hist.GetNbinsX()+2):
	for j in range(0,hist.GetNbinsY()+2):
	    error = hist.GetBinError(i,j)
	    errorhist.SetBinContent(i,j,error)
    return errorhist

def getsmallouterflowhist( hist ):
    # return a copy of the histogram with small (empty) under- and overflow bins
    # seems to be necessary to get the axis labels right in log scale

    # do initializations from original histogram
    xbins = list(hist.GetXaxis().GetXbins())
    ybins = list(hist.GetYaxis().GetXbins())
    nxbins = len(xbins)-1
    xmin = xbins[0]
    xmax = xbins[-1]
    nybins = len(ybins)-1
    ymin = ybins[0]
    ymax = ybins[-1]

    # make extended x-and y-axes
    xexwidth = min(xbins[1]-xbins[0],xbins[-1]-xbins[-2])/1000.
    yexwidth = min(ybins[1]-ybins[0],ybins[-1]-ybins[-2])/1000.
    exxbins = [xmin-xexwidth]+xbins+[xmax+xexwidth]
    exybins = [ymin-yexwidth]+ybins+[ymax+yexwidth]

    # make extended histogram
    exthist = ROOT.TH2F("","",nxbins+2,array('f',exxbins),nybins+2,array('f',exybins))
    exthist.SetTitle(hist.GetTitle())
    exthist.GetXaxis().SetTitle(hist.GetXaxis().GetTitle())
    exthist.GetYaxis().SetTitle(hist.GetYaxis().GetTitle())
    exthist.GetXaxis().SetLabelFont(hist.GetXaxis().GetLabelFont())
    exthist.GetXaxis().SetLabelSize(hist.GetXaxis().GetLabelSize())
    exthist.GetXaxis().SetNdivisions(hist.GetXaxis().GetNdivisions())
    exthist.GetYaxis().SetLabelFont(hist.GetYaxis().GetLabelFont())
    exthist.GetYaxis().SetLabelSize(hist.GetYaxis().GetLabelSize())
    exthist.GetYaxis().SetNdivisions(hist.GetYaxis().GetNdivisions())
    exthist.GetZaxis().SetLabelFont(hist.GetZaxis().GetLabelFont())
    exthist.GetZaxis().SetLabelSize(hist.GetZaxis().GetLabelSize())
    exthist.GetZaxis().SetNdivisions(hist.GetZaxis().GetNdivisions())

    # fill the bins
    for i in range(2,nxbins+2):
	for j in range(2,nybins+2):
	    exthist.SetBinContent(i,j,hist.GetBinContent(i-1,j-1))
	    exthist.SetBinError(i,j,hist.GetBinError(i-1,j-1))
    return exthist


def print2dhist( hist ):
    for i in range(0,hist.GetNbinsX()+2):
        for j in range(0,hist.GetNbinsY()+2):
	    print('--- bin {} {} ---'.format(i,j))
            content = hist.GetBinContent(i,j)
            print('content: '+str(content))
            error = hist.GetBinError(i,j)
            print('error: '+str(error))


def plot2dhistogram(hist, outfilepath, histtitle='', logx=False, logy=False, 
		    drawoptions='colztexte'):

    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    
    # set global properties
    cheight = 500 # height of canvas
    cwidth = 700 # width of canvas
    titlefont = 4; titlesize = 22
    axtitlefont = 4; axtitlesize = 22
    # title offset
    ytitleoffset = 1.5
    xtitleoffset = 1.5
    # margins:
    topmargin = 0.15
    bottommargin = 0.15
    leftmargin = 0.15
    rightmargin = 0.15
    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()
    zmin = hist.GetMinimum()
    zmax = hist.GetMaximum()

    # create canvas
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    c1.SetTopMargin(topmargin)
    c1.SetBottomMargin(bottommargin)
    c1.SetLeftMargin(leftmargin)
    c1.SetRightMargin(rightmargin)
    
    # create copy of histogram with adapted outerflow bins if needed
    if( logx or logy ):
        hist = getsmallouterflowhist( hist )
        hist.SetMinimum(zmin)
        hist.SetMaximum(zmax)

    # set title and label properties
    hist.GetXaxis().SetTitleOffset(xtitleoffset)
    hist.GetXaxis().SetTitleFont(axtitlefont*10+3)
    hist.GetXaxis().SetTitleSize(axtitlesize)
    hist.GetYaxis().SetTitleOffset(ytitleoffset)
    hist.GetYaxis().SetTitleFont(axtitlefont*10+3)
    hist.GetYaxis().SetTitleSize(axtitlesize)

    if logx:
	c1.SetLogx()
	hist.GetXaxis().SetMoreLogLabels()
	
    if logy:
        c1.SetLogy()
        hist.GetYaxis().SetMoreLogLabels()

    # set title
    ttitle = ROOT.TLatex()
    ttitle.SetTextFont(titlefont*10+3)
    ttitle.SetTextSize(titlesize)

    # draw
    hist.Draw( drawoptions )
    ttitle.DrawLatexNDC(leftmargin,0.9,histtitle)

    # save the plot
    c1.Update()
    c1.SaveAs(outfilepath)
