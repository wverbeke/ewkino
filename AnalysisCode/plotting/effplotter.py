#############################################################################
# Plot data and MC efficiencies (e.g. trigger efficiencies) and their ratio #
#############################################################################
# to be used after triggerefficiency.cc (in corresponding folder) which creates the histograms
# note: one mc histogram is assumed, while multiple data histograms can be overlayed (e.g. eras)!

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
        hist.SetDirectory(0)
	# check if histogram is readable
        try:
            nentries = hist.GetEntries() # maybe replace by more histogram-specific function
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
	# check if histogram needs to be included
	keep = True
	for el in mustcontain:
	    if el not in hist.GetTitle(): keep = False; break;
	for el in mustnotcontain: 
	    if el in hist.GetTitle(): keep = False; break;
	if not keep: continue;
	# add hist to list
        histlist.append(hist)
    return histlist

def findbytitle(histlist,title):
    # find a histogram by its title, return the index or -1 if not found
    index = -1
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: index = i
    return index

def getminmax(histlist):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 1.
    totmin = 1.
    for hist in histlist:
        for i in range(1,hist.GetNbinsX()+1):
            val = hist.GetBinContent(i)
            if val > totmax: totmax = val
            if val < totmin: totmin = val
    topmargin = (totmax-totmin)/2.
    bottommargin = (totmax-totmin)/5.
    return (totmin-bottommargin,totmax+topmargin)

def formathisttitle(title):
    # get suitable legend entry out of histogram title (depends on naming convention in source file!)
    if('mc_' in title): return 'simulation'
    if('201' in title): return 'data '+title[title.find('201'):title.find('201')+5]

