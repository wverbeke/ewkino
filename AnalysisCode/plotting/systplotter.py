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
        keep = False
        for el in mustcontain:
            if el in hist.GetName(): keep = True; break;
	if not keep: continue
        for el in mustnotcontain:
            if el in hist.GetName(): keep = False; break;
        if not keep: continue
        # add hist to list
        histlist.append(hist)
    return histlist

def findbytitle(histlist,title):
    # find a histogram by its title, return the index or -1 if not found
    index = -1
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: index = i
    return index

def findbyname(histlist,tag):
    indices = []
    for i,hist in enumerate(histlist):
	if tag in hist.GetName():
	    indices.append(i)
    if len(indices)>1:
	print('### ERROR ###: multiple histograms corresponding to "'+tag+'" in name')
	return None
    return indices[0]

def sethiststyle(hist,variable):
    # set color and line properties of a histogram
    systematic = hist.GetName().split(variable)[-1].strip('_')
    hist.SetLineWidth(3)
    hist.SetLineColor(getcolorsyst(systematic))
    if('Up' in systematic):
	hist.SetLineStyle(2)
    elif('Down' in systematic):
	hist.SetLineStyle(3)

def getcolorsyst(systematic):
    # return a color corresponding to a given systematic
    # for now return same color for up and down, maybe later asymmetrize
    if(systematic=='nominal'): return ROOT.kBlack
    if(systematic=='JECUp' or systematic=='JECDown'): return ROOT.kRed
    if(systematic=='JERUp' or systematic=='JERDown'): return ROOT.kRed+2
    if(systematic=='UnclUp' or systematic=='UnclDown'): return ROOT.kOrange-3
    if(systematic=='pileupUp' or systematic=='pileupDown'): return ROOT.kCyan
    if(systematic=='muonIDUp' or systematic=='muonIDDown'): return ROOT.kBlue
    if(systematic=='electronIDUp' or systematic=='electronIDDown'): return ROOT.kBlue-7
    if(systematic=='bTag_heavyUp' or systematic=='bTag_heavyDown'): return ROOT.kGreen+1
    if(systematic=='bTag_lightUp' or systematic=='bTag_lightDown'): return ROOT.kGreen+3
    if(systematic=='prefireUp' or systematic=='prefireDown'): return ROOT.kGreen-5
    if(systematic=='fScaleUp' or systematic=='fScaleDown'): return ROOT.kMagenta+1
    if(systematic=='rScaleUp' or systematic=='rScaleDown'): return ROOT.kMagenta+3
    if(systematic=='scalesUp' or systematic=='scalesDown'): return ROOT.kViolet
    if(systematic=='isrScaleUp' or systematic=='isrScaleDown'): return ROOT.kYellow+1
    if(systematic=='fsrScaleUp' or systematic=='fsrScaleDown'): return ROOT.kYellow-6
    if('pdfvar' in systematic): return ROOT.kGray
    if(systematic=='pdfEnvUp' or systematic=='pdfEnvDown'): return ROOT.kViolet+2
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
    totmax = 0.
    totmin = 1.
    for hist in histlist:
	for i in range(1,hist.GetNbinsX()+1):
	    val = hist.GetBinContent(i)
	    if val > totmax: totmax = val
	    if val < totmin: totmin = val
    topmargin = (totmax-totmin)/2.
    bottommargin = (totmax-totmin)/5.
    return (totmin-bottommargin,totmax+topmargin)

def histlisttotxt(histlist,txtfile):
    txtfile = txtfile.split('.')[-1]+'.txt'
    with open(txtfile,'w') as txtf:
	for hist in histlist:
	    toprint = '{:<15}'.format(hist.GetTitle())
	    for i in range(1,hist.GetNbinsX()+1):
		toprint += '\t{:<5}'.format('{0:.3f}'.format(hist.GetBinContent(i)))
	    toprint += '\n'
	    txtf.write(toprint)

