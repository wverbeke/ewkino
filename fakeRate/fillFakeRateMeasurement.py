#####################################################
# a python submitter for fillFakeRateMeasurement.cc #
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
# (choose any combination from '2016PreVFP','2016PostVFP', '2017' and '2018')
flavours = ['muon','electron']
# (choose any combination from 'muon' and 'electron')
runmode = 'condor'
# (choose 'qsub', 'condor' or 'local')
testrun = False
samplelistdirectory = os.path.abspath('sampleListsUL')
# (directory where the sample lists are)
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL'
# (directory where the samples are)
nevents = 0
# (number of events to process)
# (use 0 for all events)
# (note: a reweighting factor ntotal/nevents will be applied to each used event)
# (note: ignored for data, only applicable to simulation)

# check if executable exists
if not os.path.exists('./fillFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillFakeRateMeasurement first.')
    sys.exit()

# loop over years and flavours
cwd = os.getcwd()
cmds = []
for year in years:
    for flavour in flavours:
	# check number of samples
	samplelist = os.path.join(samplelistdirectory,
		    'samples_fakeratemeasurement_'+flavour+'_'+year+'.txt')
        nsamples = 0
	with open(samplelist) as sf:
	    for sl in sf:
		if not sl[0] == '#': nsamples += 1
	print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s')
	# loop over samples and make commands
	for i in range(nsamples):
	    if( testrun and i!=0 ): continue
	    command = './fillFakeRateMeasurement {} {} {} {} {} {} {}'.format(
                            flavour,year,sampledirectory,samplelist,i,testrun,nevents)
	    cmds.append(command)

# loop over commands and submit jobs
if(runmode=='qsub' or runmode=='local'):
    for cmd in cmds:
	script_name = 'qjob_fillFakeRateMeasurement.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script)
	    script.write('cd {}\n'.format(cwd))
	    script.write(cmd+'\n')
	if runmode=='qsub': submitQsubJob(script_name)
	else: os.system('bash '+script_name)

elif(runmode=='condor'):
    ct.submitCommandsAsCondorCluster('cjob_fillFakeRateMeasurement', cmds,
				     cmssw_version=CMSSW_VERSION)  
