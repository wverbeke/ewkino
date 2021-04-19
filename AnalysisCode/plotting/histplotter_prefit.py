###################################################################################
# Extension of histplotter.py reading input histograms from a different structure #
# and also implementing the plotting of systematic variations			  #
###################################################################################
import ROOT
import sys
import numpy as np
import os
import json
import re
import plottools as pt
import histplotter as hp
sys.path.append(os.path.abspath('../tools'))
import histtools as ht

def loadallhistograms(histfile):
    ### read file and load all histograms for a given variable in a dictionary 
    ### note: overrides similar function in histttools.py! 
    ### this function has output dict instead of list

    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    histdict = {}
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        # check if histogram is readable
        try:
            nentries = hist.GetEntries()
	    nbins = hist.GetNbinsX()
            hist.SetDirectory(0)
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
        # add hist to dict
        histdict[hist.GetName()] = hist
    return histdict


def selecthistograms(histdict,processes,variables,variations,exact=False):
    ### select a subset of histograms from a dictionary read by loadallhistograms
    ### note: overrides similar function in histttools.py!
    # depends on naming convention: histograms are assumed to be named process_variable_variation

    selhistdict = {}
    for key in list(histdict.keys()):
	keep = False
	for process in processes:
	    for variable in variables:
		for variation in variations:
		    if exact:
			if( key==process+'_'+variable+'_'+variation ): 
			    keep=True
			    break
		    else:
			if re.match(process+'_'+variable+'_'+variation,key):
			    keep=True
			    break
	if keep: selhistdict[key] = histdict[key].Clone()
    return selhistdict


def getprocesses(histdict,variable):
    ### get a list of processes present in a dictionary read by loadallhistograms
    # note: need a variable to split the name in its parts;
    #       long-term plan: think of suitable separator in histogram names...
    plist = []
    for key in histdict.keys():
        if variable not in key: continue
	if 'nominal' not in key: continue
        parts = key.split(variable)
        thisprocess = parts[0].strip('_')
        if thisprocess not in plist: plist.append(thisprocess)
    return plist


def getsystematics(histdict,process,variable,removesystematics=[]):
    ### get a list of systematics present in a dictionary read by loadallhistograms
    # depends on naming convention: histograms are assumed to be named process_variable_variation
    # note: alternative to getallsystematics, which does not distinguish between processes
    # note: this function also removes some systematics based on their name;
    #       this used to be included in the input file creation but was remvoed there,
    #       so needs to be included here, else error band will be too large
    # note: need a variable to split the name in its parts;
    #       long-term plan: think of suitable separator in histogram names...
    slist = []
    for key in histdict.keys():
	if variable not in key: continue
	parts = key.split(variable)
	thisprocess = parts[0].strip('_')
	if not re.match(process,thisprocess): continue
	systematic = parts[1].strip('_')
	if(systematic[-2:]=='Up'): systematic = systematic[:-2]
        elif(systematic[-4:]=='Down'): systematic = systematic[:-4]
        if systematic == 'nominal': continue
	if systematic in slist: continue
	keep = True
	for rmtag in removesystematics:
	    if '.+' in rmtag:
                if re.match(rmtag,systematic):
                    keep = False
                    break
            else:
                if rmtag==systematic:
                    keep = False
                    break
	print(systematic+' -> '+str(keep))
	if not keep: continue
	slist.append(systematic)
    return sorted(slist)
    

