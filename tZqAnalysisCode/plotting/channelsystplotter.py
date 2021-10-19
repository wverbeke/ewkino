####################################################################################
# make a plot of systematic variations for different channels/categories/files/... #
####################################################################################

import sys
import os
import ROOT
import plottools as pt

def readchanneltxt(path_to_txtfile):
    ### read a txt file and return a list of channels that can be used for plotchannelsyst
    # expected formatting of txt file: label systematic up down
    # (note: use multiple lines with the same label for different systematics on that channel)
    # note that label may not contain spaces, but underscores can be used instead
    channels = {}
    f = open(path_to_txtfile,'r')
    for line in f:
        entry = line.split(' ')
        if len(entry)!=4:
            raise Exception('### ERROR ### input txt file wrong format')
        channelname = entry[0].replace('_',' ')
	# if channel does not already exist, create a new one
	if not channelname in channels.keys():
	    channels[channelname] = {entry[1]:(float(entry[2]),float(entry[3]))}
	# else, add the systematic to an existing entry
	channels[channelname][entry[1]] = (float(entry[2]),float(entry[3]))
    return channels

def getallsystematics(channels):
    ### get a list of all systematics (usually the same for each channel but not necessarily)
    syslist = []
    for channel in channels.keys():
	for sys in channels[channel].keys():
	    if not sys in syslist: syslist.append(sys)
    return syslist

