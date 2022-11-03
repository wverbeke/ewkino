######################################################################
# a python looper over years and flavours to determine the fake rate #
######################################################################

import os
import sys
import ROOT
# import job submission tools for qsub
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION
# import python tools
sys.path.append('../Tools/python')
import histtools as ht
sys.path.append('../plotting/python')
import histplotter as hp
# import local tools
sys.path.append('python')
import fakeRateMeasurementTools as frt
from merge2016 import merge2016

years = ['2016PreVFP','2016PostVFP','2017','2018']
# (choose any combination from '2016', '2017' and '2018')
flavours = ['electron','muon']
# (choose any combination from 'muon' and 'electron')
use_mT = True
runmode = 'condor'
# (choose from 'condor', 'qsub' or 'local')
fitmethod = 'templatefit'
# (choose from 'subtraction', 'templatefit', or 'none')
# (in case of 'none', the fake rate is not measured, 
# but input plots are made if doprefitplots is True!)
doprefitplots = False
# (set to True to create plots of input histograms before subtraction or template fit)

# hadd files if needed
# note that the subfiles are assumed to be in a folder named FakeRateMeasurementSubFiles
# you may have to manually create that folder and move the subfiles in there
for year in years:
    if year=='2016Merged': continue # special case for later
    for flavour in flavours:
	basename = 'fakeRateMeasurement_data_'+flavour+'_'+year
	if use_mT: basename += '_mT'
	else: basename += '_met'
	basename += '_histograms'
	filename = basename + '.root'
	if os.path.exists(filename): continue
	subfolder = '.'
	cmd = 'hadd '+filename+' '
	cmd += os.path.join(subfolder,basename+'_sample*.root')
	print(cmd)
	os.system(cmd)
if '2016Merged' in years: # special case: merge 2016PreVFP and 2016PostVFP
    for flavour in flavours:
	basename = 'fakeRateMeasurement_data_'+flavour+'_2016Merged'
        if use_mT: basename += '_mT'
        else: basename += '_met'
        basename += '_histograms'
        filename = basename + '.root'
        if os.path.exists(filename): continue
	prename = filename.replace('2016Merged','2016PreVFP')
	postname = filename.replace('2016Merged','2016PostVFP')
	if not (os.path.exists(prename) and os.path.exists(postname)):
	    raise Exception('ERROR: the era 2016Merged was requested, but the files'
				+' for 2016PreVFP and 2016PostVFP are not present.')
	merge2016(prename, postname, outputfile=filename)

