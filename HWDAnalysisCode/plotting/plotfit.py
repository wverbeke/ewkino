#######################################################
# plot and compare a histogram with a fitted function #
#######################################################
# note: originally copied from plotmultihistograms.py (on 23/11/2021)

import ROOT
import sys
import os
sys.path.append('../tools/python')
import histtools as ht
import plottools as pt
import optiontools as opt

def plotfit(datahist, funclist, histlist=None, figname=None, 
			title=None, xaxtitle=None, yaxtitle=None,
			dolegend=True, 
			datalabel=None,
			histlabellist=None, histcolorlist=None,
			funclabellist=None, funccolorlist=None,
			logy=False, ymaxlinfactor=1.8, yminlogfactor=0.2, ymaxlogfactor=100,
			ylims=None, yminzero=False,
			histdrawoptions='', 
			lumitext='', extralumitext = '',
			doratio=False, ratiorange=None ):
    ### plot multiple overlaying histograms with fitted functions overlaid
    # note: the ratio plot will show ratio between the datahist and 
    #       the first function in funclist;
    #       other objects are not shown in the ratio pad (for now)
    # arguments:
    # - datahist: TH1 object representing data to fit to
    # - funclist: list of TF1 objects
    # - histlist: list of TH1 objects
    # - figname: name of the figure to save (if None, do not save but return plot dictionary)
    # - title, xaxtitle, yaxtitle: self-explanatory
    # - dolegend: boolean whether to make a legend (histogram title is used if no labellist)
    # - datalabel: label for datahist
    # - histlabellist and histcolorlist: lists of labels and ROOT colors for histlist
    # - funclabellist and funccolorlist: lists of labels and ROOT colors for funclist
    # - logy: boolean whether to make y-axis logarithmic
    # - ymaxlinfactor: factor by which to multiply maximum y value (for linear y-axis)
    # - yminlogfactor and ymaxlogfactor: same as above but for log scale
    # - ylims: a tuple of (ylow,yhigh) for the upper pad (overwrites previous args)
    # - yminzero: whether to clip minimum y to zero (overwrites previous args)
    # - histdrawoptions: string passed to TH1.Draw (not applied to datahist, only to histlist!)
    #   see https://root.cern/doc/master/classTHistPainter.html for a full list of options
    # - lumitext and extralumitext: luminosity value and extra text
    # - doratio: boolean whether to make a lower pad with ratio
    # - ratiorange: a tuple of (ylow,yhigh) for the ratio pad, default (0,2)

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### parse arguments
    if funccolorlist is None:
	funccolorlist = ([ROOT.kRed, ROOT.kGreen+1, ROOT.kGreen-1])
    if( len(funclist)>len(funccolorlist) ):
        raise Exception('ERROR in plotfit:'
                        +' function list is longer than color list')
    if(funclabellist is not None and len(funclabellist)!=len(funclist)):
        raise Exception('ERROR in plotfit:'
                        +' length of label list does not agree with function list')
    if histlist is None: histlist=[]
    if histcolorlist is None:
	histcolorlist = ([ROOT.kAzure-4, ROOT.kAzure+6, ROOT.kViolet, ROOT.kMagenta-9])
    if( len(histlist)>len(histcolorlist) ):
	raise Exception('ERROR in plotfit:'
		        +' histogram list is longer than color list')
    if(histlabellist is not None and len(histlabellist)!=len(histlist)):
	raise Exception('ERROR in plotfit:'
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
    # marker properties for data
    markerstyle = 20
    markercolor = 1
    markersize = 0.75  
    # legend box
    pentryheight = 0.05
    if doratio: pentryheight = 0.07
    nentries = 1 + len(funclist) + len(histlist)
    if nentries>3: pentryheight = pentryheight*0.8
    plegendbox = ([leftmargin+0.3,1-p1topmargin-0.03-pentryheight*nentries,
		    1-rightmargin-0.03,1-p1topmargin-0.03])

    ### style operations on data histogram
    datahist.SetMarkerStyle(markerstyle)
    datahist.SetMarkerColor(markercolor)
    datahist.SetMarkerSize(markersize)
    datahist.SetLineColor(markercolor)
    datahist.SetStats(ROOT.kFALSE)

    ### style operations on functions
    for i,func in enumerate(funclist):
	func.SetLineColor(funccolorlist[i])
        func.SetLineWidth(3)
        #func.SetLineStyle(?)

    ### style operations on other histograms
    for i,hist in enumerate(histlist):
        hist.SetLineWidth(3)
        hist.SetLineColor(histcolorlist[i])

    ### make ratio histogram
    ratiohist = datahist.Clone()
    for j in range(0,ratiohist.GetNbinsX()+2):
	scale = funclist[0].Eval(ratiohist.GetBinCenter(j))
	if scale<1e-12:
	    ratiohist.SetBinContent(j,0)
	    ratiohist.SetBinError(j,10)
	else:
	    ratiohist.SetBinContent(j,ratiohist.GetBinContent(j)/scale)
	    ratiohist.SetBinError(j,ratiohist.GetBinError(j)/scale)

    ### make the raio function (i.e. constant at 1)
    ratiofunc = ROOT.TF1("ratiofunc", "1.0", funclist[0].GetXmin(), funclist[0].GetXmax())
    ratiofunc.SetLineColor(funccolorlist[0])
    ratiofunc.SetLineWidth(3)
 
    ### make legend for upper plot and add all objects
    legend = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
    legend.SetNColumns(1)
    legend.SetFillColor(ROOT.kWhite)
    legend.SetTextFont(10*legendfont+3)
    legend.SetBorderSize(1)
    # add other histograms
    for i,hist in enumerate(histlist):
        label = hist.GetTitle()
        if histlabellist is not None: label = histlabellist[i]
        legend.AddEntry(hist,label,"l")
    # add functions
    for i,func in enumerate(funclist):
        if funclabellist is not None:
            legend.AddEntry(func,funclabellist[i],"l")
    # add data histogram
    if datalabel is None: datalabel=datahist.GetTitle()
    legend.AddEntry(datahist, datalabel, "pe1")

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
    nbins = datahist.GetNbinsX()
    xlims = (datahist.GetBinLowEdge(1),
	     datahist.GetBinLowEdge(nbins)+datahist.GetBinWidth(nbins))
    # get and set y-limits
    (totmin,totmax) = ht.getminmax(histlist+[datahist])
    # in case of log scale
    if logy:
        pad1.SetLogy()
	if ylims is None: ylims = (totmin*yminlogfactor, totmax*ymaxlogfactor)
    # in case of lin scale
    else:
	if ylims is None: ylims = (0.,totmax*ymaxlinfactor)
    if yminzero and ylims[0]<0: ylims = (0.,ylims[1])
    datahist.SetMaximum(ylims[1])
    datahist.SetMinimum(ylims[0])

    # X-axis layout
    xax = datahist.GetXaxis()
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
    yax = datahist.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    if yaxtitle is not None:
	yax.SetTitle(yaxtitle)
	yax.SetTitleFont(10*axtitlefont+3)
	yax.SetTitleSize(axtitlesize)
	yax.SetTitleOffset(ytitleoffset)

    ### draw all objects in upper pad
    # first draw reference histogram to get axes right
    datahist.Draw()
    # draw other histograms
    for hist in histlist:
        hist.Draw("same "+histdrawoptions)
    # draw functions
    for func in funclist:
	func.Draw("same")
    # re-draw datahist on top
    datahist.Draw("same")
    # draw legend
    if dolegend:
	legend.Draw("same")
    # re-draw axes on top
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
    xax = ratiohist.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    if xaxtitle is not None:
	xax.SetTitle(xaxtitle)
	xax.SetTitleFont(10*axtitlefont+3)
	xax.SetTitleSize(axtitlesize)
	xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = ratiohist.GetYaxis()
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

    # first draw reference histogram to get axes right
    ratiohist.Draw()
    # now draw unit line
    ratiofunc.Draw("same")
    # re-draw ratio hist on top
    ratiohist.Draw("same")
    # re-draw axes on top
    ROOT.gPad.RedrawAxis()

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