def plotchannelsyst(channels,xaxtitle='',yaxtitle='',lumi='',yaxrange=None,
		    colormap=None,style='graph'):
    ### make a plot of systematic impacts on yield for different channels
    ### arguments:
    # - channels is a dict of channelname to dict of sysname to tuple of (up,down)
    #   (e.g. output of readchanneltxt)
    # - xaxtitle, yaxtitle are axis titles (default: no title)
    # - lumi is luminosity value to put in header
    # - yaxrange is range of y axis (default: ?)
    # - colormap is a dict of strings (systematics names) to ROOT colors
    # - style can be either 'graph' (with TGraphAsymmErrors) or 'hist' (with TH1)
    #   ('graph' is more appropriate of not all channnels have the same systematics,
    #   'hist' will give smoother plots if all systematics are shared between channels)
    
    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    ROOT.gStyle.SetOptStat(0)

    # parse arguments
    lumi = str(lumi)
    dolumi = True
    if lumi=='': dolumi=False
    if yaxrange is None: yaxrange=(0.5,1.5) # automate min and max later on
    if style not in ['graph','hist']:
	print('### ERROR ### style {} not recognized'.format(style))
    syslist = sorted(getallsystematics(channels))
    # if no colormap, use default colors
    if colormap is None:
        clist = ([ROOT.kRed-7,ROOT.kOrange,ROOT.kCyan-7,ROOT.kYellow+1,ROOT.kBlue-10,
                    ROOT.kBlue-6,ROOT.kTeal-5,ROOT.kMagenta-7])
        colormap = {}
        for i,sys in enumerate(syslist):
            colormap[sys] = clist[ i%len(clist) ]

    # fonts and sizes:
    labelfont = 4; labelsize = 20
    axtitlefont = 4; axtitlesize = 25
    # title offset
    ytitleoffset = 1.1
    xtitleoffset = 0.5
    # margins:
    topmargin = 0.07
    bottommargin = 0.1
    leftmargin = 0.15
    rightmargin = 0.2
    # legend box
    legendbox = [1-rightmargin+0.03,bottommargin+0.03,1-0.03,1-topmargin-0.03]
    # line width
    linewidth = 2

    # other intializations
    nchannels = len(channels)
    c1 = ROOT.TCanvas("c1")
    c1.SetCanvasSize(700+100*max(0,nchannels-5),500)
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetBottomMargin(bottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetTopMargin(topmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetGridx(1)
    pad1.SetGridy(1)
    pad1.SetFrameLineWidth(2)
    pad1.Draw()
    pad1.cd()
    nchannels = len(channels)
    labels = []
    mg = ROOT.TMultiGraph() # for style 'graph' 
    graphdict = {} # for style 'graph'
    histdict = {} # for style 'hist'
    leg = ROOT.TLegend(legendbox[0], legendbox[1], legendbox[2], legendbox[3])
    leg.SetBorderSize(0)
    leg.SetNColumns(1)

    # add a tuple of graphs/hists (up and down) for each systematic
    for sys in syslist:
	# make graphs
	if style=='graph':
	    graphdict[sys] = (ROOT.TGraphAsymmErrors(nchannels),
				ROOT.TGraphAsymmErrors(nchannels))
	    for i in (0,1):
		graphdict[sys][i].SetLineColor( colormap.get(sys, ROOT.kBlack) )
		graphdict[sys][i].SetLineWidth(linewidth)
		graphdict[sys][i].SetMarkerSize(0)
		mg.Add(graphdict[sys][i])
	    leg.AddEntry(graphdict[sys][0],sys)
	# make hists
	if style=='hist':
	    histdict[sys] = (ROOT.TH1F('','',nchannels,0.5,nchannels+0.5),
			    ROOT.TH1F('','',nchannels,0.5,nchannels+0.5))
	    for i in (0,1):
		histdict[sys][i].SetLineColor( colormap.get(sys, ROOT.kBlack) )
		histdict[sys][i].SetLineWidth(linewidth)
	    leg.AddEntry(histdict[sys][0],sys,'l')

    channelnames = sorted(channels.keys())
    # loop over channels and systematics and add a graph point / hist bin content for each
    for cnum,channel in enumerate(channelnames):
	for sys in syslist:
	    if sys not in channels[channel]: continue
	    if style=='graph':
		graphdict[sys][0].SetPoint(cnum,cnum+1,channels[channel][sys][0])
		graphdict[sys][0].SetPointError(cnum,0.5,0.5,0,0)
		graphdict[sys][1].SetPoint(cnum,cnum+1,channels[channel][sys][1])
		graphdict[sys][1].SetPointError(cnum,0.5,0.5,0,0)
	    if style=='hist':
		histdict[sys][0].SetBinContent(cnum+1,channels[channel][sys][0])
		histdict[sys][1].SetBinContent(cnum+1,channels[channel][sys][1])
		# set under and overflow bins at same value as first and last bin
		#if cnum==0: 
		#    histdict[sys][0].SetBinContent(cnum,channels[channel][sys][0])
		#    histdict[sys][1].SetBinContent(cnum,channels[channel][sys][1])
		#if cnum==len(channels)-1:
		#    histdict[sys][0].SetBinContent(cnum+2,channels[channel][sys][0])
		#    histdict[sys][1].SetBinContent(cnum+2,channels[channel][sys][1])

    refobject = None
    if style=='graph': refobject = mg
    if style=='hist': refobject = histdict[histdict.keys()[0]][0]

    # draw a first time to have well-defined axes
    if style=='graph': refobject.Draw("AP")
    if style=='hist': refobject.Draw("hist")
    pad1.Update()
    c1.Update()

    # x-axis layout
    refobject.GetXaxis().SetLimits(0.5+0.001,nchannels+0.5-0.001)
    refobject.GetXaxis().SetTitle(xaxtitle)
    refobject.GetXaxis().SetTitleFont(10*axtitlefont+3)
    refobject.GetXaxis().SetTitleSize(axtitlesize)
    refobject.GetXaxis().SetTitleOffset(xtitleoffset)
    refobject.GetXaxis().SetLabelSize(0)

    # y-axis layout
    refobject.GetYaxis().SetRangeUser(yaxrange[0],yaxrange[1])
    refobject.GetYaxis().SetTitle(yaxtitle)
    refobject.GetYaxis().SetTitleFont(10*axtitlefont+3)
    refobject.GetYaxis().SetTitleSize(axtitlesize)
    refobject.GetYaxis().SetTitleOffset(ytitleoffset)
    refobject.GetYaxis().SetLabelFont(10*labelfont+3)
    refobject.GetYaxis().SetLabelSize(labelsize)
    refobject.GetYaxis().SetNdivisions(10,5,0,ROOT.kTRUE)

    # draw objects
    if style=='graph': mg.Draw("AP")
    if style=='hist':
	histdict[histdict.keys()[0]][0].Draw("hist ][")
	histdict[histdict.keys()[0]][1].Draw("hist same ][")
	for key in histdict.keys()[1:]: 
	    histdict[key][0].Draw("hist same ][")
	    histdict[key][1].Draw("hist same ][")
    for cnum,channel in enumerate(channelnames):
	tex = ROOT.TLatex()
        tex.SetTextSize(0.03)
        xpos = cnum+1
        xposndc = (xpos - ROOT.gPad.GetX1())/(ROOT.gPad.GetX2()-ROOT.gPad.GetX1())
	tex.SetTextAngle(89)
        tex.DrawLatexNDC(xposndc,0.15,channel)
    leg.Draw()
    pt.drawLumi(pad1,lumitext=lumi+" fb^{-1} (13 TeV)")

    pad1.Update()
    c1.Update()

    ### save the plot
    c1.SaveAs('test.png')
    #c1.SaveAs(outfile.replace('.png','')+'.png')


if __name__=='__main__':

    if len(sys.argv)!=2:
        print('### ERROR ###: incorrect number of command line arguments')
        print('               need <inputfile>')
        sys.exit()

    txtfile = os.path.abspath(sys.argv[1])
    if not os.path.exists(txtfile):
        print('### ERROR ###: input file does not seem to exist...')
        sys.exit()

    channels = readchanneltxt(txtfile)
    plotchannelsyst(channels,lumi=137,xaxtitle='channel',yaxtitle='systematic deviation',
			style='hist',colormap=pt.getcolormap('systematics'),
			yaxrange=[0.85,1.15])
    print('done!')
