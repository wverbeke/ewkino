#########################################################################
# Run the MC fake rate measurement in a grid search over FO definitions #
#########################################################################

import sys
import os
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION


# How to use?
# - Modify the contents of the get_conf function below,
#   defining the grid of the grid search.
# - Modify the main settings at the top of the __main__ section below,
#   according to your preferences.
# - Go to runMCFakeRateMeasurement.py and set the sample list directory
#   and sample directory correctly. 
#   These have to be hard-coded for now since condor seems to allow 10 args max.
#   Note that the sample lists are assumed to be named 
#   "samples_tunefoselection_<flavour>_<year>.txt".
# - Other prerequisites:
#   - Make sure all output of previous runs is deleted or moved away from this working directory,
#     in order to avoid unwanted interference.
#   - Make sure to have compiled the executables fillMCFakeRateMeasurement
#     and plotMCFakeRateMeasurement.
# - When all this is done, run "python gridMCFakeRateMeasurement.py".
#   It is recommended to do a few test runs first to make sure no errors appear.


def get_conf( testrun=False ):
    ### get configuration
    # must be modified manually according to the requested grid,
    # but put in a function nevertheless so it can be imported in gridMCClosureTest.py
    
    # set years and flavors
    years = ['2016PreVFP', '2016PostVFP', '2017', '2018']
    flavors = ['muon','electron']
    #years = ['2016PreVFP']
    #flavors = ['muon','electron']
    if testrun:
	years = ['2016PreVFP']
	flavors = ['muon','electron']

    # define cut values
    ptRatioCuts = [0.35, 0.4, 0.45, 0.5, 0.55]
    deepFlavorCuts = [0.015, 0.02, 0.025, 0.03, 0.05, 0.075, 0.1, 0.25, 0.5]
    #extraCuts = [0, 1, 2, 3]
    #ptRatioCuts = [0.5]
    #deepFlavorCuts = [0.5]
    extraCuts = [1]
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
    # (choose whether to do a test run (with reduced grid)
    #  or the full grid as defined in get_conf().)
    nevents = -1
    # (set maximum number of events per file to be processed, 
    #  use -1 for all available events.)
    runmode = 'condor'
    # (choose from "condor", "qsub" or "local".)

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

    # check number of jobs that will be submitted
    njobs = len(years)*len(flavors)
    njobs *= len(ptRatioCuts)*len(deepFlavorCuts)
    njobs *= len(extraCuts)
    print('the current grid configuration will result in {} jobs...'.format(njobs))
    print('coninue? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()

    # loop over cut values
    cwd = os.getcwd()
    cmds = []
    for ptRatioCut in ptRatioCuts:
	for deepFlavorCut in deepFlavorCuts:
	    for extraCut in extraCuts:
		
		# make a working directory for this FO ID
		ptrCutName = str(ptRatioCut).replace('.','p')
		dfCutName = str(deepFlavorCut).replace('.','p')
		exCutName = str(extraCut)
		wdirname = 'output_ptRatio{}_deepFlavor{}_extraCut{}'.format(
				    ptrCutName, dfCutName, exCutName)
		os.makedirs(wdirname)
		
		# loop over years and flavours
		for year in years:
		    for flavor in flavors:
			# make the runMCFakeRateMeasurement command
			cmd = 'python runMCFakeRateMeasurement.py'
			cmd += ' {}'.format(wdirname)
			cmd += ' {}'.format(year)
			cmd += ' {}'.format(flavor)
			cmd += ' {}'.format(ptRatioCut)
			cmd += ' {}'.format(deepFlavorCut)
			cmd += ' {}'.format(extraCut)
			cmd += ' nevents={}'.format(nevents)
			if testrun: cmd += ' testrun'
			cmds.append(cmd)
			# submit as a job
			if( runmode=='qsub' or runmode=='local'):
			    script_name = 'qjob_gridMCFakeRateMeasurement.sh'
			    with open(script_name,'w') as script:
				initializeJobScript(script, cmssw_version='CMSSW_10_6_29')
				script.write('cd {}\n'.format(cwd))
				script.write(cmd+'\n')
			    if runmode=='local': os.system('bash '+script_name)
			    if runmode=='qsub': submitQsubJob(script_name)

    if( runmode=='condor' ):
	ct.submitCommandsAsCondorCluster('cjob_gridMCFakeRateMeasurement', cmds,
                                        cmssw_version=CMSSW_VERSION)
