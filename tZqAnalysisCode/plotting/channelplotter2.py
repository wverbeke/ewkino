##########################################################################
# alternative channel plotting allowing for theory uncertainties as well #
##########################################################################

import sys
import os
import math
import ROOT

import plottools as pt
import channelplotter as cp

def readchanneltxt(paths_to_txtfiles):
    ### read a txt file and return lists of channels that can be used for plotting
    # see also readchanneltxt from channelplotter 
    # input: lists of files, they must have the same names and labels (also same order)!
    channels = []
    for f in paths_to_txtfiles:
	channels.append(cp.readchanneltxt(f))
    labels = [channel[1] for channel in channels[0]]
    names = [channel[0] for channel in channels[0]]
    for i in range(1,len(channels)):
	if [channel[1] for channel in channels[i]]!=labels:
	    raise Exception('ERROR: channel labels in files not compatible')
	if [channel[0] for channel in channels[i]]!=names:
	    raise Exception('ERROR: channel names in files not compatible')
    return channels

def plotchannels( thchannels, exchannels, outfile,
		    showvalues=True,font=None,fontsize=None, 
		    thbandlabels=None, exbandlabels=None, 
		    xaxtitle='', yaxtitle='', lumi='',
		    xaxcentral=None, xaxlinecoords=[], yaxlinecoords=[],
                    xaxrange=None, legendbox=None,
		    extracmstext='', style='whiskers' ):
    ### make a plot of different channels and compare to theory predictions
    # input arguments: similar to channelplotter.plotchannels with extensions:
    # - allowing for theory entries as well
    # - thchannels and exchannels can contain an arbitrary number of bands to color,
    #   with legend entries given in thbandlabels and exbandlabels

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    ROOT.gStyle.SetOptStat(0)

    # parse arguments and do some checks
    chlabels = sorted([channel[0] for channel in thchannels])
    if sorted([channel[0] for channel in exchannels])!=chlabels:
        raise Exception('ERROR: labels in theory and experimental channels not compatible')
    if thbandlabels is not None:
	if len(thbandlabels)!=int((len(thchannels[0])-2)/2):
	    raise Exception('th. band labels do not correspond to number of entries per channel')
    if exbandlabels is not None:
	if len(exbandlabels)!=int((len(exchannels[0])-2)/2):
	    raise Exception('exp. band labels do not correspond to number of entries per channel')
    if font is None: font = 42
    if fontsize is None: fontsize = 0.04
    lumi = str(lumi)
    dolumi = True
    if lumi=='': dolumi=False
    if xaxrange is None: xaxrange=(0,1.5)
    docentral=True
    if xaxcentral is None:
        docentral = False
        xaxcentral = 0
    if legendbox is None:
        legendbox = [0.75, 0.80, 0.90, 0.90]
    
    # set style options
    # default 'whiskers' style
    thfillcolorlist = [ROOT.kGray,ROOT.kGray+1]
    thfillstyle = 1001
    thlinecolorlist = [ROOT.kBlack,ROOT.kBlack+1]
    thlinewidth = 0
    thdrawoptions = "P 2"
    thlegendattr = "f"
    #exfillcolorlist = [ROOT.kOrange-4,ROOT.kCyan-4]
    exfillcolorlist = [ROOT.kAzure, ROOT.kRed]
    exfillstyle = 1001
    #exlinecolorlist = [ROOT.kOrange-4,ROOT.kCyan-4]
    exlinecolorlist = [ROOT.kAzure, ROOT.kRed]
    exlinewidth = 5
    exdrawoptions = "P"
    exlegendattr = "l"
    exverterrorsize = 0.
    ROOT.gStyle.SetEndErrorSize(7)
    if style=='boxes':
	print('making plot in "boxes" style')
	thfillcolorlist = [ROOT.kGray,ROOT.kGray+1]
	thfillstyle = 1001
	thlinecolorlist = [ROOT.kBlack,ROOT.kBlack]
	thlinewidth = 1
	thdrawoptions = "P 2"
	thlegendattr = "f"
	exfillcolorlist = [ROOT.kOrange-4,ROOT.kCyan-4]
	exfillstyle = 1001
	exlinecolorlist = [ROOT.kBlack,ROOT.kBlack]
	exlinewidth = 1
	exdrawoptions = "P 2"
	exlegendattr = "f"
	exverterrorsize = 0.1
    elif style!='whiskers':
	style = 'whiskers'
	print('WARNING: style "{}" not recognized,'.format(style)
		+' using default "whiskers" style.')

    # intializations
    c1 = ROOT.TCanvas("c1")
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetBottomMargin(0.15)
    pad1.SetLeftMargin(0.1)
    pad1.SetTopMargin(0.07)
    pad1.Draw()
    pad1.cd()
    nchannels = len(thchannels)
    thbands = int((len(thchannels[0])-2)/2)
    exbands = int((len(exchannels[0])-2)/2)
    thgraphs = []
    for i in range(thbands):
	g = ROOT.TGraphAsymmErrors(nchannels)
	thgraphs.append(g)
    exgraphs = []
    for i in range(exbands):
	g = ROOT.TGraphAsymmErrors(nchannels)
	exgraphs.append(g)
    labels = []

    # define a reference histogram
    # (for getting the axes right)
    refhist = None
    if thchannels>0: refhist = thgraphs[0]
    elif exchannels>0: refhist = exgraphs[0] 
    else:
	raise Exception('ERROR: found zero theory and zero experimental bands')

    # loop over theory channels and make points and appropriate labels
    for cnum,channel in enumerate(thchannels):
        label = str(channel[1])
        central = float(channel[2])
	for bnum in range(thbands):
	    # fill bands outwards in
	    up = float(channel[3+2*(thbands-bnum)-2])
	    down = float(channel[3+2*(thbands-bnum)-1])
	    thgraphs[bnum].SetPoint(cnum, central, cnum+1)
	    thgraphs[bnum].SetPointError(cnum, down, up, 0.5, 0.5)
    
    # also flip order labels
    thbandlabels = thbandlabels[::-1]

    # loop over experimental channels and make points and appropriate labels
    for cnum,channel in enumerate(exchannels):
        label = str(channel[1])
        central = float(channel[2])
        statup = float(channel[3])
        statdown = float(channel[4])
        totup = float(channel[5])
        totdown = float(channel[6])
        systup = math.sqrt(totup**2-statup**2)
        systdown = math.sqrt(totdown**2-statdown**2)

        exgraphs[0].SetPoint(cnum, central, cnum+1)
        exgraphs[0].SetPointError(cnum, totdown, totup, exverterrorsize, exverterrorsize)

        exgraphs[1].SetPoint(cnum, central, cnum+1)
        exgraphs[1].SetPointError(cnum, statdown, statup, exverterrorsize, exverterrorsize)

	if showvalues:
	    centraltxt = '{:.2f}'.format(central)
	    statuptxt = '{:.2f}'.format(statup)
	    statdowntxt = '{:.2f}'.format(statdown)
	    systuptxt = '{:.2f}'.format(systup)
	    systdowntxt = '{:.2f}'.format(systdown)
	    valtxt = centraltxt
	    valtxt += '^{'+statuptxt+'}_{-'+statdowntxt+'} (stat)'
	    valtxt += '^{+'+systuptxt+'}_{-'+systdowntxt+'} (syst)'
	    label += ': '+valtxt

        labels.append(label)

    # also flip order of labels
    exbandlabels = exbandlabels[::-1]

    # set style for theory bands
    if thbands>len(thfillcolorlist):
	raise Exception('ERROR: not enough fill colours for theory bands...')
    if thbands>len(thlinecolorlist):
	raise Exception('ERROR: not enough line colours for theory bands...')
    for i in range(thbands):
	thgraphs[i].SetFillStyle(thfillstyle)
	thgraphs[i].SetFillColor(thfillcolorlist[i])
	thgraphs[i].SetLineWidth(thlinewidth)
	thgraphs[i].SetLineColor(thlinecolorlist[i])
	thgraphs[i].SetMarkerStyle(0)
	thgraphs[i].SetMarkerSize(0)
	# (apparently for eps format SetMarkerStyle(0) does not work,
	#  need to set the size to zero explicitly to hide the markers...)

    # set style for experimental bands
    if exbands>len(exfillcolorlist):
	raise Exception('ERROR: not enough fill colours for experimental bands...')
    if exbands>len(exlinecolorlist):
	raise Exception('ERROR: not enough line colours for experimental bands...')
    for i in range(exbands):
        exgraphs[i].SetFillStyle(exfillstyle)
        exgraphs[i].SetFillColor(exfillcolorlist[i])
	exgraphs[i].SetLineWidth(exlinewidth)
	exgraphs[i].SetLineColor(exlinecolorlist[i])
        exgraphs[i].SetMarkerStyle(20)

    # draw first time to have well-defined axes
    refhist.Draw("A "+thdrawoptions) # (option A for drawing new axes!)
    pad1.Update()
    c1.Update()

    # x-axis layout
    xaxis = refhist.GetXaxis()
    xaxis.SetLimits(xaxrange[0],xaxrange[1])
    xaxis.SetTitle(xaxtitle)
    xaxis.SetTitleSize(0.05)
    xaxis.SetLabelSize(0.04)

    # y-axis layout
    yaxis = refhist.GetYaxis()
    yaxis.SetTitle(yaxtitle)
    yaxis.SetTitleSize(0.05)
    yaxis.SetTitleOffset(0.5)
    yaxis.SetLabelSize(0)
    yaxis.SetTickLength(0)

    # unit line
    sm = ROOT.TLine(xaxcentral,0.5,xaxcentral,nchannels+0.5)
    sm.SetLineColor(ROOT.kBlack)
    sm.SetLineWidth(1)
    sm.SetLineStyle(9)

    # additional lines
    vertlines = []
    for xcoord in xaxlinecoords:
        vertlines.append( ROOT.TLine(xcoord,0.3,xcoord,nchannels+0.7) )
        vertlines[-1].SetLineWidth(1)
        vertlines[-1].SetLineStyle(3)
    horlines = []
    for ycoord in yaxlinecoords:
        horlines.append( ROOT.TLine(xaxrange[0],ycoord,xaxrange[1],ycoord) )
        horlines[-1].SetLineWidth(1)
        horlines[-1].SetLineStyle(3)

    # legend
    leg = ROOT.TLegend(legendbox[0],legendbox[1],legendbox[2],legendbox[3])
    leg.SetBorderSize(0)
    leg.SetFillStyle(0)
    leg.AddEntry(exgraphs[0],'Measurement',"p")
    for i in range(thbands)[::-1]:
	leg.AddEntry(thgraphs[i], thbandlabels[i], thlegendattr)
    for i in range(exbands)[::-1]:
	leg.AddEntry(exgraphs[i], exbandlabels[i], exlegendattr)
    # special treatment for "whiskers" style:
    # since option "e" will show vertical error bars and "l" will not show vertical edge ticks,
    # need to work around by plotting a point at the legend position...
    # very hard to do in a clean way, so consider this temporary...
    if style=='whiskers':
	legendgraph_exstat = ROOT.TGraphAsymmErrors(1)
        legendgraph_exstat.SetPoint(0, 1.23, 4.9)
        legendgraph_exstat.SetPointError(0, 0.05, 0.05, 0., 0.)
	legendgraph_extot = ROOT.TGraphAsymmErrors(1)
        legendgraph_extot.SetPoint(0, 1.23, 4.55)
        legendgraph_extot.SetPointError(0, 0.05, 0.05, 0., 0.)
	for i,g in enumerate([legendgraph_extot,legendgraph_exstat]):
	    g.SetFillStyle(exfillstyle)
	    g.SetFillColor(exfillcolorlist[i])
	    g.SetLineWidth(exlinewidth)
	    g.SetLineColor(exlinecolorlist[i])
	    g.SetMarkerStyle(0)
	    g.SetMarkerSize(0)
	    g.Draw(exdrawoptions)


    # draw objects
    for g in thgraphs: g.Draw(thdrawoptions)
    for g in exgraphs: g.Draw(exdrawoptions)
    if docentral: sm.Draw()
    for vertline in vertlines: vertline.Draw()
    for horline in horlines: horline.Draw()
    for cnum,label in enumerate(labels):
        tex = ROOT.TLatex()
	tex.SetTextFont(font)
        tex.SetTextSize(fontsize)
	ypos = exgraphs[0].GetY()[cnum]
        yposndc = (ypos - ROOT.gPad.GetY1())/(ROOT.gPad.GetY2()-ROOT.gPad.GetY1())	
        tex.DrawLatexNDC(0.15,yposndc,label)
    leg.Draw()
    pt.drawLumi(pad1,extratext=extracmstext,lumitext=lumi+" fb^{-1} (13 TeV)",cms_in_grid=False)
    
    pad1.RedrawAxis()
    pad1.Update()
    c1.Update()

    ### save the plot
    c1.SaveAs(outfile.replace('.png','')+'.png')
    c1.SaveAs(outfile.replace('.png','')+'.pdf')
    c1.SaveAs(outfile.replace('.png','')+'.eps')
    

if __name__=='__main__':

    thfile = sys.argv[1]
    exfile = sys.argv[2]
    figfile = sys.argv[3]
    
    channels = readchanneltxt([thfile,exfile])
    thchannels = channels[0]
    exchannels = channels[1]

    thbandlabels = ['Theory']
    exbandlabels = ['Stat. Unc.', 'Total Unc.']

    xaxrange = (0.,1.8)
    xaxlinecoords = [0.6,0.8,1.2,1.4,1.6]
    yaxlinecoords = [0.5]
    for i in range(len(thchannels)):
	yaxlinecoords.append(1.5+i)
    legendbox = [0.65, 0.72, 0.96, 0.90]
    lumi = 138

    plotchannels( thchannels, exchannels, figfile, 
		    thbandlabels=thbandlabels,exbandlabels=exbandlabels,
		    xaxrange=xaxrange,xaxlinecoords=xaxlinecoords,xaxcentral=1.,
		    yaxlinecoords=yaxlinecoords,
		    legendbox=legendbox,showvalues=False,
		    lumi=lumi, xaxtitle='Observed / Predicted', yaxtitle='',
		    extracmstext='',
		    style='whiskers' )
