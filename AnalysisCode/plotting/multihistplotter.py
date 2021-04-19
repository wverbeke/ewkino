################################################################
# plotting code for plotting and comparing multiple histograms #
################################################################

import ROOT
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__),'../tools'))
import histtools as ht
import plottools as pt

def plothistograms(histlist,colorlist,xaxtitle,yaxtitle,outfile,
		    normalize=False,normalizefirst=False,dolegend=True,labellist=None,
		    logy=False,errorbars=False,ratiorange=None,
		    cliprange=False):
    ### plot multiple overlaying histograms (e.g. for shape comparison)
    ### note: the ratio plot will show ratios w.r.t. the first histogram in the list
    # arguments:
    # - histlist, colorlist, labellist: lists of TH1, ROOT colors and labels respectively
    # - xaxtitle, yaxtitle, outfile: self-explanatory
    # - normalize: boolean whether to put all histogram integrals to unit surface area
    # - normalizefirst: boolean whether to normalize first histogram in list and scale others 
    #                   by the same factor (do not use together with normalize)
    # - dolegend: boolean whether to make a legend (histogram title is used if no labellist)
    # - logy: boolean whether to make y-axis logarithmic
    # - errorbars: boolean whether to draw vertical error bars
    # - ratiorange is a tuple of (ylow,yhigh) for the ratio plot, default (0,2)
    # - cliprange is a boolean whether to clip the y-axis to minimum zero

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### parse arguments
    if(len(histlist)!=len(colorlist)):
        print('### ERROR ###: histogram list and color list must have the same length')
        sys.exit()
    if(labellist is not None and len(labellist)!=len(histlist)):
	print('### ERROR ###: label list must be None or equally long as histogram list')
	sys.exit()

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    rfrac = 0.33 # fraction of bottom plot showing the ratio
    # fonts and sizes:
    #titlefont = 6; titlesize = 60
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
    #infofont = 6; infosize = 40
    #legendfont = 4; legendsize = 30
    # title offset
    ytitleoffset = 2.5
    xtitleoffset = 2.5
    # margins:
    ptopmargin = 0.05/(1-rfrac)
    pbottommargin = 0.15/rfrac
    leftmargin = 0.2
    rightmargin = 0.05
    # legend box
    plegendbox = [leftmargin+0.03,1-ptopmargin-0.30,1-rightmargin-0.03,1-ptopmargin-0.07]

    ### operations on histograms
    scale = 1
    if( normalizefirst ): scale = histlist[0].Integral("width")
    for i,hist in enumerate(histlist):
        hist.SetLineWidth(2)
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
    legend.SetNColumns(2)
    legend.SetFillStyle(0)
    for i,hist in enumerate(histlist):
        label = hist.GetTitle()
        if labellist is not None: label = labellist[i]
        legend.AddEntry(hist,label,"l")

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,rfrac,1.,1.)
    pad1.SetTopMargin(ptopmargin)
    pad1.SetBottomMargin(0.03)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.Draw()
    pad2 = ROOT.TPad("pad2","",0.,0.,1.,rfrac)
    pad2.SetTopMargin(0.01)
    pad2.SetBottomMargin(pbottommargin)
    pad2.SetLeftMargin(leftmargin)
    pad2.SetRightMargin(rightmargin)
    pad2.Draw()

    ### make upper part of the plot
    pad1.cd()
    if logy: pad1.SetLogy()
    (rangemin,rangemax) = ht.getminmaxmargin(histlist,clip=cliprange)
    histlist[0].SetMinimum(rangemin)
    histlist[0].SetMaximum(rangemax)

    # X-axis layout
    xax = histlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(0)
    # Y-axis layout
    yax = histlist[0].GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # histograms
    extraoptions = ""
    if errorbars: extraoptions += " e"
    histlist[0].Draw("hist"+extraoptions)
    for hist in histlist[1:]:
        hist.Draw("hist same"+extraoptions)
    if dolegend:
	legend.SetFillColor(ROOT.kWhite)
	legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    pt.drawLumi(pad1,lumitext="simulation")

    ### make the lower part of the plot
    pad2.cd()
    xax = ratiohistlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetTitle(xaxtitle)
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
    yax.SetTitle('ratio')
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    extraoptions = ""
    if errorbars: extraoptions += " e"
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

