#############################################################################
# Plot data and MC efficiencies (e.g. trigger efficiencies) and their ratio #
#############################################################################
# to be used after triggerefficiency.cc (in corresponding folder) which creates the histograms
# note: one mc histogram is assumed, while multiple data histograms can be overlayed (e.g. eras)!

import ROOT
import sys
import numpy as np
import json
import os
import plottools as tools

def loadobjects(rootfile,mustcontain=[],mustnotcontain=[]):
    # load hists and/or graphs from a root file.
    # 'rootfile' is a string containing the path to the input root file.
    # the output is a list of objects of type TH1D or TGraphAsymmError
    print('loading histograms...')
    f = ROOT.TFile.Open(rootfile)
    rootlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        rootobject = f.Get(key.GetName())
	# check if object needs to be included
        keep = True
        for el in mustcontain:
            if el not in rootobject.GetTitle(): keep = False; break;
        for el in mustnotcontain:
            if el in rootobject.GetTitle(): keep = False; break;
        if not keep: continue;
	# check if rootobject is readable
	objectisok = False
        try:
            rootobject.GetErrorYhigh(1) # try to use a specific TGraphAsymmError function
	    objectisok = True
        except:
            pass
	try:
	    rootobject.GetEntries()
	    rootobject.SetDirectory(0)
	    objectisok = True
	except:
	    pass
	if not objectisok:
	    print('### WARNING ###: object '+str(rootobject.GetTitle())+' cannot be read.')
	# add hist to list
        rootlist.append(rootobject)
    return rootlist

def findbytitle(histlist,title):
    # find a histogram by its title, return the index or -1 if not found
    index = -1
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: index = i
    return index

def getminmax(histlist):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 1.
    totmin = 1.
    for hist in histlist:
        for i in range(1,hist.GetNbinsX()+1):
            val = hist.GetBinContent(i)
            if val > totmax: totmax = val
            if val < totmin: totmin = val
    topmargin = (totmax-totmin)/2.
    bottommargin = (totmax-totmin)/5.
    return (totmin-bottommargin,totmax+topmargin)

def getcolor(hist,mode):
    if mode=='random':
	clist = [ROOT.kRed,ROOT.kOrange,ROOT.kYellow,ROOT,kSpring,ROOT.kGreen,ROOT.kTeal,
		ROOT.kCYan,ROOT.kAzure,ROOT.kBlue,ROOT.kViolet,ROOT.kMagenta,ROOT.kPink]
	return clist[np.random.randint(0,high=len(clist))]
    if mode=='triggerefficiency_singleyear':
	tag = formathisttitle(hist.GetTitle())
	if 'A' in tag: return ROOT.kRed-7
	elif 'B' in tag: return ROOT.kOrange-3
	elif 'C' in tag: return ROOT.kOrange
	elif 'D' in tag: return ROOT.kGreen+1
	elif 'E' in tag: return ROOT.kCyan+1
	elif 'F' in tag: return ROOT.kAzure
	elif 'G' in tag: return ROOT.kViolet
	elif 'H' in tag: return ROOT.kMagenta+3
	elif 'data' in tag: return ROOT.kRed
    if mode=='triggerefficiency_allyears':
	tag = formathisttitle(hist.GetTitle())
	if '2016' in tag: return ROOT.kOrange
	elif '2017' in tag: return ROOT.kGreen+1
	elif '2018' in tag: return ROOT.kAzure
	elif 'all' in tag: return ROOT.kRed
    return ROOT.kBlack

def formathisttitle(title):
    # get suitable legend entry out of histogram title (depends on naming convention in source file!)
    # simulation is the easy part since only one graph is expected
    if('mc_' in title): return 'simulation'
    # data case 1: single era (expect e.g. 'Run2016B' in title)
    if('Run201' in title): return 'data '+title[title.find('201'):title.find('201')+5]
    # data case 2: single year (expect e.g. 'Summer16' in title)
    if('Summer16' in title): return 'data 2016'
    if('Fall17' in title): return 'data 2017'
    if('Autumn18' in title): return 'data 2018'
    # data case 3: everything combined
    if('allyears' in title): return 'data (all years)'
    return '<unrecognized>'

