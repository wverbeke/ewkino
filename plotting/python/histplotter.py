############################################################
# A translation into python of ewkino/plotting/plotCode.cc #
############################################################

import ROOT
import sys
import os
import numpy as np
import json
import plottools as pt

def orderhistograms(histlist,ascending=True):
    # order a list of histograms according to sumOfWeights
    weightlist = []
    for hist in histlist:
        sumofweights = hist.GetSumOfWeights()
        weightlist.append(sumofweights)
    weightlist = np.array(weightlist)
    sorted_indices = np.argsort(weightlist)
    if ascending: return [histlist[i] for i in sorted_indices]
    else: return [histlist[i] for i in np.flip(sorted_indices)]

def findbytitle(histlist,title):
    # find a histogram by its title, return the index or -1 if not found
    index = -1
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: index = i
    return index

def stackcol(hist,color):
    # set color and line properties of histogram in stack
    hist.SetFillColor(color)
    hist.SetLineWidth(1)
    hist.SetLineColor(ROOT.kBlack)

def clip(hist):
    # set minimum value for all bins to zero (no net negative weight)
    for i in range(hist.GetNbinsX()+1):
	if hist.GetBinContent(i)<0:
	    hist.SetBinContent(i,0.)
	    hist.SetBinError(i,0)

def getminmax(datahist,mchist,yaxlog):
    # get suitable minimum and maximum values for plotting a given data hist and summed mc hist
    # find maximum:
    histmax = (mchist.GetBinContent(mchist.GetMaximumBin())
                +mchist.GetBinErrorUp(mchist.GetMaximumBin()))
    histmax = max(histmax,datahist.GetBinContent(datahist.GetMaximumBin())
                            +datahist.GetBinErrorUp(datahist.GetMaximumBin()))
    if not yaxlog: return (0,histmax*1.5)
    # find minimum (manually to avoid zero)
    histmin = histmax
    for i in range(1,mchist.GetNbinsX()+1):
        if( (not mchist.GetBinContent(i)==0) and mchist.GetBinContent(i)<histmin):
            histmin = mchist.GetBinContent(i)
    rangemin = histmin/5.
    rangemax = histmax*np.power(histmax/rangemin,0.4)
    return (rangemin,rangemax)

def drawbincontent(mchistlist,mchisterror,tag):
    text = ROOT.TLatex()
    text.SetTextAlign(21)
    text.SetTextFont(5);
    text.SetTextSize(22);
    taghist = mchistlist[findbytitle(mchistlist,tag)]
    for i in range(1, taghist.GetNbinsX()+1):
	xcoord  = taghist.GetXaxis().GetBinCenter(i)
	ycoord  = mchisterror.GetBinContent(i)+mchisterror.GetBinErrorUp(i)
	printvalue = taghist.GetBinContent(i)
	text.DrawLatex(xcoord, ycoord+0.3, '{:.2f}'.format(printvalue))
    return None