def get_single_systematic_histogram(histdict,processes,variable,systematic,shapesysdict,
					issinglevar=False):
    ### exctract up and down histograms for a single uncertainty source
    # input arguments: see also getsystematichistogram
    # - issinglevar: the name of the systematic will not be appended with 'Up' and 'Down',
    #   and a single histogram is returned instead of a dict;
    #   automatically set to True if systematic is 'nominal'
    # output: dict of histograms {'Up':uphist,'Down':downhist}

    suffixes = ['Up','Down']
    if systematic=='nominal':
	issinglevar = True
    if issinglevar:
	suffixes = ['']
  
    res = {}
    for suffix in suffixes:
        histlist = []
        # loop over processes
        for p in processes:
            # if this sys should be considered for this process, find it
            if systematic in shapesysdict[p]:
                hists = selecthistograms(histdict,[p],[variable],[systematic+suffix],exact=True)
                if len(hists)==1: histlist.append(hists[hists.keys()[0]])
                else:
                    raise Exception('ERROR in getsystematichistogram:'
                                    +' {}/{}/{}'.format(p,variable,systematic+suffix)
                                    +' was requested but found'
                                    +' {} corresponding hists...'.format(len(hists)))
            # else use nominal
            else:
                hists = selecthistograms(histdict,[p],[variable],['nominal'],exact=True)
                if len(hists)==1: histlist.append(hists[hists.keys()[0]])
                else:
                    raise Exception('ERROR in getsystematichistogram:'
                                    +' {}/{}/{}'.format(p,variable,'nominal')
                                    +' was requested but found'
                                    +' {} corresponding hists...'.format(len(hists)))
        # sum the histograms for all processes for this variation and add it to the dict
        firsthist = histlist[0].Clone()
        for hist in histlist[1:]: firsthist.Add(hist)
        res[suffix] = firsthist
    # return the result
    if issinglevar: return res[res.keys()[0]]
    return res


def getsystematichistogram(histdict,processes,variable,shapesyslist,shapesysdict,
			    normuncertainties={}):
    ### extract a histogram containing combined systematic uncertainty
    # arguments:
    # - histdict is a dictionary of histograms mapping name to histogram
    # - proceses is a list of processes
    # - variable is the variable to consider
    # - shapesyslist is a list of all shape systematics to consider
    #   note: it also depends on shapesysdict what systematics are actually included
    # - shapesysdict is a dict of process names to lists of shape systematics to consider
    # - normuncertainties is a dict of process names to normalization uncertainties,
    #	e.g. {WZ:0.1} will add 10% of nominal as additional uncertainty to the plot
    #   note: need to extend this to other 'norm' uncertainties such as trigger/lumi/etc!
    #         they are currently not included in the plots (although the diff would be small)

    # for each shape systematic, sum up and down variations for all processes
    # (or nominal for processes for which this systematic should not be considered)
    # to get the total up and total down variation for that systematic
    totalvar = {}

    # loop over shape systematics and up/down variations
    for systematic in shapesyslist:
	temp = get_single_systematic_histogram(histdict,processes,variable,systematic,shapesysdict)
	totalvar[systematic+'Up'] = temp['Up']
	totalvar[systematic+'Down'] = temp['Down']

    # add normalization uncertaities
    normsyslist = []
    for normp,normunc in normuncertainties.items():
	histlistup = []
	histlistdown = []
	for p in processes:
	    hists = selecthistograms(histdict,[p],[variable],['nominal'],exact=True)
	    if len(hists)==1:
		if normp!=p:
		    histlistup.append(hists[hists.keys()[0]])
		    histlistdown.append(hists[hists.keys()[0]])
		else:
		    histup = hists[hists.keys()[0]].Clone(); histup.Scale(1+normunc)
		    histlistup.append(histup)
		    histdown = hists[hists.keys()[0]].Clone(); histdown.Scale(1-normunc)
		    histlistdown.append(histdown)
	    else:
		raise Exception('ERROR in getsystematichistogram:'
                                        +' {}/{}/{}'.format(p,variable,'nominal')
                                        +' was requested but found'
                                        +' {} corresponding hists...'.format(len(hists)))
	firsthistup = histlistup[0].Clone()
	for hist in histlistup[1:]: 
	    firsthistup.Add(hist)
	totalvar[normp+'_normUp'] = firsthistup
	firsthistdown = histlistdown[0].Clone()
        for hist in histlistdown[1:]: 
	    firsthistdown.Add(hist)
        totalvar[normp+'_normDown'] = firsthistdown
	normsyslist.append(normp+'_norm')

    # now get the nominal sum of processes
    nominalhist = get_single_systematic_histogram(histdict,processes,variable,'nominal',shapesysdict)

    # for each systematic, get the bin-per-bin max of (abs(up-nominal),abs(down-nominal))
    maxhistlist = []
    for systematic in shapesyslist+normsyslist:
	uphist = totalvar[systematic+'Up']
	downhist = totalvar[systematic+'Down']
	maxhist = ht.binperbinmaxvar( [uphist,downhist], nominalhist )
	totalvar[systematic+'Max'] = maxhist
	maxhistlist.append(maxhist)
    
    # add resulting systematic histograms in quadrature
    syshist = ht.rootsumsquare(maxhistlist)
    return syshist