def getweightedvalue(valuegraph,spectrumhist):
    # average out the per-bin values and uncertainties in valuegraph, 
    # using the spectrumhist as weighting factors.
    # make sure that the bins in spectrumhist correspond to the points in valuegraph!
    
    avgval = avgup = avgdown = 0
    sumweighting = 0
    for i in range(valuegraph.GetN()):
	# check that bin and point align
	if valuegraph.GetX()[i] != spectrumhist.GetBinCenter(i+1):
	    print('### ERROR ###: points in graph and bin centers in hist do not correspond')
	    return (0,0,0)
	# weight factor consists of two components: width of this datapoint and spectrum value
	weightfactor = spectrumhist.GetBinWidth(i+1)*spectrumhist.GetBinContent(i+1)
	sumweighting += weightfactor
	avgval += valuegraph.GetY()[i]*weightfactor
	avgup += valuegraph.GetErrorYhigh(i)*weightfactor
	avgdown += valuegraph.GetErrorYlow(i)*weightfactor
    avgval /= sumweighting
    avgup /= sumweighting
    avgdown /= sumweighting
    return (avgval,avgup,avgdown)

def drawExtraInfo(infos):
    # write extra info to current plot
    # infos is a list of dicts
    # minimal dict entries are 'text','posx' and 'posy'
    # additional optional entries: see below
    tinfo = ROOT.TLatex()
    for info in infos:
	keys = info.keys()
	if 'textcolor' in keys: 
	    tinfo.SetTextColor(info['textcolor'])
	if 'textsize' in keys: 
	    tinfo.SetTextSize(info['textsize'])
	if 'textfont' in keys: 
	    tinfo.SetTextFont(info['textfont'])
	tinfo.DrawLatexNDC(info['posx'],info['posy'],info['text'])