def plotsystematics(mchistlist,variable,yaxtitle,xaxtitle,outfile,relative=True,errorbars=False,
		    outtxtfile=""):
    
    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    # fonts and sizes:
    #titlefont = 6; titlesize = 60
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
    #infofont = 6; infosize = 40
    legendfont = 4; legendsize = 12
    # title offset
    ytitleoffset = 2.
    xtitleoffset = 1.
    # margins:
    ptopmargin = 0.07
    pbottommargin = 0.12
    leftmargin = 0.2
    rightmargin = 0.05
    # legend box
    plegendbox = [leftmargin+0.03,1-ptopmargin-0.25,1-rightmargin-0.03,1-ptopmargin-0.03]

    ### get nominal histogram and remove from the list
    nominalindex = findbyname(mchistlist,"nominal")
    if(nominalindex<0): 
	print('### ERROR ###: nominal histogram not found.')
	return
    nominalhist = mchistlist[nominalindex]
    indices = list(range(len(mchistlist)))
    indices.remove(nominalindex)
    mchistlist = [mchistlist[i] for i in indices]

    ### operations on mc histograms
    sethiststyle(nominalhist,variable)
    clip(nominalhist)
    for hist in mchistlist:
        sethiststyle(hist,variable)
	clip(hist)
    if relative: 
	for hist in mchistlist+[nominalhist]:
	    for i in range(0,hist.GetNbinsX()+2):
		if nominalhist.GetBinContent(i)==0:
		    hist.SetBinContent(i,1.)
		    hist.SetBinError(i,0.)
		else:
		    hist.SetBinContent(i,hist.GetBinContent(i)/nominalhist.GetBinContent(i))
		    hist.SetBinError(i,hist.GetBinError(i)/nominalhist.GetBinContent(i))
    
    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
    legend.SetNColumns(4)
    legend.SetFillStyle(0)
    legend.SetTextFont(legendfont)
    legend.SetTextSize(legendsize)
    for hist in mchistlist:
	label = hist.GetName().split(variable)[-1].strip('_')
	if label[-2:]=='Up': label = '~Up'
        legend.AddEntry(hist,label,"l")
    legend.AddEntry(nominalhist,nominalhist.GetTitle(),"l")

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetTopMargin(ptopmargin)
    pad1.SetBottomMargin(pbottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.Draw()
    
    ### make upper part of the plot
    pad1.cd()
    (rangemin,rangemax) = getminmax(mchistlist)
    #(rangemin,rangemax) = (0.74,1.31) # temp override
    if not relative: rangemin = 0.
    nominalhist.SetMinimum(rangemin)
    nominalhist.SetMaximum(rangemax)

    # X-axis layout
    xax = nominalhist.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
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
    if errorbars: # for now draw only error bars of nominal sample!
	nominalhist.Draw("hist e")
	for hist in mchistlist:
	    hist.Draw("hist same")
    else:
	nominalhist.Draw("hist")
	for hist in mchistlist:
	    hist.Draw("hist same")
    legend.SetFillColor(ROOT.kWhite)
    legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    tools.drawLumi(pad1,lumitext="simulation")

    ### save the plot
    c1.SaveAs(outfile+'.png')
    ### save txt files with values if requested
    if len(outtxtfile)>0: histlisttotxt([nominalhist]+mchistlist,outtxtfile)

if __name__=="__main__":
    
    ### Configure input parameters (hard-coded)
    # file to read the histograms from
    histdir = os.path.abspath('../systematics/output/2016MC/wzcontrolregion_3prompt')
    variable = '_dPhill_max'

    ### Overwrite using cmd args
    if(len(sys.argv)==2):
	histdir = os.path.abspath(sys.argv[1])
    elif(not len(sys.argv)==1):
	print('### ERROR ###: wrong number of command line args')
	sys.exit()

    figdir = 'systplotter_output'
    if os.path.exists(figdir):
        os.system('rm -r '+figdir)
    os.makedirs(figdir)
    filelist = [os.path.join(histdir,f) for f in os.listdir(histdir) if f[-5:]=='.root']

    for histfile in filelist:
	histlist = loadhistograms(histfile,mustcontain=[variable])
	figname = os.path.join(figdir,histfile.split('/')[-1].rstrip('.root'))

	### Set plot properties
	binwidth = histlist[0].GetBinWidth(1)
	if binwidth.is_integer():
	    yaxtitle = 'events / '+str(int(binwidth)) # maybe find a way to get variable unit here
	else:
	    yaxtitle = 'events / {0:.2f}'.format(binwidth) # maybe find a way to get variable unit here
	xaxtitle = histlist[0].GetXaxis().GetTitle()
	plotsystematics(histlist,variable,yaxtitle,xaxtitle,figname+'_abs',relative=False,errorbars=True)
	plotsystematics(histlist,variable,yaxtitle,xaxtitle,figname+'_rel',relative=True,errorbars=False,
			outtxtfile=figname+'_tab')
