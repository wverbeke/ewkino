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
import plottools as pt
import histplotter_prefit as hpp
sys.path.append('../tools')
import histtools as ht
import listtools as lt

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
    hist.SetLineWidth(2)
    sysname = systematic
    if('Up' in systematic):
	hist.SetLineStyle(2)
	sysname = systematic.replace('Up','')
    elif('Down' in systematic):
	hist.SetLineStyle(0)
	sysname = systematic.replace('Down','')
    elif('ShapeVar' in systematic):
	sysname = systematic[:systematic.find('ShapeVar')+8]
    hist.SetLineColor( pt.getcolormap_systematics().get(sysname,ROOT.kBlack) )
    
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

def plotsystematics(mchistlist,variable,yaxtitle,xaxtitle,outfile,
		    relative=True,errorbars=False,yaxrange=None,
		    outtxtfile=""):
    
    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 600 # width of canvas
    # fonts and sizes:
    labelfont = 4; labelsize = 22
    axtitlefont = 4; axtitlesize = 22
    # title offset
    ytitleoffset = 2
    xtitleoffset = 1
    # margins:
    topmargin = 0.05
    bottommargin = 0.1
    leftmargin = 0.15
    rightmargin = 0.25
    # legend box
    plegendbox = [1-rightmargin+0.03,bottommargin+0.03,1-0.03,1-topmargin-0.03]

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
    ht.cliphistogram(nominalhist)
    for hist in mchistlist:
        sethiststyle(hist,variable)
	ht.cliphistogram(hist)
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
    nentries = 0
    allJECHasLabel = False
    groupedJECHasLabel = False
    for hist in mchistlist:
	label = hist.GetName().split(variable)[-1].strip('_')
	# avoid drawing a legend entry for all shape variations
	if('ShapeVar0' in label): label = label[:label.find('Var0')]
	elif('ShapeVar' in label): continue
	# avoid drawing a legend entry for all JEC variations
	if('JECAll' in label):
	    if not allJECHasLabel: 
		label = 'JECAll'
		allJECHasLabel = True
	    else: continue
	if('JECGrouped' in label):
	    if not groupedJECHasLabel:
		label = 'JECGrouped'
		groupedJECHasLabel = True
	    else: continue
	if label[-2:]=='Up': label = '~Up'
        legend.AddEntry(hist,label,"l")
	nentries += 1
    legend.AddEntry(nominalhist,nominalhist.GetTitle(),"l")
    legend.SetNColumns(1)

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetTopMargin(topmargin)
    pad1.SetBottomMargin(bottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetFrameLineWidth(2)
    pad1.SetGridx(1)
    pad1.SetGridy(1)
    pad1.Draw()
    
    ### make upper part of the plot
    pad1.cd()
    # determine plot range based on minimum and maximum variation
    (rangemin,rangemax) = getminmax(mchistlist)
    # if drawing error bars, also take statistical variation into account
    if errorbars:
	(srangemin,srangemax) = getminmax([nominalhist],witherrors=True)
    if not relative: rangemin = 0.
    if yaxrange is not None:
	rangemin = yaxrange[0]
	rangemax = yaxrange[1]
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
    xax.SetNdivisions(8,4,0,ROOT.kTRUE)
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
    yax.SetNdivisions(10,5,0,ROOT.kTRUE)

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
    pt.drawLumi(pad1,lumitext="simulation")

    ### save the plot
    c1.SaveAs(outfile+'.png')
    c1.SaveAs(outfile+'.pdf')
    ### save txt files with values if requested
    if len(outtxtfile)>0: histlisttotxt([nominalhist]+mchistlist,outtxtfile)

if __name__=="__main__":
    
    ####################################################################
    # set properties: variables, processes and systematics to consider #
    ####################################################################

    # define variables with axis titles, units, etc.
    variables = ([
        {'name':'_abs_eta_recoil','title':r'\left|\eta\right|_{recoil}','unit':''},
        {'name':'_Mjj_max','title':r'm_{jet+jet}^{max}','unit':'GeV'},
        {'name':'_lW_asymmetry','title':r'Asymmetry (lepton from W)','unit':''},
        {'name':'_deepCSV_max','title':r'Highest b tagging discriminant','unit':''},
        {'name':'_deepFlavor_max','title':r'Highest b tagging discriminant','unit':''},
        {'name':'_lT','title':'L_{T}','unit':'GeV'},
        {'name':'_MT','title':'M_{T}','unit':'GeV'},
        {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
        {'name':'_dRlb_min','title':r'\Delta R(\ell,bjet)_{min}','unit':''},
        {'name':'_dPhill_max','title':r'\Delta \Phi (\ell,\ell)_{max}','unit':''},
        {'name':'_HT','title':r'H_{T}','unit':'GeV'},
        {'name':'_nJets','title':r'Number of jets','unit':''},
        {'name':'_nBJets','title':r'Number of b jets','unit':''},
        {'name':'_dRlWrecoil','title':r'\Delta R(\ell_{W},jet_{recoil})','unit':''},
        {'name':'_dRlWbtagged','title':r'\Delta R(\ell_{W},jet_{b-tagged})','unit':''},
        {'name':'_M3l','title':r'm_{3\ell}','unit':'GeV'},
        {'name':'_fineBinnedM3l','title':r'm_{3\ell}','unit':'GeV'},
        {'name':'_abs_eta_max','title':r'\left|\eta\right|_{max}','unit':''},
        {'name':'_eventBDT','title':r'Event BDT output score','unit':''},
        {'name':'_nMuons','title':r'Number of muons in event','unit':''},
        {'name':'_nElectrons','title':r'Number of electrons in event','unit':''},
        {'name':'_yield','title':r'Total yield','unit':''},
        {'name':'_leptonMVATOP_min','title':r'Minimum TOP MVA value in event','unit':''},
        {'name':'_leptonMVAttH_min','title':r'Minimum ttH MVA value in event','unit':''},
        {'name':'_rebinnedeventBDT','title':'Event BDT output score','unit':''},
        {'name':'_leptonPtLeading','title':r'Leading lepton p_{T}','unit':'GeV'},
        {'name':'_leptonPtSubLeading','title':r'Subleading lepton p_{T}','unit':'GeV'},
        {'name':'_leptonPtTrailing','title':r'Trailing lepton p_{T}','unit':'GeV'},
        {'name':'_fineBinnedleptonPtTrailing','title':r'Trailing lepton p_{T}','unit':'GeV'},
        {'name':'_leptonEtaLeading','title':r'Leading lepton \eta','unit':''},
        {'name':'_leptonEtaSubLeading','title':r'Subleading lepton \eta','unit':''},
        {'name':'_leptonEtaTrailing','title':r'Trailing lepton \eta','unit':''},
        {'name':'_jetPtLeading','title':r'Leading jet p_{T}','unit':'GeV'},
        {'name':'_jetPtSubLeading','title':r'Subleading jet p_{T}','unit':'GeV'},
        {'name':'_bestZMass','title':r'Mass of OSSF pair','unit':'GeV'},
    ])
    variables = [{'name':'_eventBDT','title':r'Event BDT output score','unit':''}]
    #variables = [{'name':'_yield','title':r'Total yield','unit':''}]
    # (smaller set for testing )

    # processes (new convention):
    #simprocesses = (['tZq'])
    #simprocesses += (['WZ','multiboson','tX','ttZ','ZZH','Xgamma'])
    #simprocesses.append('nonprompt') # it is plotted as if simulated, even if taken from data
    simprocesses = (['WZ'])

    # processes (old convention):
    #simprocesses = (['tZq','WZ','multiboson','tbarttX','tbartZ','ZZH','Xgamma'])
    #simprocesses.append('nonprompt') # it is plotted as if simulated, even if taken from data

    # processes (split ZG)
    #simprocesses = (['tZq','WZ','multiboson','tX','ttZ','ZZH','Xgamma_int','Xgamma_ext'])
    #simprocesses.append('nonprompt') # it is plotted as if simulated, even if taken from data

    # get unsplit processes (e.g. Xgamma_int and Xgamma_ext -> Xgamma)
    # (needed since the process-specific uncertainties take the unsplit name)
    simprocesses_unsplit = {}
    for p in simprocesses:
        simprocesses_unsplit[p] = p.split('_')[0]

    # color and label maps
    #colormap = pt.getcolormap('tzqanalysis')
    #labelmap = pt.getlabelmap('tzqanalysis')

    # systematics to draw
    systematictags = ['JECAll'] # empty list for all systematics in file
    # systematics to exclude
    excludetags = [] # empty list to exclude nothing

    # save some examples of systematictags and excludetags for typical plots in AN
    # tags for PDF plot
    #systematictags = ['pdf']
    #excludetags = []
    #simprocesses = ['tZq'] # (plots were originally made for tZq sample only,
			   # not necessarily best choice, just only possibility then)
    # tags for QCD scales plot
    #systematictags = ['rScale','fScale','rfScales']
    #excludetags = []
    #simprocesses = ['tZq'] # (plots were originally made for tZq sample only,
                           # not necessarily best choice, just only possibility then)
    # tags for general plot
    #systematictags = []
    #excludetags = ['ShapeVar','rScale','fScale','rfScales','Norm']
    #simprocesses = ['tZq'] # (plots were originally made for tZq sample only,
                           # not necessarily best choice, just only possibility then)

    # save some examples of systematictags and excludetags for jec split studies
    # tags for grouped total
    #systematictags = ['JECGrouped_TotalUp','JECGrouped_TotalDown','JECUp','JECDown']
    #excludetags = []
    # tags for all total
    #systematictags = ['JECAll_TotalUp','JECAll_TotalDown','JECUp','JECDown']
    #excludetags = []
    # tags for squared sum grouped
    #systematictags = ['JEC']
    #excludetags = (['JECAll','Total','PileUpEnvelope','PileUpMuZero','FlavorZJet',
    #	'FlavorPhotonJet','FlavorPureGluon','FlavorPureQuark','FlavorPureCharm','FlavorPureBottom'])
    # tags for squared sum all
    #systematictags = ['JEC']
    #excludetags = (['JECGrouped','Total','PileUpEnvelope','PileUpMuZero','FlavorZJet',
    # 'FlavorPhotonJet','FlavorPureGluon','FlavorPureQuark','FlavorPureCharm','FlavorPureBottom'])

    ################################################################
    # run the plotting function on the configuration defined above #
    ################################################################

    # set output directory
    figdir = 'systplotter_output'
    if not os.path.exists(figdir): os.makedirs(figdir)

    # if 1 command line argument and it is a root file, run on this file
    if(len(sys.argv)==2 and sys.argv[1][-5:]=='.root'):
        histfile = os.path.abspath(sys.argv[1])
        if not os.path.exists(histfile):
            raise Exception('ERROR: requested to run on '+histfile
                            +' but it does not seem to exist...')
        histdir = histfile.rsplit('/',1)[0]
        lumi = None
        if '2016' in histfile: lumi = 35900
        elif '2017' in histfile: lumi = 41500
        elif '2018' in histfile: lumi = 59700
        elif '1617' in histfile: lumi = 77400
        elif 'allyears' in histfile: lumi = 137100
        elif 'yearscombined' in histfile: lumi = 137100

        # get all relevant histograms
	mustcontainone = []
        if len(systematictags)>0: mustcontainone = systematictags + ['nominal']
	mustcontainall = []
	if len(simprocesses)==1: mustcontainall.append(simprocesses[0])
	if len(variables)==1: mustcontainall.append(variables[0]['name'])
        histlist = ht.loadhistograms(histfile,mustcontainone=mustcontainone,
					       maynotcontainone=excludetags,
						mustcontainall=mustcontainall)
	histdict = {}
	for hist in histlist: 
	    print(hist.GetName())
	    histdict[hist.GetName()] = hist
	# get all processes and compare to arguments
	processlist = hpp.getprocesses(histdict,variables[0]['name'])
	simprocesses_copy = simprocesses[:]
	for p in simprocesses_copy:
	    if p not in processlist:
		print('WARNING: requested process {}'.format(p)
			+' not found, skipping it...')
		simprocesses.remove(p)
        # get all systematics that should be condsidered
        # note: assume they are the same for all variables...
        shapesyslist = hpp.getsystematics(histdict,'.+',variables[0]['name'])
	shapesyslist = lt.subselect_strings(shapesyslist,
					    mustcontainone=systematictags,
					    maynotcontainone=excludetags)[1]
	print('extracted following relevant systematics from histogram file:')
	for systematic in shapesyslist: print('  - '+systematic)
	# make a dict encoding which systematics are present for which processes
        shapesysdict = {}
        for p in simprocesses:
            shapesysdict[p] = []
            for systematic in shapesyslist:
		# special case: temporary additional disablings
		if(p=='nonprompt' and systematic in ['CR_GluonMove','CR_QCD']): continue
		# special case for ShapeVar systematics: no up and down
		if('ShapeVar' in systematic):
		    if( p+'_'+variables[0]['name']+'_'+systematic in histdict.keys() ):
			shapesysdict[p].append(systematic)
		# normal case: up and down variation must be present
		else:
		    if( p+'_'+variables[0]['name']+'_'+systematic+'Up' in histdict.keys()
			and p+'_'+variables[0]['name']+'_'+systematic+'Down' in histdict.keys() ):
			shapesysdict[p].append(systematic)
        # prints for testing
        print('found following shape systematics to be plotted:')
        for p in simprocesses:
            print(p)
            for systematic in shapesysdict[p]:
                print(' - '+systematic)

	# loop over variables
	for variable in variables:

	    variable = variable['name'] # possible to extend here for nicer x axis title
	    print('running on {}'.format(variable))

	    # get the histograms
	    histlist = []
	    for systematic in sorted(shapesyslist):
		issinglevar = False
		if 'ShapeVar' in systematic: issinglevar = True
		temp = hpp.get_single_systematic_histogram(histdict,simprocesses,
			variable,systematic,shapesysdict,issinglevar=issinglevar)
		if issinglevar:
		    histlist.append(temp)
		else:
		    histlist.append(temp['Down'])
		    histlist.append(temp['Up'])
	    histlist.append(hpp.get_single_systematic_histogram(histdict,simprocesses,
			    variable,'nominal',shapesysdict))
	    print('found following histograms:')
	    for hist in histlist: print(hist)
	    
	    figname = histfile.split('/')[-1].rstrip('.root')+'_var_'+variable 
	    figname = os.path.join(figdir,figname)

	    # re-order histograms to put individual pdf, qcd and jec variations in front
	    # (so they will be plotted in the background)
	    firsthistlist = []
	    secondhistlist = []
	    for hist in histlist:
		if('ShapeVar' in hist.GetName() 
		    or 'JECAll' in hist.GetName() 
		    or 'JECGrouped' in hist.GetName() ):
		    firsthistlist.append(hist)
		else: 
		    secondhistlist.append(hist)
	    histlist = firsthistlist + secondhistlist

	    # temporary: add root-sum-square of the individual JEC variations
	    # make sure to exclude the superfluous JEC variations in the selection above
	    # or the rss will be too large!
	    jecsum = False
	    if jecsum:
		nominalhist = histlist[findbyname( histlist, 'nominal' )]
		jecall = ht.selecthistograms(histlist,mustcontainall=['JECAll','Down'])[1]
		jecgrouped = ht.selecthistograms(histlist,mustcontainall=['JECGrouped','Down'])[1]
		for i,hist in enumerate(jecall):
		    downhist = histlist[findbyname(histlist,hist.GetName().replace('Down','Up'))]
		    jecall[i] = ht.binperbinmaxvar( [hist,downhist], nominalhist )
		    jecall[i].SetName( hist.GetName().replace('Down','Max') )
		for i,hist in enumerate(jecgrouped):
		    downhist = histlist[findbyname(histlist,hist.GetName().replace('Down','Up'))]
		    jecgrouped[i] = ht.binperbinmaxvar( [hist,downhist], nominalhist )
		    jecgrouped[i].SetName( hist.GetName().replace('Down','Max') )
		if( len(jecall)>0 ):
		    jecallup = nominalhist.Clone()
		    jecallup.Add( ht.rootsumsquare(jecall) )
		    jecallup.SetName( jecall[0].GetName()[0:jecall[0].GetName().find('JECAll')]
				    + 'JECSqSumAllUp' )
		    jecalldown = nominalhist.Clone()
		    jecalldown.Add( ht.rootsumsquare(jecall), -1 )
		    jecalldown.SetName( jecall[0].GetName()[0:jecall[0].GetName().find('JECAll')]
                                    + 'JECSqSumAllDown' )
		    histlist.append(jecallup)
		    histlist.append(jecalldown)
		if( len(jecgrouped)>0 ):
		    jecgroupedup = nominalhist.Clone()
		    jecgroupedup.Add( ht.rootsumsquare(jecgrouped) )
		    jecgroupedup.SetName( jecgrouped[0].GetName()[0:jecgrouped[0].GetName().find(
			'JECGrouped')] + 'JECSqSumGroupedUp' )
		    jecgroupeddown = nominalhist.Clone()
		    jecgroupeddown.Add( ht.rootsumsquare(jecgrouped), -1 )
		    jecgroupeddown.SetName( jecgrouped[0].GetName()[0:jecgrouped[0].GetName().find(
			'JECGrouped')] + 'JECSqSumGroupedDown' )
		    histlist.append(jecgroupedup)
		    histlist.append(jecgroupeddown)
	    
	    # set plot properties
	    binwidth = histlist[0].GetBinWidth(1)
	    yaxtitle = 'yield'
	    relyaxtitle = 'normalized yield'
	    xaxtitle = histlist[0].GetXaxis().GetTitle().strip('_')
	    plotsystematics(histlist,variable,yaxtitle,xaxtitle,figname+'_abs',relative=False,
				errorbars=True)
	    plotsystematics(histlist,variable,relyaxtitle,xaxtitle,figname+'_rel',relative=True,
				errorbars=True,
				#yaxrange=(0.9,1.1),
				outtxtfile=figname+'_tab')
