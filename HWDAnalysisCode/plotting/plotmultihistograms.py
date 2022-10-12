########################################
# plot and compare multiple histograms #
########################################

import ROOT
import sys
import os
sys.path.append('../tools/python')
import histtools as ht
import plottools as pt
import optiontools as opt

def plotmultihistograms(histlist, figname=None, title=None, xaxtitle=None, yaxtitle=None,
			normalize=False, normalizefirst=False, 
			dolegend=True, labellist=None, 
			colorlist=None,
			logy=False, ymaxlinfactor=1.8, yminlogfactor=0.2, ymaxlogfactor=100,
			drawoptions='', 
			lumitext='', extralumitext = '',
			doratio=False, ratiorange=None, ylims=None, yminzero=False):
    ### plot multiple overlaying histograms (e.g. for shape comparison)
    # note: the ratio plot will show ratios w.r.t. the first histogram in the list!
    # arguments:
    # - histlist, colorlist, labellist: lists of TH1, ROOT colors and labels respectively
    # - figname: name of the figure to save (if None, do not save but return plot dictionary)
    # - title, xaxtitle, yaxtitle, figname: self-explanatory
    # - normalize: boolean whether to put all histogram integrals to unit surface area
    # - normalizefirst: boolean whether to normalize first histogram in list and scale others 
    #                   by the same factor (do not use together with normalize)
    # - dolegend: boolean whether to make a legend (histogram title is used if no labellist)
    # - logy: boolean whether to make y-axis logarithmic
    # - ymaxlinfactor: factor by which to multiply maximum y value (for linear y-axis)
    # - yminlogfactor and ymaxlogfactor: same as above but for log scale
    # - drawoptions: string passed to TH1.Draw
    #   see https://root.cern/doc/master/classTHistPainter.html for a full list of options
    # - lumitext and extralumitext: luminosity value and extra text
    # - ratiorange: a tuple of (ylow,yhigh) for the ratio pad, default (0,2)
    # - ylims: a tuple of (ylow,yhigh) for the upper pad
    # - yminzero: whether to clip minimum y to zero.

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### parse arguments
    if colorlist is None:
        colorlist = ([ROOT.kAzure-4, ROOT.kAzure+6, ROOT.kViolet, ROOT.kMagenta-9,
                      ROOT.kRed, ROOT.kGreen+1, ROOT.kGreen-1])
    if( len(histlist)>len(colorlist) ):
        raise Exception('ERROR in plotmultihistograms:'
			+' histogram list is longer than color list')
    if(labellist is not None and len(labellist)!=len(histlist)):
	raise Exception('ERROR in plotmultihistograms:'
			+' length of label list does not agree with histogram list')

    ### define global parameters for size and positioning
    cheight = 600 # height of canvas
    cwidth = 600 # width of canvas
    rfrac = 0.33 # fraction of bottom plot showing the ratio
    if not doratio: rfrac = 0
    # fonts and sizes:
    labelfont = 4; labelsize = 22
    axtitlefont = 4; axtitlesize = 26
    infofont = 4; infosize = 26
    legendfont = 4; 
    # margins and title offsets
    ytitleoffset = 1.5
    p1topmargin = 0.05
    p1bottommargin = 0.15
    xtitleoffset = 1.
    if doratio:
	p1topmargin = 0.07 
	p1bottommargin = 0.03
	xtitleoffset = 3.5
    p2topmargin = 0.01
    p2bottommargin = 0.4
    leftmargin = 0.15
    rightmargin = 0.05
    # legend box
    pentryheight = 0.05
    if doratio: pentryheight = 0.07
    nentries = 1 + len(histlist)
    if nentries>3: pentryheight = pentryheight*0.8
    plegendbox = ([leftmargin+0.3,1-p1topmargin-0.03-pentryheight*nentries,
                    1-rightmargin-0.03,1-p1topmargin-0.03])

    ### normalization and style operations on histograms
    scale = 1
    if( normalizefirst ): scale = histlist[0].Integral("width")
    for i,hist in enumerate(histlist):
        hist.SetLineWidth(3)
        hist.SetLineColor(colorlist[i])
        if normalize: 
	    scale = hist.Integral("width")
	    if scale==0.: scale = 1
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
    legend.SetFillColor(ROOT.kWhite)
    legend.SetTextFont(10*legendfont+3)
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

    # get x-limits (for later use)
    nbins = histlist[0].GetNbinsX()
    xlims = (histlist[0].GetBinLowEdge(1),
	     histlist[0].GetBinLowEdge(nbins)+histlist[0].GetBinWidth(nbins))
    # get and set y-limits
    (totmin,totmax) = ht.getminmax(histlist)
    # in case of log scale
    if logy:
        pad1.SetLogy()
	if ylims is None: ylims = (totmin*yminlogfactor, totmax*ymaxlogfactor)
    # in case of lin scale
    else:
	if ylims is None: ylims = (0.,totmax*ymaxlinfactor)
    if yminzero and ylims[0]<0: ylims = (0.,ylims[1])
    histlist[0].SetMaximum(ylims[1])
    histlist[0].SetMinimum(ylims[0])

    # X-axis layout
    xax = histlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    if doratio:
	xax.SetLabelSize(0)
    else:
	xax.SetLabelSize(labelsize)
	xax.SetLabelFont(10*labelfont+3)
	if xaxtitle is not None:
	    xax.SetTitle(xaxtitle)
	    xax.SetTitleFont(10*axtitlefont+3)
	    xax.SetTitleSize(axtitlesize)
	    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = histlist[0].GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    if yaxtitle is not None:
	yax.SetTitle(yaxtitle)
	yax.SetTitleFont(10*axtitlefont+3)
	yax.SetTitleSize(axtitlesize)
	yax.SetTitleOffset(ytitleoffset)

    # histograms
    histlist[0].Draw(drawoptions)
    for hist in histlist[1:]:
        hist.Draw("same "+drawoptions)
    if dolegend:
	legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    pt.drawLumi(pad1, extratext=extralumitext, lumitext=lumitext, rfrac=rfrac)

    if not doratio: 
	# return or save the plot
	if figname is None:
	    plotobject = {'canvas':c1, 'pads':(pad1), 'xlims':xlims, 'ylims':ylims,
			    'legend':legend}
	    return plotobject
	else:
	    c1.SaveAs(figname.replace('.png','')+'.png')
	    return None

    ### make the lower part of the plot
    pad2.cd()
    xax = ratiohistlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    if xaxtitle is not None:
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
    yax.SetTitle('Ratio')
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    ratiohistlist[0].Draw(drawoptions)
    for hist in ratiohistlist[1:]:
        hist.Draw("same "+drawoptions)
    ROOT.gPad.RedrawAxis()

    # make and draw unit ratio line
    xmax = histlist[0].GetXaxis().GetBinUpEdge(histlist[0].GetNbinsX())
    xmin = histlist[0].GetXaxis().GetBinLowEdge(1)
    line = ROOT.TLine(xmin,1,xmax,1)
    line.SetLineStyle(2)
    line.Draw("same")

    # return or save the plot
    if figname is None:
        plotobject = {'canvas':c1, 'pads':(pad1,pad2), 'xlims':xlims, 'ylims':ylims,
			'legend':legend}
        return plotobject
    else:
        c1.SaveAs(figname.replace('.png','')+'.png')
        return None