def plotefficiencies(datagraphlist,mcgraph,mchist=None,mcsysthist=None,
		    mode='random',yaxtitle='',xaxtitle='',lumi=0.,
		    outfile='plotefficiencies.png'):
    # note: datagraphlist and mcgraph contain efficiencies and are of type (list of) TGraphAsymmError
    #	    mchist is of type TH1D and contains the absolute numbers instead of efficiency
    #	    mcsyshist is of type TH1D and contains systematic variations to be applied to mcgraph
    #	    both of the above can be None
    
    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    rfrac = 0.25 # fraction of ratio plot in canvas
    # fonts and sizes:
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
    infofont = 5; infosize = 20
    # title offset
    ytitleoffset = 1.9
    xtitleoffset = 4.5
    # margins:
    p1topmargin = 0.07
    p2bottommargin = 0.4
    leftmargin = 0.15
    rightmargin = 0.05
    # legend box
    p1legendbox = [leftmargin+0.03,1-p1topmargin-0.25,1-rightmargin-0.03,1-p1topmargin-0.03]
    p2legendbox = [leftmargin+0.03,0.84,1-rightmargin-0.03,0.97]
    # marker properties for data
    markerstyle = 20
    markersize = 0.5
    # y axis range
    yaxmin = 0.
    yaxmax = 1.5
    # x axis range
    xaxmin = mcgraph.GetX()[0]-mcgraph.GetErrorXlow(0)
    xaxmax = mcgraph.GetX()[mcgraph.GetN()-1]+mcgraph.GetErrorXhigh(mcgraph.GetN()-1)

    ### set properties of absolute MC histogram showing the spectrum
    plotspectrum = False
    if mchist is not None:
	plotspectrum = True
	mchist.SetFillColor(ROOT.kGray)
	mchist.SetLineColor(ROOT.kBlack)
	mchist.SetLineWidth(1)
	# normalization and offset
	mchistmax = 0
	for i in range(1,mchist.GetNbinsX()+1):
	    if mchist.GetBinContent(i)>mchistmax: mchistmax=mchist.GetBinContent(i)
	scale = 0.5*(yaxmax-yaxmin)/mchistmax
	mchist.Scale(scale)
	for i in range(1,mchist.GetNbinsX()+1):
	    mchist.SetBinContent(i,mchist.GetBinContent(i)+yaxmin)
   
    ### calculate statistical and total mc error and set graph properties
    # note: mcstaterror is a copy of mcgraph before modifying it, containing statistical errors;
    #       mcerror starts as a copy of mcgraph but contains both statistical and systematic errors;
    #	    mcgraph gets is vertical errors set to zero (for plotting purposes)
    mcstaterror = mcgraph.Clone()
    mcerror = mcgraph.Clone()
    # if a systematics histogram is given, calculate total stat+sys error and modify mcerror
    if mcsysthist is not None:
        for i in range(0,mcgraph.GetN()):
	    staterrorup = mcgraph.GetErrorYhigh(i)
            staterrordown = mcgraph.GetErrorYlow(i)
            systerror = mcsyshist.GetBinContent(i+1)
	    toterrorup = np.sqrt(np.power(staterrorup,2)+np.power(systerror,2))
	    toterrordown = np.sqrt(np.power(staterrordown,2)+np.power(systerror,2))
	    toterrorup = min(1-mcgraph.GetY()[i],toterrorup)
	    toterrordown = min(mcgraph.GetY()[i],toterrordown)
	    mcerror.SetPointEYhigh(i,toterrorup)
	    mcerror.SetPointEYlow(i,toterrordown)
    # in any case, put vertical errors of mcgraph to zero
    for i in range(0,mcgraph.GetN()):
	mcgraph.SetPointEYhigh(i,0.)
	mcgraph.SetPointEYlow(i,0.)
    # set drawing properties of mcgraph and mcerror (mcstaterror is not drawn, maybe introduce later)
    mcgraph.SetLineColor(ROOT.kBlack)
    mcgraph.SetLineWidth(2)
    mcerror.SetFillStyle(3244)
    mcerror.SetLineWidth(0)
    mcerror.SetFillColor(ROOT.kGray+2)
    mcerror.SetMarkerStyle(0)

    ### calculate statistical and total mc error (scaled to unit bin content) 
    scstaterror = mcstaterror.Clone()
    scerror = mcerror.Clone()
    for i in range(0,mcgraph.GetN()):
        scstaterror.SetPoint(i,mcgraph.GetX()[i],1.)
        scerror.SetPoint(i,mcgraph.GetX()[i],1.)
        if not mcgraph.GetY()[i]==0:
            scstaterror.SetPointEYhigh(i,mcstaterror.GetErrorYhigh(i)/mcstaterror.GetY()[i])
	    scstaterror.SetPointEYlow(i,mcstaterror.GetErrorYlow(i)/mcstaterror.GetY()[i])
            scerror.SetPointEYhigh(i,mcerror.GetErrorYhigh(i)/mcerror.GetY()[i])
	    scerror.SetPointEYlow(i,mcerror.GetErrorYlow(i)/mcerror.GetY()[i])
        else:
            scstaterror.SetPointEYhigh(i,0.)
	    scstaterror.SetPointEYlow(i,0.)
            scerror.SetPointEYhigh(i,0.)
	    scerror.SetPointEYlow(i,0.)
    scstaterror.SetFillStyle(1001)
    scerror.SetFillStyle(1001)
    scstaterror.SetFillColor(ROOT.kCyan-4)
    scerror.SetFillColor(ROOT.kOrange-4)
    scstaterror.SetMarkerStyle(1)
    scerror.SetMarkerStyle(1)

    ### operations on data histogram
    for i,datagraph in enumerate(datagraphlist):
	color = getcolor(datagraph,mode)
	datagraph.SetMarkerStyle(markerstyle)
	datagraph.SetMarkerColor(color)
	datagraph.SetMarkerSize(markersize)
	datagraph.SetLineWidth(1)
	datagraph.SetLineColor(color)
	# highlight graph plotted in red
	if color==ROOT.kRed:
	    datagraph.SetLineWidth(2)
	    datagraph.SetMarkerSize(markersize*2)

    ### calculate data to mc ratio
    ratiographlist = []
    for datagraph in datagraphlist:
	ratiograph = datagraph.Clone()
	for i in range(0,mcgraph.GetN()):
	    if not mcgraph.GetY()[i]==0:
		ratiograph.GetY()[i] *= 1./mcgraph.GetY()[i]
		ratiograph.SetPointEYhigh(i,datagraph.GetErrorYhigh(i)/mcgraph.GetY()[i])
                ratiograph.SetPointEYlow(i,datagraph.GetErrorYlow(i)/mcgraph.GetY()[i])
	    # avoid drawing empty mc or data bins
	    else: ratiograph.GetY()[i] = 1e6
	    if(datagraph.GetY()[i]<=0): ratiograph.GetY()[i] += 1e6
	ratiographlist.append(ratiograph)

    ### get weighted efficiency values
    info = []
    if mchist is not None:
	mctup = getweightedvalue(mcerror,mchist)
	info.append( { 'text':'weighted sim eff:',
			'posx':0.65,'posy':0.5,
			'textsize':infosize*0.75,'textfont':infofont} )
	info.append( { 'text':'{:.4}'.format(mctup[0])
                        +'^{+'+'{:.4}'.format(mctup[1])+'}'
                        +'_{-'+'{:.4}'.format(mctup[2])+'}',
                        'posx':0.65,'posy':0.45,
                        'textsize':infosize,'textfont':infofont} )
	for datagraph in datagraphlist:
	    if getcolor(datagraph,mode)==ROOT.kRed:
		datatup = getweightedvalue(datagraph,mchist)
		info.append( { 'text':'weighted data eff:',
				'posx':0.65,'posy':0.35,
				'textsize':infosize*0.75,'textfont':infofont,
				'textcolor':ROOT.kRed} )
		info.append( { 'text':'{:.4}'.format(datatup[0])
                                +'^{+'+'{:.4}'.format(datatup[1])+'}'
                                +'_{-'+'{:.4}'.format(datatup[2])+'}',
                                'posx':0.65,'posy':0.3,
                                'textsize':infosize,'textfont':infofont,
                                'textcolor':ROOT.kRed} )
		
    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(p1legendbox[0],p1legendbox[1],p1legendbox[2],p1legendbox[3])
    legend.SetNColumns(2)
    legend.SetFillStyle(0)
    legend.AddEntry(mcgraph,formathisttitle(mcgraph.GetTitle()),"l")
    legend.AddEntry(mcerror,"total sim. unc.","f")
    for datagraph in datagraphlist:
	legend.AddEntry(datagraph,formathisttitle(datagraph.GetTitle()),"lpe")
    if plotspectrum: legend.AddEntry(mchist,"simulated spectrum")

    ### make legend for lower plot and add all histograms
    legend2 = ROOT.TLegend(p2legendbox[0],p2legendbox[1],p2legendbox[2],p2legendbox[3])
    legend2.SetNColumns(2); 
    legend2.SetFillStyle(0);
    legend2.AddEntry(scstaterror, "stat. pred. unc.", "f");
    legend2.AddEntry(scerror, "total pred. unc.", "f");
    #legend2.AddEntry(ratiograph, "obs./pred.", "pe12"); # do not add all ratio graphs, no space.

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,rfrac,1.,1.)
    pad1.SetTopMargin(p1topmargin)
    pad1.SetBottomMargin(0.03)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.Draw()
    pad2 = ROOT.TPad("pad2","",0.,0.,1.,rfrac)
    pad2.SetTopMargin(0.01)
    pad2.SetBottomMargin(p2bottommargin)
    pad2.SetLeftMargin(leftmargin)
    pad2.SetRightMargin(rightmargin)
    pad2.Draw()
    
    ### make upper part of the plot
    pad1.cd()
    #(rangemin,rangemax) = getminmax(datahistlist+[mchist])
    #mcerror.SetMinimum(rangemin)
    #mcerror.SetMaximum(rangemax)
    mcerror.SetMinimum(yaxmin)
    mcerror.SetMaximum(yaxmax)

    # X-axis layout
    xax = mcerror.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLimits(xaxmin,xaxmax)
    xax.SetLabelSize(0)
    # Y-axis layout
    yax = mcerror.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw mcerror first to get range correct
    mcerror.Draw("A 2") # option A to draw axes, option 2 to draw error as rectangular bands
    if plotspectrum: mchist.Draw("hist same")
    # draw data graphs
    # note: drawing two times to have perpendicular bars at vertical but not at horizontal error bars
    for datagraph in datagraphlist: 
	datagraph.Draw("Z P same") # option Z to suppress perpendicular bars
	for i in range(datagraph.GetN()): 
	    datagraph.SetPointEXhigh(i,0)
	    datagraph.SetPointEXlow(i,0)
	datagraph.Draw("P same")
    mcgraph.Draw("Z same") # option Z to suppress perpendicular bars
    legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # make and draw unit line
    line = ROOT.TLine(xaxmin,1,xaxmax,1)
    line.SetLineStyle(2)
    line.Draw("same")

    # draw extra info
    if len(info)>0:
	drawExtraInfo(info)

    # draw header
    lumistr = '{0:.1f}'.format(lumi/1000.)
    tools.drawLumi(pad1,lumitext=lumistr+" fb^{-1} (13 TeV)")

    ### make the lower part of the plot
    pad2.cd()
    # X-axis layout
    xax = scerror.GetXaxis()
    xax.SetLimits(xaxmin,xaxmax)
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = scerror.GetYaxis()
    #yax.SetLimits(0.9,1.0999)
    scerror.SetMinimum(0.95)
    scerror.SetMaximum(1.0499)
    yax.SetTitle("obs./pred.")
    yax.SetMaxDigits(3)
    yax.SetNdivisions(4,5,0)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    scerror.Draw("A 2")
    scstaterror.Draw("2 same")
    # draw ratio graphs
    # note: drawing two times to have perpendicular bars at vertical but not at horizontal error bars
    for ratiograph in ratiographlist:
        ratiograph.Draw("Z P same") # option Z to suppress perpendicular bars
        for i in range(ratiograph.GetN()):
            ratiograph.SetPointEXhigh(i,0)
            ratiograph.SetPointEXlow(i,0)
        ratiograph.Draw("P same")
    legend2.Draw("same")
    ROOT.gPad.RedrawAxis()

    # make and draw unit ratio line
    line2 = ROOT.TLine(xaxmin,1,xaxmax,1)
    line2.SetLineStyle(2)
    line2.Draw("same")
    
    ### save the plot
    c1.SaveAs(outfile)

