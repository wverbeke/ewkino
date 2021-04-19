#####################################################################
# script to make postfit plots using CombineHarvester functionality #
#####################################################################
# based on this: http://cms-analysis.github.io/CombineHarvester/post-fit-shapes-ws.html

import sys
import os
import json
import numpy as np
import ROOT
import histplotter as hp
import plottools as pt
sys.path.append(os.path.abspath('../tools'))
import histtools as ht
sys.path.append(os.path.abspath('../newcombine'))
import datacardtools as dct
import uncertaintytools as ut
import combinetools as ct
sys.path.append('../../jobSubmission')
import condorTools as condortools
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

def readPostFitShapesFromWorkspace( histfile ):
    ### read an output file from the PostFitShapesFromWorkspace command
    # input:
    # - path to the file to be read
    # output:
    # - a dict of dicts mapping directories to histogram names to histograms

    res = {}
    f = ROOT.TFile.Open(histfile)
    gdir = ROOT.gDirectory
    channelkeys = gdir.GetListOfKeys()
    for chkey in channelkeys:
	f.cd()
	f.cd(chkey.GetName())
	thisdir = ROOT.gDirectory
	thisdict = {}
	histkeys = thisdir.GetListOfKeys()
	for key in histkeys:
	    hist = f.Get(chkey.GetName()+'/'+key.GetName())
	    try:
		hist.SetDirectory(0)
	    except:
		print('WARNING in loadhistograms: could not call SetDirectory(0)'
                        +' on object {}, skipping it...'.format(hist.GetName()))
		continue
	    # add hist to dict
	    thisdict[hist.GetName()] = hist
	# add dict to total dict
	res[chkey.GetName()] = thisdict
    return res

def runPostFitShapesFromWorkspace( workspace,datacard,mode,fitresultfile=None ):
    
    if not (mode=='postfit' or mode=='prefit'):
	raise Exception('ERROR: unrecognized mode: {}'.format(mode))
    tempfile = workspace.replace('.root','')+'_temp.root'
    pfcmd = 'PostFitShapesFromWorkspace'
    pfcmd += ' -d '+datacard
    pfcmd += ' -w '+workspace
    pfcmd += ' -o '+tempfile
    pfcmd += ' -m 125'
    if mode=='postfit':
        pfcmd += ' -f '+fitresultfile+':fit_s'
        pfcmd += ' --postfit'
        pfcmd += ' --sampling'
    pfcmd += ' --print'
    pfcmd += ' --total-shapes'

    os.system(pfcmd)
    print('running following command: {}'.format(pfcmd))

    histdict = readPostFitShapesFromWorkspace( tempfile )
    # - the histdict contains an entry 'postfit' (if --total-shapes was added)
    # with all processes for all channels summed + correctly evaluated uncertainty;
    # use this only for total uncertainty (not nominal) as it is not split per process.
    # - the histdict contains entries <channelname>_postfit
    # with the different processes for that channel; 
    # use only for nominal, not total uncertainty.
    # also similar entries for prefit

    return histdict