def histplotter_prefit(figname,histdict,variables,simprocesses,dataprocesses,histdir,
			shapesyslist=[],shapesysdict={},
			varstoblind=None,
			labelmap=None,colormap=None,lumi=None,
			normuncertainties={}):
    ### main function, called by __main__ either directly or in job submission
    # arguments:
    # - figname: path to the figure to make
    # - histdict is a dict mapping histogram names to histograms
    #   (assumed to be named process_variable_variation)
    # - variables is a list of dicts containing the entries 'name', 'title' and 'unit'
    # - simprocesses is a list of tags identifying the simulated processes to use
    # - dataprocesses is a list of tags identifying the data processes to use
    # - histdir is the directory where to put the figures
    # - shapesyslist is a list of shape systematics to consider (default: none)
    # - shapesysdict is a dict mapping process names to systematics to include for this process
    #   note: for each process must be subset of shapesyslist, else they will be ignored
    # - varstoblind is a list of tags identifying the variables for which data is ignored
    # - legendmap is a dict mapping process names to legend entries, e.g. ttZ to t#bar{t}Z
    # - colormap is a dict mapping process names to root colors
    # - lumi is the luminosity value to display (float, in pb-1)
    # - normuncertainties is a dict of process names to normalization uncertainties,
    #   e.g. {WZ:0.1} will add 10% of nominal as additional uncertainty to the WZ contribution

    # loop over input variables
    for k,vardict in enumerate(variables):
        varname = str(vardict['name'])
	print('running on variable: '+varname)
	
        # select histograms for this variable
        allhists = selecthistograms(histdict,['.+'],[varname],['.+'])
	# if no histograms are found, skip to next variable without crashing
	if len(allhists)==0: 
	    print('### WARNING ###: no histograms were found for '+varname)    
	    continue

        # load nominal histograms
	simhistlist = []
	for p in simprocesses:
	    selhists = selecthistograms(allhists,[p],[varname],['nominal'])
	    if len(selhists)==1:
		simhistlist.append(selhists[selhists.keys()[0]])
	    else:
		raise Exception('ERROR in histplotter_prefit: a requested process is not'
				+' present in the provided histogram collection: {}'.format(p))
        datahists = selecthistograms(allhists,dataprocesses,[varname],['nominal'])
        if not len(datahists)==1:
	    # (maybe later allow multiple data entries but so far only one at a time is supported)
            raise Exception('ERROR in histplotter_prefit: list of data histograms'
			    +' has unexpected length: {}'.format(len(datahists)))
        datahist = datahists[datahists.keys()[0]]
        
	# if running blindly, reset the data histogram
	if varname in varstoblind:
	    datahist.Reset()

        # get total systematic variation
        syshist = getsystematichistogram(allhists,simprocesses,varname,
					    shapesyslist,shapesysdict,
					    normuncertainties=normuncertainties)
	#ht.printhistogram(syshist)
	ht.cliphistogram(syshist,clipboundary=1e-5)

        # set plot properties
        binwidth = datahist.GetBinWidth(1)
        #if binwidth.is_integer():
        #    yaxtitle = 'events / '+str(int(binwidth))+' '+vardict['unit']
        #else:
        #    yaxtitle = 'events / {0:.2f}'.format(binwidth)+' '+vardict['unit']
	yaxtitle = 'Number of events' # use simple title instead of mentioning bin width
        xaxtitle = vardict['title']
        if not vardict['unit']=='':
            xaxtitle += '['+vardict['unit']+']'
	thisfigname = os.path.splitext(figname)[0]+'_var_'+varname
	figdir = os.path.dirname(figname)
	if not os.path.exists(figdir):
	    os.makedirs(figdir)

        hp.plotdatavsmc(thisfigname+'_lin',datahist,simhistlist,syshist,signals=['tZq'],
			colormap=colormap,labelmap=labelmap,
			xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,yaxlog=False)
        hp.plotdatavsmc(thisfigname+'_log',datahist,simhistlist,syshist,signals=['tZq'],
			colormap=colormap,labelmap=labelmap,
			xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,yaxlog=True)

