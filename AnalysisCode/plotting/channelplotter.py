############################################################
# make a plot of results for different channels to compare #
############################################################
# partly copied from Gianny's ttg code...

import sys
import os
import math
import ROOT
import plottools as pt

def readchanneltxt(path_to_txtfile):
    ### read a txt file and return a list of channels that can be used for plotchannels
    # expected formatting of txt file: label central statup statdown totup totdown
    # note that label may not contain spaces, but underscores can be used instead
    channels = []
    f = open(path_to_txtfile,'r')
    for line in f:
	channel = line.split(' ')
	if len(channel)!=6:
	    raise Exception('### ERROR ### input txt file wrong format')
	channel[0] = channel[0].replace('_',' ')
	for i in range(len(channel))[1:]: channel[i] = float(channel[i])
	channels.append(channel)
    return channels

def plotchannels(channels,outfile,
		xaxtitle='',yaxtitle='',lumi='',xaxcentral=None,xaxrange=None):
    ### make the plot of different channels
    ### arguments:
    # - channels is a list of lists of the form [label,central,statup,statdown,totup,totdown]
    # - outfile: name of the output file to make (should not contain the file type extension!)
    # - xaxtitle, yaxtitle are axis titles (default: no title)
    # - lumi is luminosity value to put in header
    # - xaxcentral is x-axis value where to draw a vertical line (default: no vertical line)
    # - xaxrange is range of x axis (default: 0-1.5)

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    ROOT.gStyle.SetOptStat(0)

    # parse arguments
    lumi = str(lumi)
    dolumi = True
    if lumi=='': dolumi=False
    if xaxrange is None: xaxrange=(0,1.5)
    docentral = True
    if xaxcentral is None: 
	docentral = False
	xaxcentral = 0

    # intializations
    c1 = ROOT.TCanvas("c1")
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetBottomMargin(0.15)
    pad1.SetLeftMargin(0.1)
    pad1.SetTopMargin(0.07)
    pad1.SetGridx(1)
    pad1.SetGridy(1)
    pad1.Draw()
    pad1.cd()
    nchannels = len(channels)
    points = ROOT.TGraphAsymmErrors(nchannels)
    points_statonly = ROOT.TGraphAsymmErrors(nchannels)
    labels = []
    mg = ROOT.TMultiGraph()
    mg.Add(points)
    mg.Add(points_statonly)   

    # loop over channels and make points and appropriate labels
    for cnum,channel in enumerate(channels):
	label = str(channel[0])
	central = float(channel[1])
	statup = float(channel[2])
	statdown = float(channel[3])
	totup = float(channel[4])
	totdown = float(channel[5])
	systup = math.sqrt(totup**2-statup**2)
	systdown = math.sqrt(totdown**2-statdown**2)

	points.SetPoint(cnum, central, cnum+1)
	points.SetPointError(cnum, totdown, totup, 0.5, 0.5)
	
	points_statonly.SetPoint(cnum, central, cnum+1)
        points_statonly.SetPointError(cnum, statdown, statup, 0.5, 0.5)

	centraltxt = '{:.2f}'.format(central)
	statuptxt = '{:.2f}'.format(statup)
	statdowntxt = '{:.2f}'.format(statdown)
	systuptxt = '{:.2f}'.format(systup)
	systdowntxt = '{:.2f}'.format(systdown)
        valtxt = centraltxt
	valtxt += '^{'+statuptxt+'}_{-'+statdowntxt+'} (stat)'
	valtxt += '^{+'+systuptxt+'}_{-'+systdowntxt+'} (syst)'
	labels.append(label+' '+valtxt)

    # set style for points
    points.SetFillStyle(3001)
    points.SetFillColor(ROOT.kOrange-4)
    points.SetMarkerStyle(20)

    # set style for statonly points
    points_statonly.SetFillStyle(3001)
    points_statonly.SetFillColor(ROOT.kCyan-4)
    points_statonly.SetMarkerStyle(20)
    
    # draw first time to have well-defined axes
    mg.Draw("A2")
    pad1.Update()
    c1.Update()

    # x-axis layout
    mg.GetXaxis().SetLimits(xaxrange[0],xaxrange[1])
    mg.GetXaxis().SetTitle(xaxtitle)
    mg.GetXaxis().SetTitleSize(0.05)
    mg.GetXaxis().SetLabelSize(0.04)
    
    # y-axis layout
    mg.GetYaxis().SetTitle(yaxtitle)
    mg.GetYaxis().SetTitleSize(0.05)
    mg.GetYaxis().SetTitleOffset(0.5)
    mg.GetYaxis().SetLabelSize(0)
    
    # unit line
    sm = ROOT.TLine(xaxcentral,0.5,xaxcentral,nchannels+0.5)
    # style 1: gray band
    #sm.SetLineColor(ROOT.kGray)
    #sm.SetLineWidth(3)
    #sm.SetLineStyle(1)
    # style 2: black dashes
    sm.SetLineColor(ROOT.kBlack)
    sm.SetLineWidth(1)
    sm.SetLineStyle(9)

    # legend
    leg = ROOT.TLegend(0.75, 0.80, 0.90, 0.90)
    leg.SetBorderSize(0)
    leg.SetFillStyle(0)
    leg.AddEntry(points, 'stat+sys', "f")
    leg.AddEntry(points_statonly, 'stat', "f")

    # draw objects
    mg.Draw("P A2")
    if docentral: sm.Draw()
    for cnum,label in enumerate(labels):
	tex = ROOT.TLatex()
        tex.SetTextSize(0.03)
	ypos = points.GetY()[cnum]
	yposndc = (ypos - ROOT.gPad.GetY1())/(ROOT.gPad.GetY2()-ROOT.gPad.GetY1())
        tex.DrawLatexNDC(0.15,yposndc,label)
    leg.Draw()
    pt.drawLumi(pad1,lumitext=lumi+" fb^{-1} (13 TeV)",cms_in_grid=False)

    pad1.Update()
    c1.Update()

    ### save the plot
    c1.SaveAs(outfile.replace('.png','')+'.png')
    c1.SaveAs(outfile.replace('.png','')+'.pdf')

if __name__=='__main__':

    if len(sys.argv)!=3:
	print('### ERROR ###: incorrect number of command line arguments')
	print('               need <inputfile> <outputfile>')
	sys.exit()

    txtfile = os.path.abspath(sys.argv[1])
    figfile = sys.argv[2]
    if not os.path.exists(txtfile):
	print('### ERROR ###: input file does not seem to exist...') 
	sys.exit()
    
    channels = readchanneltxt(txtfile)
    #print(channels)

    xaxrange = (-1,1.75)

    plotchannels(channels,figfile,
		xaxcentral=1,xaxrange=xaxrange,
		lumi=137,xaxtitle='signal strength',yaxtitle='channel')
    print('done!')