if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('histnames', vtype='list', default=[]) )
    options.append( opt.Option('xaxtitle', default='') )
    options.append( opt.Option('yaxtitle', default='') )
    options.append( opt.Option('title', default='') )
    options.append( opt.Option('logy', vtype='bool', default=False) )
    options.append( opt.Option('drawoptions', default='') )
    options.append( opt.Option('lumitext', default='') )
    options.append( opt.Option('extralumitext', default='') )
    options.append( opt.Option('doratio', vtype='bool', default=False) )
    options.append( opt.Option('outputfile', default='figure.png') )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # option 1: use histtools (but only works if histograms are in top-level directory)
    #histlist = ht.loadhistograms(options.inputfile,mustcontainone=options.histnames)
    # option 2: manually load histograms
    f = ROOT.TFile.Open(options.inputfile)
    histlist = []
    for histname in options.histnames:
	histname = str(histname)
	print(histname)
        hist = f.Get(histname)
        # check if histogram is readable
        try:
            nentries = hist.GetEntries()
            nbins = hist.GetNbinsX()
            hist.SetDirectory(ROOT.gROOT)
        except:
            print('WARNING: key "'+str(histname)+'" does not correspond to valid hist.')
            continue
        histlist.append(hist)
    f.Close()
	
    print('found {} histograms to plot'.format(len(histlist)))
    if( len(histlist)==0 ): raise Exception('ERROR: found no histograms.')

    plotmultihistograms(histlist, options.outputfile,
                        xaxtitle=options.xaxtitle, yaxtitle=options.yaxtitle,
                        title=options.title,
                        logy=options.logy, drawoptions=options.drawoptions,
			lumitext=options.lumitext, extralumitext=options.extralumitext,
			doratio=options.doratio)

