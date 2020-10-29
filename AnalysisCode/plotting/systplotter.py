##########################################################
# Process and plot histograms with systematic variations #
##########################################################
# to be used on output files of runsystematics.py

import ROOT
import sys
import numpy as np
import math
import json
import os
import plottools as tools

def loadhistograms(histfile,mustcontain=[],mustnotcontain=[]):
    # load histograms from a root file.
    # 'histfile' is a string containing the path to the input root file.
    # the output is a list of histograms
    # note: histogram name must contain ALL tags in mustcontain and NONE in mustnotcontain
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
            if not el in hist.GetName(): keep = False; break;
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
	hist.SetLineStyle(0)
    elif('Down' in systematic):
	hist.SetLineStyle(2)

def getcolorsyst(systematic):
    # return a color corresponding to a given systematic
    # for now return same color for up and down, maybe later asymmetrize

    # acceptance uncertainties in shades of red
    if(systematic=='nominal'): return ROOT.kBlack
    if(systematic=='JECUp' or systematic=='JECDown'): return ROOT.kRed
    if(systematic=='JERUp' or systematic=='JERDown'): return ROOT.kRed+2
    if(systematic=='UnclUp' or systematic=='UnclDown'): return ROOT.kOrange-3

    # lepton uncertainties in shades of blue
    if(systematic=='muonIDUp' or systematic=='muonIDDown'): return ROOT.kBlue
    if(systematic=='muonIDSystUp' or systematic=='muonIDSystDown'): return ROOT.kBlue
    if(systematic=='muonIDStatUp' or systematic=='muonIDStatDown'): return ROOT.kBlue+2
    if(systematic=='electronIDUp' or systematic=='electronIDDown'): return ROOT.kBlue-9
    if(systematic=='electronIDSystUp' or systematic=='electronIDSystDown'): return ROOT.kBlue-9
    if(systematic=='electronIDStatUp' or systematic=='electronIDStatDown'): return ROOT.kBlue-10
    if(systematic=='electronRecoUp' or systematic=='electronRecoDown'): return ROOT.kBlue-5

    # other weights in shades of green
    if(systematic=='pileupUp' or systematic=='pileupDown'): return ROOT.kGreen-7
    if(systematic=='bTag_heavyUp' or systematic=='bTag_heavyDown'): return ROOT.kGreen+1
    if(systematic=='bTag_lightUp' or systematic=='bTag_lightDown'): return ROOT.kGreen+3
    if(systematic=='prefireUp' or systematic=='prefireDown'): return ROOT.kGreen-5

    # scales in shaded of purple
    # first three are obsolete and replaced by qcdScalesShapeEnv and qcdScalesNorm
    if(systematic=='fScaleUp' or systematic=='fScaleDown'): return ROOT.kMagenta
    if(systematic=='rScaleUp' or systematic=='rScaleDown'): return ROOT.kMagenta+2
    if(systematic=='rfScalesUp' or systematic=='rfScalesDown'): return ROOT.kMagenta+3
    if(systematic=='isrScaleUp' or systematic=='isrScaleDown'): return ROOT.kViolet+1
    if(systematic=='fsrScaleUp' or systematic=='fsrScaleDown'): return ROOT.kViolet+2
    
    # qcd scale variations in magenta
    if('qcdScalesShapeVar' in systematic): return ROOT.kGray
    if(systematic=='qcdScalesShapeEnvUp' or systematic=='qcdScalesShapeEnvDown'): 
	return ROOT.kMagenta-9
    if(systematic=='qcdScalesNormUp' or systematic=='qcdScalesNormDown'): 
	return ROOT.kViolet+6
    
    # pdf variations in yellow
    if('pdfShapeVar' in systematic): return ROOT.kGray
    if(systematic=='pdfShapeEnvUp' or systematic=='pdfShapeEnvDown'): return ROOT.kOrange
    if(systematic=='pdfShapeRMSUp' or systematic=='pdfShapeRMSDown'): return ROOT.kOrange+7
    if(systematic=='pdfNormUp' or systematic=='pdfNormDown'): return ROOT.kOrange+10

    print('### WARNING ###: tag not recognized (in getcolorsyst): '+str(systematic))
    return ROOT.kBlack

def clip(hist):
    # set minimum value for all bins to zero (no net negative weight)
    for i in range(hist.GetNbinsX()+1):
	if hist.GetBinContent(i)<0:
	    hist.SetBinContent(i,0.)
	    hist.SetBinError(i,0)

