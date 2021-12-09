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


if __name__=='__main__':

    # global settings
    testrun = False
    # (run a single working point and a small number of events locally)
    nevents = -1
    # (number of events per file to process)
    runcondor = True
    # (submit on condor instead of qsub)
    resubmit = False
    # (only submit jobs for which no output root file is found)

    # check if all executables exist
    exes = []
    exes.append('closureTest_MC')
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

    # loop over cut values
    condorclustercmds = []
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
			    closurecmd = 'python closureTest_MC.py'
			    closurecmd += ' {}'.format(wdirname)
			    closurecmd += ' {}'.format(year)
			    closurecmd += ' {}'.format(flavor)
			    closurecmd += ' {}'.format(process)
			    closurecmd += ' {}'.format(ptRatioCut)
			    closurecmd += ' {}'.format(deepFlavorCut)
			    closurecmd += ' {}'.format(extraCut)
			    closurecmd += ' runlocal'
			    closurecmd += ' nevents={}'.format(nevents)
			    if testrun: closurecmd += ' testrun'
			    # add all commands to a list
			    cmds = []
			    cmds.append(closurecmd)
			    condorclustercmds.append(closurecmd)
			    # (only works if only one command per job!)
			    # submit as a job
			    cwd = os.getcwd()
			    script_name = 'qjob_gridMCClosureTest.sh'
			    with open(script_name,'w') as script:
				initializeJobScript(script)
				script.write('cd {}\n'.format(cwd))
				for cmd in cmds: script.write(cmd+'\n')
			    if testrun: 
				#os.system('bash '+script_name)
				continue
			    elif runcondor: continue
			    else: submitQsubJob(script_name)

    #if( runcondor and not testrun ):
    if( runcondor ):
	ct.submitCommandsAsCondorCluster('cjob_gridMCClosureTest', condorclustercmds,
					cmssw_version='CMSSW_10_2_25')
