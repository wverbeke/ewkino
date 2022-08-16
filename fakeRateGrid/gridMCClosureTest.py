#########################################################################
# Run the MC fake rate measurement in a grid search over FO definitions #
#########################################################################

import sys
import os
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from gridMCFakeRateMeasurement import get_conf


# How to use?
# - Check that the get_conf function in gridMCFakeRateMeasurement.py
#   is correctly configured.
# - Modify the main settings at the top of the __main__ section below,
#   according to your preferences.
# - Go to runMCClosureTest.py and set the sample list directory
#   and sample directory correctly.
#   These have to be hard-coded for now since condor seems to allow 10 args max.
#   Note that the sample lists are assumed to be named 
#   "samples_closuretest_<process>_<year>.txt".
# - Other prerequisites:
#   - Make sure to have compiled the executable runMCClosureTest
# - When all this is done, run "python gridMCClosureTest.py".
#   It is recommended to do a few test runs first to make sure no errors appear.


if __name__=='__main__':

    # global settings
    testrun = False
    # (run a single working point and a small number of events locally)
    nevents = 50000000
    # (number of events per file to process)
    runmode = 'condor'
    # (choose from "condor" or "qsub")
    resubmit = False
    # (only submit jobs for which no output root file is found)

    # check if all executables exist
    exes = []
    exes.append('runMCClosureTest')
    for exe in exes:
	if not os.path.exists(exe):
	    raise Exception('ERROR: exe {} does not exist.'.format(exe))

    # get configuration
    conf = get_conf( testrun=testrun )
    years = conf['years']
    flavors = conf['flavors']
    ptRatioCuts = conf['ptRatioCuts']
    deepFlavorCuts = conf['deepFlavorCuts']
    extraCuts = conf['extraCuts']
    processes = ['TT','DY']

    # check number of jobs that will be submitted
    njobs = len(years)*len(flavors)
    njobs *= len(ptRatioCuts)*len(deepFlavorCuts)
    njobs *= len(extraCuts)
    njobs *= len(processes)
    print('the current grid configuration will result in {} jobs...'.format(njobs))
    print('coninue? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()

    # loop over cut values
    cmds = []
    cwd = os.getcwd()
    for ptRatioCut in ptRatioCuts:
	for deepFlavorCut in deepFlavorCuts:
	    for extraCut in extraCuts:
		# check the  working directory
		ptrCutName = str(ptRatioCut).replace('.','p')
		dfCutName = str(deepFlavorCut).replace('.','p')
		exCutName = str(extraCut)
		wdirname = 'output_ptRatio{}_deepFlavor{}_extraCut{}'.format(
				    ptrCutName, dfCutName, exCutName)
		if not os.path.exists(wdirname):
		    msg = 'ERROR: working directory {}'.format(wdirname)
		    msg += ' does not exist, skipping...'
		    print(msg)
		    continue
		# loop over years and flavours and processes
		for year in years:
		    for flavor in flavors:
			for process in processes:
			    # check if output already exists
			    expout = 'closurePlots_MC_{}_{}_{}.root'.format(process,year,flavor)
			    if( resubmit and expout in os.listdir(wdirname) ): continue
			    # make the closureTest_MC command
			    cmd = 'python runMCClosureTest.py'
			    cmd += ' {}'.format(wdirname)
			    cmd += ' {}'.format(year)
			    cmd += ' {}'.format(flavor)
			    cmd += ' {}'.format(process)
			    cmd += ' {}'.format(ptRatioCut)
			    cmd += ' {}'.format(deepFlavorCut)
			    cmd += ' {}'.format(extraCut)
			    cmd += ' nevents={}'.format(nevents)
			    if testrun: cmd += ' testrun'
			    cmds.append(cmd)
			    # submit as a job
			    if( runmode=='qsub' or runmode=='local'):
				script_name = 'qjob_gridMCClosureTest.sh'
				with open(script_name,'w') as script:
				    initializeJobScript(script, cmssw_version='CMSSW_10_6_29')
				    script.write('cd {}\n'.format(cwd))
				    script.write(cmd+'\n')
				if runmode=='local': os.system('bash '+script_name)
				if runmode=='qsub': submitQsubJob(script_name)

    if( runmode=='condor' ):
        ct.submitCommandsAsCondorCluster('cjob_gridMCClosureTest', cmds,
                                        cmssw_version='CMSSW_10_6_29')
