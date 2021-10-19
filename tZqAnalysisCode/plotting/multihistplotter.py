################################################################
# plotting code for plotting and comparing multiple histograms #
################################################################

import ROOT
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__),'../tools'))
import histtools as ht
import plottools as pt

def plothistograms(histlist,outfile,xaxtitle=None,yaxtitle=None,
		    normalize=False,normalizefirst=False,dolegend=True,
		    labellist=None,colorlist=None,
		    logy=False,errorbars=False,ratiorange=None,yrange=None,yminzero=False,
		    lumistr='',extracmstext='',
		    doratio=True):
    ### plot multiple overlaying histograms (e.g. for shape comparison)
    ### note: the ratio plot will show ratios w.r.t. the first histogram in the list
    ### note: try to add argument to disable ratio plotting
    # arguments:
    # - histlist, colorlist, labellist: lists of TH1, ROOT colors and labels respectively
    # - xaxtitle, yaxtitle, outfile: self-explanatory
    # - normalize: boolean whether to put all histogram integrals to unit surface area
    # - normalizefirst: boolean whether to normalize first histogram in list and scale others 
    #                   by the same factor (do not use together with normalize)
    # - dolegend: boolean whether to make a legend (histogram title is used if no labellist)
    # - logy: boolean whether to make y-axis logarithmic
    # - errorbars: boolean whether to draw vertical error bars
    # - ratiorange: a tuple of (ylow,yhigh) for the ratio pad, default (0,2)
    # - yrange: a tuple of (ylow,yhigh) for the upper pad
    # - yminzero: whether to clip minimum y to zero.

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### parse arguments
    if( colorlist is not None and len(histlist)>len(colorlist) ):
        print('### ERROR ###: histogram list is longer than color list')
        sys.exit()
    if(labellist is not None and len(labellist)!=len(histlist)):
	print('### ERROR ###: label list must be None or equally long as histogram list')
	sys.exit()

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 600 # width of canvas
    rfrac = 0.33 # fraction of bottom plot showing the ratio
    if not doratio: rfrac = 0
    # fonts and sizes:
    labelfont = 4; labelsize = 22
    axtitlefont = 4; axtitlesize = 26
    legendfont = 4; legendsize = 15
    # margins and title offsets
    ytitleoffset = 1.5
    p1topmargin = 0.07
    if doratio: 
	p1bottommargin = 0.03
	xtitleoffset = 3.5
    else: 
	p1bottommargin = 0.15
	xtitleoffset = 1.
    p2topmargin = 0.01
    p2bottommargin = 0.4
    leftmargin = 0.15
    rightmargin = 0.05
    # legend box
    plegendbox = [leftmargin+0.3,1-p1topmargin-0.25,1-rightmargin-0.03,1-p1topmargin-0.03]

    ### normalization and style operations on histograms
    scale = 1
    if colorlist is None: 
	    colorlist = ([ROOT.kAzure-4,ROOT.kAzure+6,ROOT.kViolet,ROOT.kMagenta-9,
			    ROOT.kRed,ROOT.kPink-9,ROOT.kBlue+1])
    if( normalizefirst ): scale = histlist[0].Integral("width")
    for i,hist in enumerate(histlist):
        hist.SetLineWidth(3)
        hist.SetLineColor(colorlist[i])
        if normalize: scale = hist.Integral("width")
        for j in range(0,hist.GetNbinsX()+2):
            hist.SetBinContent(j,hist.GetBinContent(j)/scale)
            hist.SetBinError(j,hist.GetBinError(j)/scale)

    ### make ratio histograms
    ratiohistlist = []
    for hist in histlist:
	rhist = hist.Clone()
	for j in range(0,rhist.GetNbinsX()+2):
	    scale = histlist[0].GetBinContent(j)
	    if scale<1e-12:
		rhist.SetBinContent(j,0)
		rhist.SetBinError(j,10)
	    else:
		rhist.SetBinContent(j,rhist.GetBinContent(j)/scale)
		rhist.SetBinError(j,rhist.GetBinError(j)/scale)
	ratiohistlist.append(rhist)
 
    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
    legend.SetNColumns(1)
    #legend.SetFillStyle(3003)
    legend.SetFillColor(ROOT.kWhite)
    legend.SetTextFont(10*legendfont+3)
    legend.SetTextSize(legendsize)
    legend.SetBorderSize(1)
    for i,hist in enumerate(histlist):
        label = hist.GetTitle()
        if labellist is not None: label = labellist[i]
        legend.AddEntry(hist,label,"l")

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,rfrac,1.,1.)
    pad1.SetTopMargin(p1topmargin)
    pad1.SetBottomMargin(p1bottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetTicks(1,1)
    pad1.SetFrameLineWidth(2)
    pad1.SetGrid()
    pad1.Draw()
    if doratio:
	pad2 = ROOT.TPad("pad2","",0.,0.,1.,rfrac)
	pad2.SetTopMargin(p2topmargin)
	pad2.SetBottomMargin(p2bottommargin)
	pad2.SetLeftMargin(leftmargin)
	pad2.SetRightMargin(rightmargin)
	pad2.SetTicks(1,1)
	pad2.SetFrameLineWidth(2)
	pad2.SetGrid()
	pad2.Draw()

    ### make upper part of the plot
    pad1.cd()
    (totmin,totmax) = ht.getminmax(histlist)
    # log scale
    if logy:
        pad1.SetLogy()
	if yrange is None: yrange = (totmin/5,totmax*1e2)
    # lin scale
    else:
	if yrange is None: yrange = (0.,totmax*1.8)
    if yminzero and yrange[0]<0: yrange = (0.,yrange[1])
    histlist[0].SetMaximum(yrange[1])
    histlist[0].SetMinimum(yrange[0])

    # X-axis layout
    xax = histlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    if doratio:
	xax.SetLabelSize(0)
    else:
	xax.SetLabelSize(labelsize)
	xax.SetLabelFont(10*labelfont+3)
	if xaxtitle is not None: xax.SetTitle(xaxtitle)
	xax.SetTitleFont(10*axtitlefont+3)
	xax.SetTitleSize(axtitlesize)
	xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = histlist[0].GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    if yaxtitle is not None: yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # histograms
    extraoptions = ""
    if errorbars: extraoptions += " e1"
    histlist[0].Draw("hist"+extraoptions)
    for hist in histlist[1:]:
        hist.Draw("hist same"+extraoptions)
    if dolegend:
	legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    pt.drawLumi(pad1,extratext=extracmstext,lumitext=lumistr)

    if not doratio: 
	### save the plot
	c1.SaveAs(outfile.replace('.png','')+'.png')
	return

    ### make the lower part of the plot
    pad2.cd()
    xax = ratiohistlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    if xaxtitle is not None: xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = ratiohistlist[0].GetYaxis()
    if ratiorange==None: ratiorange = (0,1.999)
    yax.SetRangeUser(ratiorange[0],ratiorange[1]);
    yax.SetMaxDigits(3)
    yax.SetNdivisions(4,5,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle('Ratio')
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    extraoptions = ""
    if errorbars: extraoptions += " e1"
    ratiohistlist[0].Draw("hist"+extraoptions)
    for hist in ratiohistlist[1:]:
        hist.Draw("hist same"+extraoptions)
    ROOT.gPad.RedrawAxis()

    # make and draw unit ratio line
    xmax = histlist[0].GetXaxis().GetBinUpEdge(histlist[0].GetNbinsX())
    xmin = histlist[0].GetXaxis().GetBinLowEdge(1)
    line = ROOT.TLine(xmin,1,xmax,1)
    line.SetLineStyle(2)
    line.Draw("same")

    ### save the plot
    c1.SaveAs(outfile.replace('.png','')+'.png')