def plotdatavsmc(outfile,datahist,mchistlist,mcsysthist=None,mcstathist=None,
	signals=None,
	dostat=True,datalabel=None,labelmap=None,colormap=None,
	xaxtitle=None,xaxinteger=False,
	yaxtitle=None,yaxlog=False,yaxrange=None,
	p2yaxtitle=None,p2yaxrange=None,
	p1legendncols=None,p1legendbox=None,
	extracmstext='',lumi=None,
	extrainfos=[]):
    ### make a (stacked) simulation vs. data plot
    # arguments:
    # - outfile is the output file where the figure will be saved
    # - datahist is the data histogram
    # - mchistlist is a list of simulated histograms
    #   (note that the title of each histogram will be used to determine its label!)
    #   (note that the title of each histogram will be used to determine its color!)
    # - mcsyshist is a histogram containing the total systematic variation on the simulation
    #   (note: the bin content of mcsyshist must be the absolute difference wrt nominal!)
    #   (note: if dostat is False, mcsysthist must be the total uncertainty, not only systematic!
    #          if dostat is True, mcsysthist is syst only;
    #	       stat error will be taken from mcstathist if it is defined or from mchistlist otherwise!)
    # - mcstathist is a histogram containing the total statistical variation on the simulation
    #   (note: see above!)
    # - signals = list of signal processes (will be put at the top of the plot)
    # - datalabel is a string that will be used as legend entry for datahist (default: Data)
    # - labelmap is a dict mapping strings (histogram titles) to legend entries
    # - colormap is a dict mapping strings (histogram titles) to colors
    # - xaxtitle is the x-axis title (default: no title)
    # - xaxinteger is a boolean whether to use only integer axis labels and ticks for the x-axis
    # - yaxtitle: y-axis title (default: no title)
    # - yaxlog is whether to make the y-axis log scale
    # - yaxrange: tuple of (min,max) for plotting range (default: automatic range)
    # - p2yaxtitle:  y-axis title of bottom pad (default: datalabel/Pred.)
    # = p2yaxrange: tuple of (min,max) for plotting range of bottom pad (default: (0,2))
    # - lumi is the luminosity value (float, in pb-1) that will be displayed
    # - extrainfos is a list of strings with extra info to display
    
    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### parse arguments
    if datalabel is None: datalabel = "Data"
    # if no labelmap, use histogram titles
    if labelmap is None: 
	labelmap = {}
	for hist in mchistlist: labelmap[hist.GetTitle()] = hist.GetTitle()
    # if no colormap, use default colors
    if colormap is None:
	clist = ([ROOT.kRed-7,ROOT.kOrange,ROOT.kCyan-7,ROOT.kYellow+1,ROOT.kBlue-10,
		    ROOT.kBlue-6,ROOT.kTeal-5,ROOT.kMagenta-7])
	colormap = {}
	for i,hist in enumerate(mchistlist):
	    colormap[hist.GetTitle()] = clist[ i%len(clist) ]
    # y-axis title and range for bottom pad
    if p2yaxtitle is None: p2yaxtitle = datalabel+' / Pred.'
    if p2yaxrange is None: p2yaxrange = (0.,1.999)
    
    ### define global parameters for size and positioning
    cheight = 600 # height of canvas
    cwidth = 600 # width of canvas
    rfrac = 0.3 # fraction of ratio plot in canvas
    # fonts and sizes:
    labelfont = 4; labelsize = 22
    axtitlefont = 4; axtitlesize = 27
    infofont = 4; infosize = 20
    # title offset
    ytitleoffset = 1.5
    # temporary modification for an annoying comment on one figure:
    xtitleoffset = 3.5
    # margins:
    p1topmargin = 0.07
    p2bottommargin = 0.4
    leftmargin = 0.15
    rightmargin = 0.05
    # legend box
    if p1legendbox is None: 
	p1legendbox = [leftmargin+0.35,1-p1topmargin-0.3,1-rightmargin-0.03,1-p1topmargin-0.03]
    if p1legendncols is None:
	p1legendncols = 2
    p2legendbox = [leftmargin+0.03,0.84,1-rightmargin-0.03,0.97]
    # extra info box parameters
    infoleft = leftmargin+0.05
    infotop = 1-p1topmargin-0.1
    # marker properties for data
    markerstyle = 20
    markercolor = 1
    markersize = 0.75  

    ### order mc histograms
    # order by sumofweights
    mchistlist = orderhistograms(mchistlist)
    # get signals to the back (i.e. on top of the plot)
    if signals is not None:
	for signal in signals[::-1]:
	    sindex = findbytitle(mchistlist,signal)
	    if(sindex>-1):
		indices = list(range(len(mchistlist)))
		indices.remove(sindex)
		indices = indices+[sindex]
		mchistlist = [mchistlist[i] for i in indices]

    ### operations on mc histograms
    mchistsum = mchistlist[0].Clone()
    mchistsum.Reset()
    mchiststack = ROOT.THStack("mchiststack","")
    for hist in mchistlist:
        stackcol( hist, colormap.get(hist.GetTitle(), ROOT.kBlack) )
	clip(hist) # set negative bins to zero!
        mchistsum.Add(hist)
        mchiststack.Add(hist)

    ### calculate total mc error and set its histogram properties
    mcerror = mchistsum.Clone()
    mcstaterror = mchistsum.Clone()
    # in case of dostat=False: mcsysthist represents total variation to plot
    if not dostat:
	if mcsysthist is None:
	    raise Exception('ERROR in histplotter.py: option dostat = False'
		    +' is not compatible with mcsysthist = None')
	else:
	    for i in range(1,mchistsum.GetNbinsX()+1):
		mcerror.SetBinError(i,mcsysthist.GetBinContent(i))
    # in case of dostat=True: add stat errors and syst errors quadratically
    else:
	if mcstathist is not None:
	    # take statistical variations from externally provided histograms
	    for i in range(1,mchistsum.GetNbinsX()+1):
		mcstaterror.SetBinError(i,mcstathist.GetBinContent(i))
		mcerror.SetBinError(i,mcstathist.GetBinContent(i))
	if mcsysthist is not None:
	    for i in range(1,mchistsum.GetNbinsX()+1):
		staterror = mcstaterror.GetBinError(i)
		systerror = mcsysthist.GetBinContent(i)
		mcerror.SetBinError(i,np.sqrt(np.power(staterror,2)+np.power(systerror,2)))
    mcerror.SetLineWidth(0)
    mcerror.SetMarkerStyle(0)
    mcerror.SetFillStyle(3254)
    mcerror.SetFillColor(ROOT.kBlack)

    ### calculate total and statistical mc error (scaled)
    scstaterror = mcerror.Clone()
    scerror = mcerror.Clone()
    for i in range(1,mcerror.GetNbinsX()+1):
        scstaterror.SetBinContent(i,1.)
        scerror.SetBinContent(i,1.)
        if not mcerror.GetBinContent(i)==0:
            scstaterror.SetBinError(i,mcstaterror.GetBinError(i)/mcstaterror.GetBinContent(i))
            scerror.SetBinError(i,mcerror.GetBinError(i)/mcerror.GetBinContent(i))
        else:
            scstaterror.SetBinError(i,0.)
            scerror.SetBinError(i,0.)
    # in case of dostat=False: plot only total scaled variation, same style as unscaled
    if not dostat:
	scstaterror.Reset()
	scerror.SetLineWidth(0)
	scerror.SetMarkerStyle(0)
	scerror.SetFillStyle(3254)
	scerror.SetFillColor(ROOT.kBlack)
    # in case of dostat=True: plot total and stat only scaled variation
    else:
	scstaterror.SetFillStyle(1001)
	scstaterror.SetFillColor(ROOT.kGray+1)
	scstaterror.SetMarkerStyle(0)
	scerror.SetFillStyle(3254)
        scerror.SetFillColor(ROOT.kBlack)
	scerror.SetMarkerStyle(0)

    ### operations on data histogram
    datahist.SetMarkerStyle(markerstyle)
    datahist.SetMarkerColor(markercolor)
    datahist.SetMarkerSize(markersize)
    datahist.SetLineColor(markercolor)

    ### calculate data to mc ratio
    ratiograph = ROOT.TGraphAsymmErrors(datahist)
    for i in range(1,datahist.GetNbinsX()+1):
        if not mchistsum.GetBinContent(i)==0:
            ratiograph.GetY()[i-1] *= 1./mchistsum.GetBinContent(i)
            ratiograph.SetPointError(i-1,0,0,datahist.GetBinErrorLow(i)/mchistsum.GetBinContent(i),
                                        datahist.GetBinErrorUp(i)/mchistsum.GetBinContent(i))
        # avoid drawing empty mc or data bins
        else: ratiograph.GetY()[i-1] = 1e6
        if(datahist.GetBinContent(i)<=0): ratiograph.GetY()[i-1] += 1e6

    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(p1legendbox[0],p1legendbox[1],p1legendbox[2],p1legendbox[3])
    legend.SetNColumns(p1legendncols)
    legend.SetFillStyle(0)
    legend.AddEntry(datahist,datalabel,"pe1")
    for hist in mchistlist[::-1]:
        legend.AddEntry(hist,labelmap.get(hist.GetTitle(), '-'),"f")
    legend.AddEntry(mcerror,"Uncertainty","f")

    ### make legend for lower plot and add all histograms
    # (note: will not be drawn if dostat = False)
    legend2 = ROOT.TLegend(p2legendbox[0],p2legendbox[1],p2legendbox[2],p2legendbox[3])
    legend2.SetNColumns(3) 
    legend2.SetFillStyle(0)
    legend2.AddEntry(scstaterror, "Stat. uncertainty", "f")

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,rfrac,1.,1.)
    pad1.SetTopMargin(p1topmargin)
    pad1.SetBottomMargin(0.03)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetFrameLineWidth(2)
    pad1.Draw()
    pad2 = ROOT.TPad("pad2","",0.,0.,1.,rfrac)
    pad2.SetTopMargin(0.01)
    pad2.SetBottomMargin(p2bottommargin)
    pad2.SetLeftMargin(leftmargin)
    pad2.SetRightMargin(rightmargin)
    pad2.SetFrameLineWidth(2)
    pad2.Draw()

    ### make upper part of the plot
    pad1.cd()
    # determine range of pad
    if(yaxlog): pad1.SetLogy()
    (rangemin,rangemax) = getminmax(datahist,mcerror,yaxlog)
    if yaxrange is not None: (rangemin,rangemax) = yaxrange
    # temporary modification for an annoying comment on one figure:
    #rangemax = rangemax*1.2
    mcerror.SetMinimum(rangemin)
    mcerror.SetMaximum(rangemax)

    # X-axis layout
    xax = mcerror.GetXaxis()
    xax.SetNdivisions(10,5,0,ROOT.kTRUE)
    xax.SetLabelSize(0)
    # Y-axis layout
    yax = mcerror.GetYaxis()
    yax.SetMaxDigits(4)
    yax.SetNdivisions(10,5,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    if yaxtitle is not None: yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw mcerror first to get range correct
    mcerror.Draw("e2")
    # now draw in correct order
    mchiststack.Draw("hist same")
    mcerror.Draw("e2 same")
    datahist.Draw("pe e1 x0 same") 
    # (note: e1 draws error bars, x0 suppresses horizontal error bars)
    legend.Draw("same")
    # draw some extra info if needed
    ROOT.gPad.RedrawAxis()

    # draw header
    lumistr = ''
    if lumi is not None:
	lumistr = '{0:.3g}'.format(lumi/1000.)+' fb^{-1} (13 TeV)'
    pt.drawLumi(pad1,extratext=extracmstext,lumitext=lumistr)

    # draw extra info
    tinfo = ROOT.TLatex()
    tinfo.SetTextFont(10*infofont+3)
    tinfo.SetTextSize(infosize)
    for i,info in enumerate(extrainfos):
	tinfo.DrawLatexNDC(infoleft,infotop-(i+1)*0.07,info)

    ### make the lower part of the plot
    pad2.cd()
    # X-axis layout
    xax = scerror.GetXaxis()
    if xaxinteger: 
	# option 1:
	#xax.SetOption("I") 
	# does not work properly since it is not defined for TAxis, only TGAxis (?)
	# option 2:
	xax.SetNdivisions(xax.GetNbins()*2,0,0,ROOT.kFALSE)
	for i in range(0,xax.GetNbins()+1):
	    xax.ChangeLabel(2*i+1,-1,-1,-1,-1,-1," ")
	# works, but half-integer ticks are still there...
	# option 3: 
	#xax.SetNdivisions(xax.GetNbins(),0,0,ROOT.kFALSE)
	#xax.CenterLabels()
	# does not work properly since labels are still half-integer but simply shifted
    else: xax.SetNdivisions(10,5,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    if xaxtitle is not None: xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = scerror.GetYaxis()
    yax.SetRangeUser(p2yaxrange[0],p2yaxrange[1]);
    yax.SetTitle(p2yaxtitle)
    yax.SetMaxDigits(3)
    yax.SetNdivisions(4,5,0)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    scerror.Draw("e2")
    if dostat: scstaterror.Draw("e2 same")
    ratiograph.Draw("p e1 same")
    if dostat: legend2.Draw("same")
    ROOT.gPad.RedrawAxis()

    # make and draw unit ratio line
    xmax = datahist.GetXaxis().GetBinUpEdge(datahist.GetNbinsX())
    xmin = datahist.GetXaxis().GetBinLowEdge(1)
    line = ROOT.TLine(xmin,1,xmax,1)
    line.SetLineStyle(2)
    line.Draw("same")
    
    ### save the plot
    c1.SaveAs(outfile+'.png')
    c1.SaveAs(outfile+'.eps')
    c1.SaveAs(outfile+'.pdf')