def getminmax(histlist,witherrors=False):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 0.
    totmin = 1.
    for hist in histlist:
	for i in range(1,hist.GetNbinsX()+1):
	    val = hist.GetBinContent(i)
	    upval = val
	    downval = val
	    if witherrors:
		upval = val + hist.GetBinError(i)
		downval = val - hist.GetBinError(i)
	    if upval > totmax: totmax = upval
	    if downval < totmin: totmin = downval
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
		    hist.SetBinError(i,hist.GetBinError(i)/nominalhist.GetBinContent(i))
		    hist.SetBinContent(i,hist.GetBinContent(i)/nominalhist.GetBinContent(i))
    
    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
    legend.SetFillStyle(0)
    legend.SetTextFont(legendfont)
    legend.SetTextSize(legendsize)
    nentries = 0
    for hist in mchistlist:
	label = hist.GetName().split(variable)[-1].strip('_')
	if label[-2:]=='Up': label = '~Up'
	# avoid drawing a legend entry for all shape variations
	if('ShapeVar0' in label): label = label[:label.find('Var0')]
	elif('ShapeVar' in label): continue
        legend.AddEntry(hist,label,"l")
	nentries += 1
    legend.AddEntry(nominalhist,nominalhist.GetTitle(),"l")
    # determine number of columns for legend based on number of entries
    ncolumns = int(math.ceil(float(nentries)/8))
    legend.SetNColumns(ncolumns)

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
    # determine plot range based on minimum and maximum variation
    (rangemin,rangemax) = getminmax(mchistlist)
    # if drawing error bars, also take statistical variation into account
    if errorbars:
	(srangemin,srangemax) = getminmax([nominalhist],witherrors=True)
	#rangemin = min(rangemin,srangemin)
	#rangemax = max(rangemax,srangemax)
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
    erroroption = ''
    if errorbars: erroroption = ' e' # for now draw only error bars of nominal sample!
    nominalhist.Draw("hist"+erroroption)
    for hist in mchistlist:
	# draw ShapeVar histograms first to put them in background
	if not 'ShapeVar' in hist.GetName(): continue
	hist.Draw("hist same")
    for hist in mchistlist:
	# now draw all other histograms
	if 'ShapeVar' in hist.GetName(): continue
	hist.Draw("hist same")
    # redraw nominal to put it on top
    nominalhist.Draw("hist same"+erroroption)
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
    # file(s) to read the histograms from
    # if the argument is a .root file, will use this file
    # else will assume it's a folder and run on all .root files within
    histdir = os.path.abspath('../systematics/test')
    # variable to plot
    variables = []
    variables = (['_abs_eta_recoil','_Mjj_max','_lW_asymmetry',
                '_deepCSV_max','_lT','_MT','_dPhill_max',
                '_pTjj_max','_dRlb_min','_HT','_dRlWrecoil','_dRlWbtagged',
                '_M3l','_abs_eta_max','_nJets'])
    variables += ['_eventBDT']
    variables += ['_nMuons','_nElectrons']

    # systematics to draw
    #systematictags = [] # empty list for all systematics in file
    systematictags = []
    systematictags += ['_rfScales','_qcdScale']
    # systematics to exclude
    excludetags = [] # empty list to exclude nothing
    #excludetags = ['Var','rScale','fScale']

    ### Overwrite using cmd args
    if(len(sys.argv)==2):
	histdir = os.path.abspath(sys.argv[1])
    elif(not len(sys.argv)==1):
	print('### ERROR ###: wrong number of command line args')
	sys.exit()

    ### set output directory
    figdir = 'systplotter_output'
    if os.path.exists(figdir):
        os.system('rm -r '+figdir)
    os.makedirs(figdir)
    
    ### set input files to run on
    filelist = []
    if histdir[-5:]=='.root': filelist.append(histdir)
    else: filelist = [os.path.join(histdir,f) for f in os.listdir(histdir) if f[-5:]=='.root']

    ### loop over files and variables
    for histfile in filelist:
	for variable in variables:
	    print('running on {}/{}'.format(histfile,variable))
	    histlist = []
	    if len(systematictags)==0:
		histlist = loadhistograms(histfile,mustcontain=[variable],
						mustnotcontain=excludetags)
	    else:
		for systematictag in systematictags+['nominal']:
		    histlist += loadhistograms(histfile,mustcontain=[variable,systematictag],
						    mustnotcontain=excludetags)
		figname = os.path.join(figdir,histfile.split('/')[-1].rstrip('.root')+'_'+variable)

	    ### Set plot properties
	    binwidth = histlist[0].GetBinWidth(1)
	    yaxtitle = 'yield'
	    relyaxtitle = 'normalized yield'
	    xaxtitle = histlist[0].GetXaxis().GetTitle().strip('_')
	    plotsystematics(histlist,variable,yaxtitle,xaxtitle,figname+'_abs',relative=False,
				errorbars=True)
	    plotsystematics(histlist,variable,relyaxtitle,xaxtitle,figname+'_rel',relative=True,
				errorbars=True,outtxtfile=figname+'_tab')
