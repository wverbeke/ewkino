#########################################################################
# Run the MC fake rate measurement in a grid search over FO definitions #
#########################################################################

import sys
import os
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct

def get_conf( testrun=False ):
    ### get configuration
    # must be modified manually according to the requested grid,
    # but put in a function nevertheless so it can be imported in gridMCClosureTest.py
    
    # set years and flavors
    years = ['2016','2017','2018']
    flavors = ['muon','electron']
    if testrun:
	years = ['2016']
	flavors = ['muon','electron']

    # define cut values
    ptRatioCuts = [0.35, 0.4, 0.45, 0.5, 0.55]
    deepFlavorCuts = [0.015, 0.02, 0.025, 0.03, 0.05, 0.075, 0.1, 0.25, 0.5]
    extraCuts = [0, 1, 2, 3]
    if testrun:
	ptRatioCuts = [0.4]
	deepFlavorCuts = [0.02]
	extraCuts = [1]

    # return the result
    return {'years':years, 'flavors':flavors, 
	    'ptRatioCuts':ptRatioCuts, 
	    'deepFlavorCuts':deepFlavorCuts,
	    'extraCuts':extraCuts}


if __name__=='__main__':

    # global settings
    testrun = False
    nevents = -1
    runcondor = True

    # check if all executables exist
    exes = []
    exes.append('fillMCFakeRateMeasurement')
    exes.append('plotMCFakeRateMeasurement')
    for exe in exes:
	if not os.path.exists(exe):
	    raise Exception('ERROR: exe {} does not exist.'.format(exe))

    # get configuration
    conf = get_conf( testrun )
    years = conf['years']
    flavors = conf['flavors']
    ptRatioCuts = conf['ptRatioCuts']
    deepFlavorCuts = conf['deepFlavorCuts']
    extraCuts = conf['extraCuts']

    # loop over cut values
    condorclustercmds = []
    for ptRatioCut in ptRatioCuts:
	for deepFlavorCut in deepFlavorCuts:
	    for extraCut in extraCuts:
		# make a working directory
		ptrCutName = str(ptRatioCut).replace('.','p')
		dfCutName = str(deepFlavorCut).replace('.','p')
		exCutName = str(extraCut)
		wdirname = 'output_ptRatio{}_deepFlavor{}_extraCut{}'.format(
				    ptrCutName, dfCutName, exCutName)
		os.makedirs(wdirname)
		# loop over years and flavours
		for year in years:
		    for flavor in flavors:
			# make the fillMCFakeRateMeasurement command
			fillcmd = 'python fillMCFakeRateMeasurement.py'
			fillcmd += ' {}'.format(wdirname)
			fillcmd += ' {}'.format(year)
			fillcmd += ' {}'.format(flavor)
			fillcmd += ' {}'.format(ptRatioCut)
			fillcmd += ' {}'.format(deepFlavorCut)
			fillcmd += ' {}'.format(extraCut)
			fillcmd += ' nevents={}'.format(nevents)
			fillcmd += ' runlocal'
			if testrun: fillcmd += ' testrun'
			# make the plotMCFakeRateMeasurement command
			plotcmd = 'python plotMCFakeRateMeasurement.py'
			plotcmd += ' {}'.format(wdirname)
			plotcmd += ' {}'.format(year)
			plotcmd += ' {}'.format(flavor)
			# add all commands to a list
			cmds = []
			cmds.append(fillcmd)
			cmds.append(plotcmd)
			condorclustercmds.append(fillcmd)
			# (works only for single command so do not plot in this case!)
			# submit as a job
			cwd = os.getcwd()
			script_name = 'qjob_gridMCFakeRateMeasurement.sh'
			with open(script_name,'w') as script:
			    initializeJobScript(script)
			    script.write('cd {}\n'.format(cwd))
			    for cmd in cmds: script.write(cmd+'\n')
			if testrun: os.system('bash '+script_name)
			elif runcondor: 
			#    continue # work via single cluster submission instead
			    ct.submitCommandsAsCondorJob('cjob_gridMCFakeRateMeasurement',
				cmds, cmssw_version='CMSSW_10_2_25')
			else: submitQsubJob(script_name)

    if( runcondor and not testrun ):
	pass
	#ct.submitCommandsAsCondorCluster('cjob_gridMCFakeRateMeasurement', condorclustercmds,
        #                                cmssw_version='CMSSW_10_2_25')