def plotefficiencies(datahistlist,mchist,mcabs,mcsysthist,yaxtitle,xaxtitle,lumi,outfile):
    # note: mcabs is absolute histogram (while datahistlist and mchist contain efficiencies)
    #	    mcsyshist is systematic uncertainty to apply to mchist
    #	    both of the above can be None
    
    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    rfrac = 0.25 # fraction of ratio plot in canvas
    # fonts and sizes:
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
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
    markersize = 0.5
    # y axis range
    yaxmin = 0.
    yaxmax = 1.5

    ### set properties of MC histogram
    mchist.SetLineColor(ROOT.kBlack)
    mchist.SetLineWidth(2)

    ### set properties of absolute MC histogram
    plotspectrum = False
    if mcabs is not None:
	plotspectrum = True
	mcabs.SetFillColor(ROOT.kGray)
	mcabs.SetLineColor(ROOT.kBlack)
	mcabs.SetLineWidth(1)
	# normalization and offset
	mcabsmax = 0
	for i in range(1,mcabs.GetNbinsX()+1):
	    if mcabs.GetBinContent(i)>mcabsmax: mcabsmax=mcabs.GetBinContent(i)
	scale = 0.5*(yaxmax-yaxmin)/mcabsmax
	mcabs.Scale(scale)
	for i in range(1,mcabs.GetNbinsX()+1):
	    mcabs.SetBinContent(i,mcabs.GetBinContent(i)+yaxmin)
   
    ### calculate total mc error and set its histogram properties
    mcerror = mchist.Clone()
    if mcsysthist is not None:
        for i in range(1,mchist.GetNbinsX()+1):
            staterror = mchist.GetBinError(i)
            systerror = mcsyshist.GetBinContent(i)
            mcerror.SetBinError(i,np.sqrt(np.power(staterror,2)+np.power(systerror,2)))
    mcerror.SetFillStyle(3244)
    mcerror.SetLineWidth(0)
    mcerror.SetFillColor(ROOT.kGray+2)
    mcerror.SetMarkerStyle(0)

    ### calculate total and statistical mc error (scaled)
    scstaterror = mcerror.Clone()
    scerror = mcerror.Clone()
    for i in range(1,mcerror.GetNbinsX()+1):
        scstaterror.SetBinContent(i,1.)
        scerror.SetBinContent(i,1.)
        if not mcerror.GetBinContent(i)==0:
            scstaterror.SetBinError(i,mchist.GetBinError(i)/mchist.GetBinContent(i))
            scerror.SetBinError(i,mcerror.GetBinError(i)/mcerror.GetBinContent(i))
        else:
            scstaterror.SetBinError(i,0.)
            scerror.SetBinError(i,0.)
    scstaterror.SetFillStyle(1001)
    scerror.SetFillStyle(1001)
    scstaterror.SetFillColor(ROOT.kCyan-4)
    scerror.SetFillColor(ROOT.kOrange-4)
    scstaterror.SetMarkerStyle(1)
    scerror.SetMarkerStyle(1)

    ### operations on data histogram
    clist = ([	ROOT.kRed, ROOT.kOrange-3, ROOT.kOrange, ROOT.kGreen+1,
		ROOT.kCyan+1, ROOT.kAzure, ROOT.kViolet])
    for i,datahist in enumerate(datahistlist):
	datahist.SetMarkerStyle(markerstyle)
	datahist.SetMarkerColor(clist[i])
	datahist.SetMarkerSize(markersize)
	datahist.SetLineWidth(1)
	datahist.SetLineColor(clist[i])

    ### calculate data to mc ratio
    ratiographlist = []
    for datahist in datahistlist:
	ratiograph = ROOT.TGraphAsymmErrors(datahist)
	for i in range(1,mchist.GetNbinsX()+1):
	    if not mchist.GetBinContent(i)==0:
		ratiograph.GetY()[i-1] *= 1./mchist.GetBinContent(i)
		ratiograph.SetPointError(i-1,0,0,datahist.GetBinErrorLow(i)/mchist.GetBinContent(i),
                                        datahist.GetBinErrorUp(i)/mchist.GetBinContent(i))
	    # avoid drawing empty mc or data bins
	    else: ratiograph.GetY()[i-1] = 1e6
	    if(datahist.GetBinContent(i)<=0): ratiograph.GetY()[i-1] += 1e6
	ratiographlist.append(ratiograph)

    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(p1legendbox[0],p1legendbox[1],p1legendbox[2],p1legendbox[3])
    legend.SetNColumns(2)
    legend.SetFillStyle(0)
    legend.AddEntry(mchist,formathisttitle(mchist.GetTitle()),"l")
    legend.AddEntry(mcerror,"total sim. unc.","f")
    for datahist in datahistlist:
	legend.AddEntry(datahist,formathisttitle(datahist.GetTitle()),"lpe")
    if plotspectrum: legend.AddEntry(mcabs,"simulated spectrum")

    ### make legend for lower plot and add all histograms
    legend2 = ROOT.TLegend(p2legendbox[0],p2legendbox[1],p2legendbox[2],p2legendbox[3])
    legend2.SetNColumns(2); 
    legend2.SetFillStyle(0);
    legend2.AddEntry(scstaterror, "stat. pred. unc.", "f");
    legend2.AddEntry(scerror, "total pred. unc.", "f");
    #legend2.AddEntry(ratiograph, "obs./pred.", "pe12"); # do not add all ratio graphs, no space.

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
    #(rangemin,rangemax) = getminmax(datahistlist+[mchist])
    #mcerror.SetMinimum(rangemin)
    #mcerror.SetMaximum(rangemax)
    mcerror.SetMinimum(yaxmin)
    mcerror.SetMaximum(yaxmax)

    # X-axis layout
    xax = mcerror.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(0)
    # Y-axis layout
    yax = mcerror.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw mcerror first to get range correct
    mcerror.Draw("e2")
    if plotspectrum: mcabs.Draw("hist same")
    for datahist in datahistlist: datahist.Draw("pe same")
    mchist.Draw("hist same")
    legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    lumistr = '{0:.1f}'.format(lumi/1000.)
    tools.drawLumi(pad1,lumitext=lumistr+" fb^{-1} (13 TeV)")

    ### make the lower part of the plot
    pad2.cd()
    # X-axis layout
    xax = scerror.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = scerror.GetYaxis()
    yax.SetRangeUser(0.,1.999);
    yax.SetTitle("obs./pred.");
    yax.SetMaxDigits(3)
    yax.SetNdivisions(4,5,0)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    scerror.Draw("e2")
    scstaterror.Draw("e2 same")
    for ratiograph in ratiographlist: ratiograph.Draw("pe01 same")
    legend2.Draw("same")
    ROOT.gPad.RedrawAxis()

    # make and draw unit ratio line
    xmax = datahist.GetXaxis().GetBinUpEdge(datahist.GetNbinsX())
    xmin = datahist.GetXaxis().GetBinLowEdge(1)
    line = ROOT.TLine(xmin,1,xmax,1)
    line.SetLineStyle(2)
    line.Draw("same")
    
    ### save the plot
    c1.SaveAs(outfile.rstrip('.png')+'.png')

if __name__=="__main__":
    
    # load histograms
    histfile = os.path.abspath('../triggerefficiency/test_dilepton/combined.root')
    lumi = 35900.
    histdir = histfile[:histfile.rfind('/')]
    mchistlist = loadhistograms(histfile,mustcontain=['mc','leadleppt_eff'])
    datahistlist = loadhistograms(histfile,mustcontain=['data','leadleppt_eff'])
    if not len(mchistlist)==1:
        print('### ERROR ###: list of MC histograms has unexpected length: '+str(len(mchistlist)))
        sys.exit()
    mchist = mchistlist[0]
    mcabs = loadhistograms(histfile,mustcontain=['mc','leadleppt_tot'])[0]
    # set plot properties
    binwidth = mchist.GetBinWidth(1)
    yaxtitle = 'trigger efficiency'
    xaxtitle = r'lepton p_T (GeV)'
    figname = os.path.join('test.png')
    plotefficiencies(datahistlist,mchist,mcabs,None,yaxtitle,xaxtitle,lumi,figname)
