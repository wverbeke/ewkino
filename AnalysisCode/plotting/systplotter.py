##########################################################
# Process and plot histograms with systematic variations #
##########################################################
# to be used on output files of runsystematics.py

import ROOT
import sys
import numpy as np
import json
import os
import plottools as tools

def loadhistograms(histfile):
    # load histograms from a root file.
    # 'histfile' is a string containing the path to the input root file.
    # the output is a list of histograms
    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
	hist = f.Get(key.GetName())
	hist.SetDirectory(0)
	try:
	    nentries = hist.GetEntries() # maybe replace by more histogram-specific function
	except:
	    print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
	histlist.append(hist)
    return histlist

def findbytitle(histlist,title):
    # find a histogram by its title, return the index or -1 if not found
    index = -1
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: index = i
    return index

def sethiststyle(hist):
    # set color and line properties of a histogram
    systematic = hist.GetTitle()
    hist.SetLineWidth(1)
    hist.SetLineColor(getcolorsyst(systematic))
    if('Up' in systematic):
	hist.SetLineStyle(9)
    elif('Down' in systematic):
	hist.SetLineStyle(10)

def getcolorsyst(systematic):
    # return a color corresponding to a given systematic
    # for now return same color for up and down, maybe later asymmetrize
    if(systematic=='JECUp' or systematic=='JECDown'): return ROOT.kRed
    if(systematic=='JERUp' or systematic=='JERDown'): return ROOT.kRed+2
    if(systematic=='UnclUp' or systematic=='UnclDown'): return ROOT.kOrange-3
    if(systematic=='pileupUp' or systematic=='pileupDown'): return ROOT.kBlue
    print('### WARNING ###: tag not recognized (in setcolorTZQ), returning default color')
    return ROOT.kBlack

def clip(hist):
    # set minimum value for all bins to zero (no net negative weight)
    for i in range(hist.GetNbinsX()+1):
	if hist.GetBinContent(i)<0:
	    hist.SetBinContent(i,0.)
	    hist.SetBinError(i,0)

def getminmax(histlist):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 1.
    totmin = 1.
    for hist in histlist:
	histmax = (hist.GetBinContent(hist.GetMaximumBin())
		    +hist.GetBinErrorUp(hist.GetMaximumBin()))
	histmin = (hist.GetBinContent(hist.GetMinimumBin())
                    -hist.GetBinErrorLow(hist.GetMinimumBin()))
	if histmax > totmax: totmax = histmax
	if histmin < totmin: totmin = histmin
    topmargin = (totmax-1)
    bottommargin = (1-totmin)/2.
    return (totmin-bottommargin,totmax+topmargin)

#### adapted up to here ######

def plotsystematics(mchistlist,yaxtitle,xaxtitle,outfile,relative=True):
    
    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    rfrac = 0.25 # fraction of ratio plot in canvas
    # fonts and sizes:
    #titlefont = 6; titlesize = 60
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
    #infofont = 6; infosize = 40
    #legendfont = 4; legendsize = 40
    # title offset
    ytitleoffset = 1.9
    xtitleoffset = 4.5
    # margins:
    p1topmargin = 0.07
    p2bottommargin = 0.4
    leftmargin = 0.15
    rightmargin = 0.05
    # legend box
    p1legendbox = [leftmargin+0.03,1-p1topmargin-0.25,1-rightmargin-0.03,1-p1topmargin-0.03]
    p2legendbox = [leftmargin+0.03,0.84,1-rightmargin-0.03,0.97]
    # marker properties for data
    markerstyle = 20
    markercolor = 1
    markersize = 0.5  

    ### get nominal histogram and remove from the list
    nominalindex = findbytitle(mchistlist,"nominal")
    nominalhist = mchistlist[nominalindex]
    if(nominalindex>-1):
        indices = list(range(len(mchistlist)))
        indices.remove(nominalindex)
        mchistlist = [mchistlist[i] for i in indices]

    ### operations on mc histograms
    sethiststyle(nominalhist)
    clip(nominalhist)
    for hist in mchistlist:
        sethiststyle(hist)
	clip(hist)
    if relative: 
	nominalhist.Divide(nominalhist)
	newhistlist = []
	for hist in mchistlist:
	    newhist = hist.Clone()
	    for i in range(0,hist.GetNbinsX()+2):
		if nominalhist.GetBinContent(i)==0:
		    newhist.SetBinContent(i,1.)
		    newhist.SetBinError(i,0.)
		else:
		    newhist.SetBinContent(i,hist.GetBinContent(i)/nominalhist.GetBinContent(i))
		    newhist.SetBinError(i,hist.GetBinError(i)/nominalhist.GetBinContent(i))
	    newhistlist.append(newhist)
	mchistlist = newhistlist
    
    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(p1legendbox[0],p1legendbox[1],p1legendbox[2],p1legendbox[3])
    legend.SetNColumns(2)
    legend.SetFillStyle(0)
    legend.AddEntry(nominalhist,nominalhist.GetTitle(),"l")
    for hist in mchistlist:
        legend.AddEntry(hist,hist.GetTitle(),"l")

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,rfrac,1.,1.)
    pad1.SetTopMargin(p1topmargin)
    pad1.SetBottomMargin(0.03)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.Draw()
    pad2 = ROOT.TPad("pad2","",0.,0.,1.,rfrac)
    pad2.SetTopMargin(0.01)
    pad2.SetBottomMargin(p2bottommargin)
    pad2.SetLeftMargin(leftmargin)
    pad2.SetRightMargin(rightmargin)
    pad2.Draw()
    
    ### make upper part of the plot
    pad1.cd()
    (rangemin,rangemax) = getminmax(mchistlist)
    nominalhist.SetMinimum(rangemin)
    nominalhist.SetMaximum(rangemax)

    # X-axis layout
    xax = nominalhist.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(0)
    # Y-axis layout
    yax = nominalhist.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # histograms
    nominalhist.Draw("hist")
    for hist in mchistlist:
	hist.Draw("hist same")
    legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    ### save the plot
    c1.SaveAs(outfile+'.png')

if __name__=="__main__":
    
    ### Configure input parameters (hard-coded)
    # file to read the histograms from
    histfile = os.path.abspath('../systematics/test/tZq_ll_4f_13TeV-amcatnlo-pythia8_Summer16.root')

    ### Overwrite using cmd args
    if(len(sys.argv)==2):
	histfile = sys.argv[1]
    elif(not len(sys.argv)==1):
	print('### ERROR ###: wrong number of command line args')
	sys.exit()

    histlist = loadhistograms(histfile)
    histdir = os.path.abspath('')

    ### Set plot properties
    binwidth = histlist[0].GetBinWidth(1)
    if binwidth.is_integer():
        yaxtitle = 'events / '+str(int(binwidth)) # maybe find a way to get variable unit here
    else:
        yaxtitle = 'events / {0:.2f}'.format(binwidth) # maybe find a way to get variable unit here
    xaxtitle = histlist[0].GetXaxis().GetTitle()
    figname = os.path.join(histdir,'test')
    plotsystematics(histlist,yaxtitle,xaxtitle,figname+'_abs',relative=False)
    plotsystematics(histlist,yaxtitle,xaxtitle,figname+'_rel',relative=True)