def makeworkspace(channels,signals,variables,outputdir,workspacename,
		    dostatonly=False):
    ### make a combined workspace as input for PostFitShapesFromWorkspace
    # input arguments:
    # - channels = a list of dicts encoding the channels to combine in the workspace
    #   note: each dict must have the keys 'path' (to a root file), 'name', 'year', 'region'
    #         and 'npfromdata'
    # - variables = list of variable names for which to make a plot
    # - outputdir = directory where to put the output
    # - workspacename = name for the workspace to be created (should not contain .root suffix)
    # - dostatonly = boolean whether to make an additional datacard,
    #                discarding all systematic variations (still experimental)

    thisoutputdir = os.path.join(outputdir,workspacename)
    if not os.path.exists(thisoutputdir):
	os.makedirs(thisoutputdir)
    statonlysuffix = '_statonly'

    # define the variable for which to run the full procedure
    # (other variables: see below)
    variable = variables[0]

    # write the elementary datacards for all channels
    for channel in channels:
        # get region, year and file to use
        name = channel['name']
        region = channel['region']
        year = channel['year']
        path = channel['path']
	npfromdata = channel['npfromdata']

        # check if histogram file exists
        if not os.path.exists(path):
            print('WARNING in makeworkspace: '+path+' not found, skipping it.')
            continue
        print('making datacard for '+path+'...')

        # get necessary info (see e.g inclusive_makedatacards for more info)
        processinfo = dct.readhistfile(path,variable,signals)
	ut.disable_default( processinfo, year, region, npfromdata )
        normsyslist = ut.addnormsys_default( processinfo, year, region, npfromdata )
        rateparamlist = []
        shapesyslist = []
        for sys in processinfo.slist:
            if sys not in normsyslist: shapesyslist.append(sys)
        shapesyslist = ut.subselect_systematics(shapesyslist)
        print('will take into account the following shape systematics:')
        for sys in shapesyslist: print('  - '+sys)
        print('will take into account the following norm systematics:')
        for sys in normsyslist: print('  - '+sys)

        # write the datacard
        dct.writedatacard(thisoutputdir,name,processinfo,path,variable,
                    shapesyslist=shapesyslist,lnNsyslist=normsyslist,
                    rateparamlist=rateparamlist,
		    writeobs=False,
                    autof=True)
	# rename the datacard taking into account the specific variable
	oldname = os.path.join(thisoutputdir,'datacard_'+name+'.txt')
	newname = oldname.replace('.txt','_var_'+variable+'.txt')
	os.system('mv {} {}'.format(oldname,newname))

	# do the same thing for stat only
	dct.writedatacard(thisoutputdir,name,processinfo,path,variable,
                    shapesyslist=[],lnNsyslist=[],
                    rateparamlist=rateparamlist,
                    writeobs=False,
                    autof=True)
        newname = oldname.replace('.txt','_var_'+variable+statonlysuffix+'.txt')
        os.system('mv {} {}'.format(oldname,newname))

    # create identical datacards with only the name of the variable changed
    for var in variables[1:]:
	for channel in channels:
	    statonlysuffixes = ['']
	    if dostatonly: statonlysuffixes = ['',statonlysuffix]
	    for statsuffix in statonlysuffixes:
		name = channel['name']
		oldname = os.path.join(thisoutputdir,'datacard_'+name+'_var_'+variable
				    +statsuffix+'.txt')
		newname = oldname.replace(variable,var)
		f = open(oldname,'r')
		fdata = f.read()
		f.close()
		fdata = fdata.replace(variable,var)
		f = open(newname,'w')
		f.write(fdata)
		f.close()

    # for each variable, create a combined workspace
    combinationdict = {}
    for var in variables:
	statonlysuffixes = ['']
        if dostatonly: statonlysuffixes = ['',statonlysuffix]
        for statsuffix in statonlysuffixes:
	    combinationname = 'dc_combined_'+workspacename+'_var_'+var+statsuffix+'.txt'
	    combinationdict[combinationname] = {}
	    for c in channels:
		name = c['name']
		combinationdict[combinationname]['datacard_'+name+'_var_'+var+statsuffix+'.txt'] = name
    ct.makecombinedcards( thisoutputdir, combinationdict )
    res = []
    for wsname in combinationdict.keys():
	onelinecmd = ''
	for c in ct.get_workspace_commands( thisoutputdir, wsname ):
	    onelinecmd += c+'; '
	os.system(onelinecmd)
	if not statonlysuffix in wsname:
	    res.append( os.path.join(thisoutputdir,wsname.replace('.txt','.root')) )
    return res

def histplotter_postfit( simtoterrorhist, simstaterrorhist, simhistlist, datahist, figname, 
			 xaxtitle=None, yaxtitle=None, lumi=None, extrainfos=[] ):
    ### wrapper for plotdatavsmc
   
    # color and label maps
    colormap = pt.getcolormap('tzqanalysis')
    labelmap = pt.getlabelmap('tzqanalysis')

    # define signals
    signals = [hist.GetName() for hist in simhistlist if 'tZq' in hist.GetName()]
 
    # make a histogram for total variation
    dostat = False
    for i in range(0,simtoterrorhist.GetNbinsX()+2):
        toterror = simtoterrorhist.GetBinError(i)
        simtoterrorhist.SetBinContent(i,toterror)

    # do the same for statistical variation
    if simstaterrorhist is not None:
	dostat = True
	for i in range(0,simstaterrorhist.GetNbinsX()+2):
	    staterror = simstaterrorhist.GetBinError(i)
	    simstaterrorhist.SetBinContent(i,staterror)
	    # modify simtoterrorhist to be systematic only instead of total variation
	    toterror = simtoterrorhist.GetBinContent(i)
	    if staterror>toterror:
		toterror = staterror
	    systerror = np.sqrt(toterror**2-staterror**2)
	    simtoterrorhist.SetBinContent(i,systerror)

    # set histogram titles (for plot legend)
    for hist in simhistlist:
        hist.SetTitle(hist.GetName())
    datahist.SetTitle('data')

    # set plot properties
    binwidth = datahist.GetBinWidth(1)
    if yaxtitle is None: yaxtitle = 'Number of events'
    if xaxtitle is None: xaxtitle = 'bin number'

    hp.plotdatavsmc(figname+'_lin',datahist,simhistlist,mcsysthist=simtoterrorhist,
		    mcstathist=simstaterrorhist,
		    signals=signals,
		    dostat=dostat,
                    colormap=colormap,labelmap=labelmap,
                    xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,yaxlog=False,
		    extrainfos=extrainfos)
    hp.plotdatavsmc(figname+'_log',datahist,simhistlist,mcsysthist=simtoterrorhist,
		    mcstathist=simstaterrorhist,
		    signals=signals,
		    dostat=dostat,
                    colormap=colormap,labelmap=labelmap,
                    xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,yaxlog=True,
		    extrainfos=extrainfos)