if __name__=="__main__":
    
    ### search for histogram files to run on
    # note: the directory 'histdir' will be scanned for files matching the ones in 'histfilenames'.
    #	    plots will be made for all variable names in 'variables'
    histdir = os.path.abspath('../triggerefficiency/output')
    histfilenames = ['combined.root']
    variables = [
		  {'name':'leptonptleading','xaxtitle':r'lepton p_{T}^{leading} (GeV)'},
		  {'name':'leptonptsubleading','xaxtitle':r'lepton p_{T}^{subleading} (GeV)'},
		  {'name':'leptonpttrailing','xaxtitle':r'lepton p_{T}^{trailing} (GeV)'}
		]
    if not os.path.exists(histdir):
        print('### ERROR ###: requested to run on '+histdir+' but it does not seem to exist...')
        sys.exit()
    histfiles = []
    for root,dirs,files in os.walk(histdir):
        for fname in files:
            if fname in histfilenames:
                    histfiles.append(os.path.join(root,fname))
    print('effplotter.py will run on the following files:')
    print(histfiles)

    #histfiles = [f for f in histfiles if 'allyears' in f] # temp for testing 

    for histfile in histfiles:
	mode = 'triggerefficiency_singleyear'
	if 'allyears' in histfile: mode='triggerefficiency_allyears' 
	localhistdir = histfile[:histfile.rfind('/')]
	for variable in variables:
	    #try:
		mcgraphlist = loadobjects(histfile,mustcontain=['mc',variable['name']+'_eff'])
		datagraphlist = loadobjects(histfile,mustcontain=['data',variable['name']+'_eff'])
		if not len(mcgraphlist)==1:
		    print('### ERROR ###: list of MC histograms has unexpected length: '
			    +str(len(mcgraphlist)))
		    sys.exit()
		mcgraph = mcgraphlist[0]
		mchist = loadobjects(histfile,mustcontain=['mc',variable['name']+'_tot'])[0]
		# set plot properties
		yaxtitle = 'trigger efficiency'
		xaxtitle = variable['xaxtitle']
		lumi = 0.
		if '2016' in histfile: lumi = 35900.
		elif '2017' in histfile: lumi = 41500.
		elif '2018' in histfile: lumi = 59700.
		elif 'allyears' in histfile: lumi = 137100.
		figname = os.path.join(localhistdir,histfile.rstrip('.root')+'_'
						    +variable['name']+'.png')
		plotefficiencies(datagraphlist,mcgraph,mchist=mchist,mcsysthist=None,
				mode=mode,yaxtitle=yaxtitle,xaxtitle=xaxtitle,lumi=lumi,
				outfile=figname)
	    #except:
		#print('### WARNING ###: something went wrong for '+histfile+','+variable['name'])
