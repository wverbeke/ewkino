###############################################
# a python submitter for fillMCClosureTest.cc #
###############################################

import sys
import os
# import submission tools for qsub
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
# import submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

# set global properties
isMCFR = True
use_mt = False
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (pick any combination from '2016PreVFP', '2016PostVFP', '2017' and '2018')
flavours = ['both','electron','muon']
# (pick any combination from 'muon', 'electron' and any string 
# different from 'muon' or 'electron' (perform closure test inclusive in lepton flavour))
processes = ['DY']
# (pick any combination from 'DY' and 'TT')
runmode = 'condor'
# (pick from 'qsub', 'condor' or 'local')
istestrun = False
# (note that this will not overwrite runmode, 
# only affects number of samples and number of events per sample)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL/'
samplelistdirectory = os.path.abspath('sampleListsUL')
# (see also below to set correct sample list name in loop!)
nevents = 0
# number of events to process
# (use 0 for all events)
# (note: a reweighting factor ntotal/nevents will be applied to each used event)
# (note: ignored for data, only applicable to simulation)

# check if executable exists
if not os.path.exists('./fillMCClosureTest'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeClosureTest_MC first.')
    sys.exit()

# loop over years, flavours and processes
cwd = os.getcwd()
cmds = []
for year in years:
    for flavour in flavours:
	for process in processes:
	    # retrieve the correct samplelist
	    samplelist = os.path.join(samplelistdirectory,
                            'samples_closureTest_'+process+'_'+year+'.txt')
	    # make the command
	    command = './fillMCClosureTest {} {} {} {} {} {} {} {} {}'.format(
                            isMCFR,use_mt,process,year,flavour,
                            sampledirectory,samplelist,istestrun,nevents)
	    cmds.append(command)

# loop over commands and submit jobs
if( runmode=='qsub' or runmode=='local' ):
    for cmd in cmds:
	script_name = 'qsub_fillMCClosureTest.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script)
	    script.write('cd {}\n'.format(cwd))
	    script.write(cmd+'\n')
	if runmode=='qsub': submitQsubJob(script_name)
	else: os.system('bash '+script_name)

elif( runmode=='condor' ):
    ct.submitCommandsAsCondorCluster('cjob_fillMCClosureTest', cmds,
                                     cmssw_version=CMSSW_VERSION) 