def get_variable_list( istest=False ):
    ### get a dict of variables
    
    if istest: 
	variables = ([
	    {'name':'_M3l','title':r'm_{3l}','unit':'GeV'}
	])
	return variables

    variables = ([
        {'name':'_abs_eta_recoil','title':r'\left|\eta\right|_{recoil}','unit':''},
        {'name':'_Mjj_max','title':r'm_{jet+jet}^{max}','unit':'GeV'},
        {'name':'_lW_asymmetry','title':r'Asymmetry (lepton from W)','unit':''},
        {'name':'_deepCSV_max','title':r'Highest b tagging discriminant','unit':''},
        {'name':'_deepFlavor_max','title':r'Highest b tagging discriminant','unit':''},
        {'name':'_lT','title':'L_{T}','unit':'GeV'},
	{'name':'_MT','title':'Transverse W boson mass','unit':'GeV'},
	{'name':'_coarseBinnedMT','title':'Transverse W boson mass','unit':'GeV'},
        {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
        {'name':'_dRlb_min','title':r'\Delta R(l,bjet)_{min}','unit':''},
        {'name':'_dPhill_max','title':r'\Delta \Phi (l,l)_{max}','unit':''},
        {'name':'_HT','title':r'H_{T}','unit':'GeV'},
        {'name':'_nJets','title':r'Number of jets','unit':''},
        {'name':'_nBJets','title':r'Number of b-tagged jets','unit':''},
        {'name':'_dRlWrecoil','title':r'\Delta R(l_{W},jet_{recoil})','unit':''},
        {'name':'_dRlWbtagged','title':r'\Delta R(l_{W},jet_{b-tagged})','unit':''},
        {'name':'_M3l','title':r'm_{3l}','unit':'GeV'},
        {'name':'_fineBinnedM3l','title':r'm_{3l}','unit':'GeV'},
        {'name':'_abs_eta_max','title':r'\left|\eta\right|_{max}','unit':''},
        {'name':'_eventBDT','title':r'Event BDT output score','unit':''},
        {'name':'_nMuons','title':r'Number of muons in event','unit':''},
        {'name':'_nElectrons','title':r'Number of electrons in event','unit':''},
        {'name':'_yield','title':r'Total yield','unit':''},
        #{'name':'_leptonMVATOP_min','title':r'Minimum TOP MVA value in event','unit':''},
        #{'name':'_leptonMVAttH_min','title':r'Minimum ttH MVA value in event','unit':''},
        #{'name':'_rebinnedeventBDT','title':'Event BDT output score','unit':''},
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
	{'name':'_Z_pt','title':'Z boson p_{T}','unit':'GeV'},
	{'name':'_coarseBinnedZ_pt','title':'Z boson p_{T}','unit':'GeV'},
	{'name':'_lW_pt','title':'Lepton from W boson p_{T}','unit':'GeV'},
	{'name':'_coarseBinnedlW_pt','title':'Lepton from W boson p_{T}','unit':'GeV'}
    ])
    return variables

def doblind( variable, region ):
    ### check whether a plot must be blinded
    if( 'eventBDT' in variable and 'signalregion' in region ): return True
    return False

def isdict( dstr ):
    ### small help function to check if a str encodes a dict
    try:
	d = json.loads(dstr)
	return True
    except:
	return False

def cleanplotdir( plotdir ):
    ### remove all txt and root files from a directory
    os.system('rm {}/*.root'.format(plotdir))
    os.system('rm {}/*.txt'.format(plotdir))


if __name__=='__main__':

    sys.stderr.write('###starting###\n')

    ### parse input arguments
    # nonpositional arguments
    doclean = True
    dotestvars = False
    runlocal = False
    runqsub = False
    unblind = False
    args = sys.argv[1:]
    argscopy = args[:]
    nonposargs = []
    for arg in argscopy:
	if arg=='noclean': 
	    doclean = False
	    print('found option noclean')
	    args.remove(arg)
	    nonposargs.append(arg)
	if arg=='dotestvars':
	    dotestvars = True
	    print('found option dotestvars')
	    args.remove(arg)
	    nonposargs.append(arg)
	if arg=='runlocal':
	    runlocal = True
	    print('found option runlocal')
	    args.remove(arg)
	    nonposargs.append(arg)
	if arg=='runqsub':
	    runqsub = True
            print('found option runqsub')
            args.remove(arg)
            nonposargs.append(arg)
	if arg=='unblind':
	    unblind = True
	    print('found option unblind')
	    args.remove(arg)
	    nonposargs.append(arg)
    # positional arguments
    if len(args)==4:	
	workspace = args[0]
	fitresultfile = os.path.abspath(args[1]) # note: ignored if making prefit plots
	mode = args[2] # must be either 'prefit' or 'postfit'
	outputdir = args[3] # note: ignored if running on a workspace directly
    else:
	raise Exception('ERROR: wrong number of command line arguments.'
			+' Found {}'.format(args))
    # check mode
    if mode not in ['prefit','postfit']:
	raise Exception('ERROR: mode {} not recognized'.format(mode))
    # check output dir
    if not os.path.exists(outputdir):
	os.makedirs(outputdir)
    
    ### if first argument is a workspace, directly run on that workspace
    if workspace[-5:]=='.root':
	workspace = os.path.abspath(workspace)
	thisoutputdir = os.path.dirname(workspace)

	datacard = workspace.replace('.root','.txt')
	if not os.path.exists(workspace):
	    raise Exception('ERROR: workspace {} does not exist'.format(workspace))
	if( mode=='postfit' and not os.path.exists(fitresultfile) ):
	    raise Exception('ERROR: fitresultfile {} does not exist'.format(fitresultfile))
	if not os.path.exists(datacard):
	    raise Exception('ERROR: datacard {} does not exist'.format(datacard))
      
	histdict = runPostFitShapesFromWorkspace( workspace,datacard,mode,
						    fitresultfile=fitresultfile)
	# - the histdict contains an entry 'postfit' (if --total-shapes was added)
	# with all processes for all channels summed + correctly evaluated uncertainty;
	# use this only for total uncertainty (not nominal) as it is not split per process.
	# - the histdict contains entries <channelname>_postfit
	# with the different processes for that channel; 
	# use only for nominal, not total uncertainty.
	# also similar entries for prefit
	
	# get total data
	datahist = histdict[mode]['data_obs'].Clone()
	# get total simulated uncertainty (bin content = sum, bin error = stat+syst error)
	simhisterror = histdict[mode]['TotalProcs'].Clone()
	# get nominal sums for each process over all channels
	simhistdict = {}
	for key in histdict.keys():
	    if not mode in key: continue
	    if key==mode: continue
	    #print(key)
	    thisdict = histdict[key]
	    for pkey in thisdict.keys():
		if('Total' in pkey or 'data' in pkey): continue 
		#print('- '+pkey)
		if not pkey in simhistdict:
		    simhistdict[pkey] = thisdict[pkey].Clone()
		else:
		    simhistdict[pkey].Add( thisdict[pkey].Clone() )
	# clip all histograms and ignore artificial small values
	ht.cliphistogram(simhisterror,clipboundary=1e-5)
	for hist in simhistdict.values(): ht.cliphistogram(hist,clipboundary=1e-5)
	# check that bin content of simhisterror = sum of processes
	simhistsum = simhistdict[simhistdict.keys()[0]].Clone()
	simhistsum.Reset()
	for pkey in simhistdict: simhistsum.Add(simhistdict[pkey].Clone())
	ht.printhistogram(simhistsum)
	ht.printhistogram(simhisterror)
	for i in range(0,simhistsum.GetNbinsX()+2):
	    if( abs(simhistsum.GetBinContent(i)-simhisterror.GetBinContent(i)) > 1e-2 ):
		raise Exception('ERROR: total simulation and sum-of-processes does not agree')

	# check if stat-only workspace exists and run on it
	statworkspace = workspace.replace('.root','_statonly.root')
	statdatacard = datacard.replace('.txt','_statonly.txt')
	simhiststaterror = None
	if os.path.exists(statworkspace):
	    print('trying to read stat-only uncertainties...')
	    stathistdict = runPostFitShapesFromWorkspace( statworkspace,statdatacard,mode,
							    fitresultfile=fitresultfile )
	    # get total simulated uncertainty (bin content = sum, bin error = stat+syst error)
	    simhiststaterror = stathistdict[mode]['TotalProcs'].Clone()
	    ht.printhistogram(simhiststaterror)
	else:
	    print('WARNING: stat-only workspace not found, considering only total uncertainty!')

	# set lumi
	lumi = 0.
	if '2016' in workspace: lumi = 35900
	elif '2017' in workspace: lumi = 41500
	elif '2018' in workspace: lumi = 59700
	else: lumi = 137100

	# set extra info to display
	extrainfos = []
	if('signalregion' in workspace): extrainfos.append('tZq enriched')
	if('wzcontrolregion' in workspace): extrainfos.append('WZ enriched')
	if('zzcontrolregion' in workspace): extrainfos.append('ZZ enriched')
	if('zgcontrolregion' in workspace): extrainfos.append(r'Z#gamma enriched')
	if('ttzcontrolregion' in workspace): extrainfos.append(r't#bar{t}Z enriched')
	if('signalsideband' in workspace): extrainfos.append('nonprompt enriched')
	if('signalregion' in workspace and '_cat1_' in workspace): 
	    extrainfos.append('1 b-jet, 2-3 jets')
	if('signalregion' in workspace and '_cat2_' in workspace): 
	    extrainfos.append(r'1 b-jet, #geq 4 jets')
	if('signalregion' in workspace and '_cat3_' in workspace): 
	    extrainfos.append(r'#geq 2 b-jets')
	if('signalregion' in workspace and '_ch0_' in workspace): 
	    extrainfos.append('channel eee')
	if('signalregion' in workspace and '_ch1_' in workspace): 
	    extrainfos.append('channel eem')
	if('signalregion' in workspace and '_ch2_' in workspace): 
	    extrainfos.append('channel emm')
	if('signalregion' in workspace and '_ch3_' in workspace): 
	    extrainfos.append('channel mmm')
	
	# set x-axis title
	var = ''
	varsplit = workspace.split('_var_')
	if len(varsplit)!=2: xaxtitle=None
	else:
	    var = varsplit[1].replace('.root','')
	    varentry = [v for v in get_variable_list( istest=dotestvars ) if v['name']==var]
	    if len(varentry)!=1: xaxtitle=None
	    else:
		varentry = varentry[0]
		xaxtitle = varentry['title']
		if varentry['unit']!='':
		    xaxtitle += r' ['+varentry['unit']+r']'

	# determine if blinding is needed
	if( 'eventBDT' in var and 'signalregion' in datacard and not unblind ): datahist.Reset()

	figname = workspace.replace('.root','')
        figname = figname.replace('dc_combined_','')
        figname = figname.replace('datacard_','')
	histplotter_postfit( simhisterror, simhiststaterror, simhistdict.values(), datahist, 
			    figname, xaxtitle=xaxtitle, lumi=lumi, extrainfos=extrainfos )

    ### if first argument is a dict matching a name to a list of channels, run on this combo
    # mostly for internal use, do not try to call this manually
    elif( isdict(workspace) ):
	variables = get_variable_list( istest=dotestvars )
	print('running on the following variables:')
	for v in variables: print('  - {}'.format(v['name']))
	channelcombo = json.loads( workspace )
	comboname = channelcombo['name']
	channels = channelcombo['channels']
	signals = channelcombo['signals']
        wspacepaths = makeworkspace(channels,signals,[v['name'] for v in variables],
				    outputdir,comboname,dostatonly=True)
        for wspacepath in wspacepaths:
            command = 'python histplotter_postfit.py {} {} {} {}'.format(
                        wspacepath,fitresultfile,mode,outputdir)
	    for arg in nonposargs: command += ' '+arg
	    os.system( command )
	if doclean: cleanplotdir(os.path.dirname(wspacepaths[0]))

    ### if first argument is a folder, run on predefined regions and years within it
    else:
	workspace = os.path.abspath(workspace)

	# define what histograms to read (regions, years, npdatatype)
	npfromdata = True
	regions = []
	regions.append('all')
	#regions.append('signalregion_cat1')
	#regions.append('signalregion_cat2')
	#regions.append('signalregion_cat3')
	#regions.append('signalregion_cat123')
	#regions.append('wzcontrolregion')
	#regions.append('zzcontrolregion')
	#regions.append('zgcontrolregion')
	#regions.append('ttzcontrolregion')
	#regions.append('signalsideband_noossf_cat1')
	#regions.append('signalsideband_noossf_cat123')
	#regions.append('signalsideband_noz_cat1')
	#regions.append('signalsideband_noz_cat123')
	#regions.append('signalsideband_cat1')
        #regions.append('signalsideband_cat123')
	#regions.append('signalregion_cat1_top')
        #regions.append('signalregion_cat2_top')
        #regions.append('signalregion_cat3_top')
        #regions.append('signalregion_cat123_top')
	#regions.append('signalregion_cat1_antitop')
        #regions.append('signalregion_cat2_antitop')
        #regions.append('signalregion_cat3_antitop')
        #regions.append('signalregion_cat123_antitop')
	years = ['2016','2017','2018']

	# define what processes to consider as signal
	signals = ['tZq']

	# automatically determine what regions to consider if requested
	regionsdict = {}
	if( len(regions)==1 and regions[0]=='all' ):
	    regionsdict[years[0]] = os.listdir(os.path.join(workspace,years[0]+'combined'))
	    regionsdict['yearscombined'] = regionsdict[years[0]][:]
	    for year in years[1:]:
		regionsdict[year] = os.listdir(os.path.join(workspace,year+'combined'))
		for r in regionsdict['yearscombined']:
		    if r not in regionsdict[year]: regionsdict['yearscombined'].remove(r)
	else:
	    for year in years: regionsdict[year] = regions[:]
	    regionsdict['yearscombined'] = regions[:]
    
	# make a list of channel info dicts for easy looping
	channels = []
	suffix = 'npfromdata' if npfromdata else 'npfromsim'
	for year in years:
	    for region in regionsdict[year]:
		channels.append( {'name':region+'_'+year,
		    'region':region,
		    'year':year,
		    'npfromdata':npfromdata,
		    'path':os.path.join(workspace,year+'combined',region,suffix,'combined.root')})

	commands = []
	# make plots of individual channels
	for c in channels:
	    print('running on channel '+c['name'])
	    channelcombo = ({'name':c['name'],'channels':[c],'signals':signals})
	    channelcombostr = json.dumps(channelcombo).replace(' ','')
	    cmd = "python histplotter_postfit.py '{}' {} {} {}".format(
			    channelcombostr,fitresultfile,mode,outputdir)
	    for arg in nonposargs: cmd += ' '+arg
	    if runlocal: os.system(cmd)
	    commands.append(cmd)

	# make plots for full run II
	if sorted(years)==['2016','2017','2018']:
	    for region in regionsdict['yearscombined']:
		print('running full run II for region '+region)
		thischannels = [c for c in channels if c['region']==region]
		print(thischannels)
		thisname = region+'_yearscombined'
		channelcombo = ({'name':thisname,'channels':thischannels,'signals':signals})
		channelcombostr = json.dumps(channelcombo).replace(' ','')
		cmd = "python histplotter_postfit.py '{}' {} {} {}".format(
                            channelcombostr,fitresultfile,mode,outputdir)
		for arg in nonposargs: cmd += ' '+arg
		if runlocal: os.system(cmd)
		commands.append(cmd)
	
	# submit jobs
	# case 1: qsub
	if runqsub:
	    for command in commands:
		script_name = 'qsub_histplotter_postfit.sh'
		with open( script_name, 'w') as script:
		    initializeJobScript( script )
		    script.write( command+'\n' )
		submitQsubJob( script_name )
	# case 2: condor
	if not (runqsub or runlocal):
	    # need to escape double quotes by repeating them (not for running locally!)
	    commands = [cmd.replace('"','""') for cmd in commands]
	    condortools.submitCommandsAsCondorCluster( 'cjob_histplotter_postfit', commands )
	    #pass

    sys.stderr.write('###done###\n')