if __name__=="__main__":
    # read a root file containing histograms and make the plots
    # for all variables in an input dict

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
        {'name':'_MT','title':'Transverse W mass','unit':'GeV'},
        {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
        {'name':'_dRlb_min','title':r'\Delta R(l,bjet)_{min}','unit':''},
        {'name':'_dPhill_max','title':r'\Delta \Phi (l,l)_{max}','unit':''},
        {'name':'_HT','title':r'H_{T}','unit':'GeV'},
        {'name':'_nJets','title':r'Number of jets','unit':''},
	{'name':'_nBJets','title':r'Number of b jets','unit':''},
        {'name':'_dRlWrecoil','title':r'\Delta R(l_{W},jet_{recoil})','unit':''},
        {'name':'_dRlWbtagged','title':r'\Delta R(l_{W},jet_{b-tagged})','unit':''},
        {'name':'_M3l','title':r'm_{3l}','unit':'GeV'},
	{'name':'_fineBinnedM3l','title':r'm_{3l}','unit':'GeV'},
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
    #variables = [{'name':'_M3l','title':'test','unit':''}]
    # (smaller set for testing )
    
    # processes (new convention):
    simprocesses = (['tZq','WZ','multiboson','tX','ttZ','ZZH','Xgamma'])
    simprocesses.append('nonprompt') # it is plotted as if simulated, even if taken from data
    dataprocesses = ['data']
    normuncertainties = {'WZ':0.1,'ZZH':0.1,'ttZ':0.15,'Xgamma':0.1,'nonprompt':0.3}

    # processes (old convention):
    #simprocesses = (['tZq','WZ','multiboson','tbarttX','tbartZ','ZZH','Xgamma'])
    #simprocesses.append('nonprompt') # it is plotted as if simulated, even if taken from data
    #dataprocesses = ['data']
    #normuncertainties = {'WZ':0.1,'ZZH':0.1,'tbartZ':0.15,'Xgamma':0.1,'nonprompt':0.3}

    # processes (split ZG)
    #simprocesses = (['tZq','WZ','multiboson','tX','ttZ','ZZH','Xgamma_int','Xgamma_ext'])
    #simprocesses.append('nonprompt') # it is plotted as if simulated, even if taken from data
    #dataprocesses = ['data']
    #normuncertainties = ({'WZ':0.1,'ZZH':0.1,'ttZ':0.15,'Xgamma_int':0.1,'Xgamma_ext':0.1,
    #			    'nonprompt':0.3})

    # get unsplit processes (e.g. Xgamma_int and Xgamma_ext -> Xgamma)
    # (needed since the process-specific uncertainties take the unsplit name)
    simprocesses_unsplit = {}
    for p in simprocesses:
	simprocesses_unsplit[p] = p.split('_')[0]

    # color and label maps
    colormap = pt.getcolormap('tzqanalysis')
    labelmap = pt.getlabelmap('tzqanalysis')

    # blind the event BDT variables
    varstoblind = []
    #varstoblind = [var['name'] for var in variables if 'eventBDT' in var['name']]

    # define shape systematics to be plotted
    # (default: all in file, define here which ones should be removed)
    rmall = [] # list of systematics to remove for all processes

    # choose between separate renormalization and factorization scale variations,
    # correlated variations, or their envelope
    rmall.append('rScale.+')
    rmall.append('fScale.+')
    rmall.append('rfScales.+')
    #rmall.append('qcdScalesShapeEnv.+')

    # choose between pdf envelope or rms
    rmall.append('pdfShapeEnv')
    #rmall.append('_pdfShapeRMS')

    # choose between total JEC, grouped sources or individual sources
    rmall.append('JEC')
    rmall.append('JECGrouped.+')
    rmall.append('JECGrouped_Total')
    #rmall.append('JECAll.+')

    # remove individual variations (if not done so before)
    rmall.append('qcdScalesShapeVar.+')
    rmall.append('pdfShapeVar.+')

    rmdict = {} # dict of systematics to remove for specific processes
    for p in simprocesses: rmdict[p] = []
    # remove norm uncertainties
    for p in normuncertainties.keys()+['tZq']:
	for syst in ['rScale','fScale','rfScales','qcdScales','isr','fsr']:
	    rmdict[p].append(syst+'Norm_'+simprocesses_unsplit[p])
	rmdict[p].append('pdfNorm')
	rmdict[p].append('qcdScalesNorm') # should be split normally
    # remove process-specific ones for other processes (maybe do automatically later?)
    for p in simprocesses:
	for p2 in simprocesses:
	    if simprocesses_unsplit[p]!=simprocesses_unsplit[p2]: 
		rmdict[p].append('.+_'+simprocesses_unsplit[p2])
    # remove color reconnection and underlying event for all but tZq and ttZ
    for p in simprocesses:
	if not (p=='tZq' or p=='ttZ' or p=='tbartZ'):
	    rmdict[p].append('CR_GluonMove')
	    rmdict[p].append('CR_QCD')
	    rmdict[p].append('UE')
    # remove some for nonprompt
    rmdict['nonprompt'].append('JECAll.+')
    # some temporary removals, see uncertaintytools for more info!
    rmdict['tX'].append('pileup')
    rmdict['nonprompt'].append('pileup')


    ################################################################
    # run the plotting function on the configuration defined above #
    ################################################################

    # two command line arguments: path to input root file and path to output file
    # (possible extension will be ignored/overwritten)
    # note: the output filename will be modifed by appending _var_<varname> and _lin/_log!
    if(len(sys.argv)==3 and sys.argv[1][-5:]=='.root'):
	histfile = os.path.abspath(sys.argv[1])
	outfile = os.path.abspath(sys.argv[2])
	if not os.path.exists(histfile):
	    raise Exception('ERROR: requested to run on '+histfile
			    +' but it does not seem to exist...')
	histdir = os.path.dirname(histfile)
	outdir = os.path.dirname(outfile)
	if not os.path.exists(outdir):
	    os.makedirs(outdir)
	lumi = None
	if '2016' in histfile: lumi = 35900
	elif '2017' in histfile: lumi = 41500
	elif '2018' in histfile: lumi = 59700
	elif '1617' in histfile: lumi = 77400
	elif 'allyears' in histfile: lumi = 137100
	elif 'yearscombined' in histfile: lumi = 137100
	if not 'signalregion' in histfile: varstoblind = []

	# get all histograms
	histdict = loadallhistograms(histfile)
	# get all systematics that should be condsidered
	# note: assume they are the same for all variables...
	shapesyslist = getsystematics(histdict,'.+',variables[0]['name'],
					removesystematics=rmall)
	shapesysdict = {}
	for p in simprocesses: 
	    shapesysdict[p] = []
	    for sys in shapesyslist:
		keep = True
		for ignoresys in rmdict[p]:
		    if re.match(ignoresys,sys): 
			keep = False
			break
		if keep: shapesysdict[p].append(sys)
	# prints for testing
	print('found following shape systematics to be applied:')
	for p in simprocesses:
	    print(p)
	    for sys in shapesysdict[p]:
		print(' - '+sys)
	# call the plot functionality
	histplotter_prefit(outfile,histdict,variables,simprocesses,dataprocesses,histdir,
			    shapesyslist=shapesyslist,shapesysdict=shapesysdict,
			    varstoblind=varstoblind,
			    lumi=lumi,colormap=colormap,labelmap=labelmap,
			    normuncertainties=normuncertainties)
    
    else:
	print('### ERROR ###: unrecognized command line arguments.')
	sys.exit()