# loop over years and flavours
cwd = os.getcwd()
cmds = []
for year in years:
    for flavour in flavours:
	print('processing year {} / flavour {}...'.format(year,flavour))
	
	# check if the correct input file exists
	filebasename = 'fakeRateMeasurement_data_'+flavour+'_'+year
        if use_mT: filebasename += '_mT'
        else: filebasename += '_met'
        filebasename += '_histograms'
        inputfile = filebasename + '.root'
	if not os.path.exists(inputfile):
	    print('### ERROR ###: file '+inputfile+' not found, skipping it.')
	    continue
	
	# plot input histograms if requested
	# note: histograms are assumed to be named as follows:
	#	process_ptype_ftype_var_year_flavour_pT_ptvalue_eta_etavalue
	#	where:
	#	- process is a process tag (e.g. "DY" or "VV" or "data")
	#	- ptype is either "prompt" or "nonprompt" (absent for data!)
	#	- ftype is either "numerator" or "denominator"
	#	- var is usually "mT"
	if( doprefitplots ):
	    print('making input plots...')
	    outputdir_sub = 'fakeRateMeasurementPlots_{}_{}_subtraction'.format(year, flavour)
	    outputdir_fit = 'fakeRateMeasurementPlots_{}_{}_prefit'.format(year, flavour)
	    if not os.path.exists(outputdir_sub): os.makedirs(outputdir_sub)
	    if not os.path.exists(outputdir_fit): os.makedirs(outputdir_fit)
	    # initializations
	    var = 'mT' if use_mT else 'met'
	    histbasename = 'data_numerator_{}_{}_{}'.format(var,year,flavour)
	    (ptbins, etabins) = frt.readptetabins( inputfile, histbasename )
	    # loop over bins
	    for ptbin in ptbins:
		for etabin in etabins:
		    print('   bin pt {}, eta {}'.format(ptbin, etabin))
		    for ftype in ['denominator','numerator']:
			# get histograms
			ptbinstr = str(ptbin).replace('.','p')
			etabinstr = str(etabin).replace('.','p')
			thisbin = '{}_{}_{}_{}_{}_{}'.format(ftype, var, year, flavour, 
								ptbinstr, etabinstr)
			histograms = frt.loadselectedhistograms(inputfile, 
					    ftype, var, year, flavour, ptbin, etabin)
			datahist = histograms['datahist']
			prompthists = histograms['prompthists']
			nonprompthists = histograms['nonprompthists']
			# other plot settings
			xaxtitle = datahist.GetXaxis().GetTitle()
			yaxtitle = datahist.GetYaxis().GetTitle()
			lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
				    '2016PreVFP':19520, '2016PostVFP':16810,
				    '2016Merged':36300 }
			lumi = lumimap[year]
			extracmstext = 'Preliminary'
			extrainfos = []
			extrainfos.append('{} {}'.format(year, flavour))
			extrainfos.append('pT: {}, eta: {}, {}'.format(ptbin, etabin, ftype))
			colormap = {}
			colormap['WJets'] = ROOT.kAzure + 1
			colormap['TT'] = ROOT.kCyan + 1
			colormap['DY'] = ROOT.kBlue + 1
			colormap['VV'] = ROOT.kCyan - 7
			colormap['QCD'] = ROOT.kRed - 7
			colormap['Other nonprompt'] = ROOT.kRed + 1
			# make a plot without nonprompt
			figname = os.path.join(outputdir_sub, thisbin)
			hp.plotdatavsmc( figname, datahist, 
			    prompthists,
			    datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
			    colormap=colormap, 
			    xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
			    extracmstext=extracmstext,
			    extrainfos=extrainfos, infosize=15 )
			# make a plot with nonprompt
			figname = os.path.join(outputdir_fit, thisbin)
                        hp.plotdatavsmc( figname, datahist,
                            prompthists+nonprompthists,
                            datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
                            colormap=colormap,
                            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
                            extracmstext=extracmstext,
                            extrainfos=extrainfos, infosize=15 )

	# define the command depending on fitmetod
	command = ''
	### method 1: simple prompt subtraction
	if fitmethod=='subtraction':
	    # check if executable exists
	    if not os.path.exists('./fitFakeRateMeasurementSubtraction'):
		print('### ERROR ###: executable does not seem to exist...')
		print('               run make -f makeFitFakeRateMeasurementSubtraction first.')
		sys.exit()
	    doinputplot = False # do not use this older plotting method (replaced by above)
	    doinputsave = False # do not store intermediate results (only usable for plotting)
	    command = './fitFakeRateMeasurementSubtraction {} {} {} {} {}'.format(
			str(use_mT), flavour, year, doinputplot, doinputsave)
	    cmds.append(command)
	### method 2: template fits using combine
	elif fitmethod=='templatefit':
	    # make fake rate map directory if it does not exist yet
	    frmapdir = 'fakeRateMapsTemplateFit'
	    if not os.path.exists(frmapdir): os.makedirs(frmapdir)
	    var = 'mT' if use_mT else 'met'
	    command = 'python fitFakeRateMeasurementTemplateFit.py {} {} {} {}'.format(
			    var,flavour,year,frmapdir)
	    cmds.append(command)
	### method 3: template fits using simple chi2 fit
	elif fitmethod=='chi2fit':
	    # make fake rate map directory if it does not exist yet
            frmapdir = 'fakeRateMapsChi2Fit'
            if not os.path.exists(frmapdir): os.makedirs(frmapdir)
            var = 'mT' if use_mT else 'met'
            command = 'python fitFakeRateMeasurementChi2Fit.py {} {} {} {}'.format(
                            var,flavour,year,frmapdir)
            cmds.append(command)

# end the program if no fit method was specified
if( fitmethod is None or fitmethod=='none' ):
    print('fit method was set to none, terminating.')
    sys.exit()

# also end the program if the fit method was invalid
if( fitmethod not in ['subtraction', 'templatefit','chi2fit'] ):
    print('ERROR: fit method was invalid, terminating.')
    sys.exit()

# loop over commands and submit jobs
if( runmode=='qsub' or runmode=='local' ):
    for cmd in cmds:
	script_name = 'qjob_fitFakeRateMeasurement.sh'
        with open(script_name,'w') as script:
            initializeJobScript(script)
            script.write(cmd+'\n')
        if runmode=='qsub': submitQsubJob(script_name)
	else: os.system('bash '+script_name)

if( runmode=='condor' ):
    ct.submitCommandsAsCondorCluster('cjob_fitFakeRateMeasurement', cmds,
                                     cmssw_version=CMSSW_VERSION)
