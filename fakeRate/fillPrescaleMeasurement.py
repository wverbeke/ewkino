#####################################################
# a python submitter for fillPrescaleMeasurement.cc #
#####################################################

import sys
import os
# import job submission tools for qsub
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

# set global properties
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (choose any combination from '2016PreVFP', '2016PostVFP', '2017' and '2018')
runmode = 'local'
# (choose from 'condor', 'qsub' or 'local')
testrun = False
# (does not overwrite runmode, 
# only changes number of samples and number of events per sample to small values)
samplelistdirectory = os.path.abspath('sampleListsUL')
# directory where the sample lists are
# (see also below to set correct sample list name in the loop!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL'
# directory where the samples are
nevents = 1000
# number of events to process
# (use 0 for all events)
# (note: a reweighting factor ntotal/nevents will be applied to each used event)
# (note: ignored for data, only applicable to simulation)

# check if executable exists
if not os.path.exists('./fillPrescaleMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillPrescaleMeasurement first.')
    sys.exit()

# loop over years and samples and make commands
cwd = os.getcwd()
cmds = []
for year in years:
    samplelist = os.path.join(samplelistdirectory,
				'samples_fakeratemeasurement_'+year+'.txt')
    # check number of samples
    nsamples = 0
    with open(samplelist) as sf:
	for sl in sf: 
	    if not sl[0] == '#': nsamples += 1
    print('found '+str(nsamples)+' samples for '+year)
    # loop over samples
    for i in range(nsamples):
	if( testrun and i!=0 ): continue
	if i!=1: continue
	command = './fillPrescaleMeasurement {} {} {} {} {} {}'.format(
                        year, sampledirectory, samplelist, i, testrun, nevents)
	cmds.append(command)

# loop over commands and submit jobs
if( runmode=='qsub' or runmode=='local' ):
    for cmd in cmds:
	script_name = 'qjob_fillPrescaleMeasurement.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script, cmssw_version=CMSSW_VERSION)
	    script.write('cd {}\n'.format(cwd))
	    script.write(cmd+'\n')
	if runmode=='qsub': submitQsubJob(script_name)
	else: os.system('bash '+script_name)

if( runmode=='condor' ):
    ct.submitCommandsAsCondorCluster('cjob_fillPrescaleMeasurement', cmds,
                                     cmssw_version=CMSSW_VERSION)
