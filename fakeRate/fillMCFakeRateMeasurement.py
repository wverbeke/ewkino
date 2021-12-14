#######################################################
# a python submitter for fillMCFakeRateMeasurement.cc #
#######################################################

import sys
import os
# import job submission tools for qsub
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct

# set global properties
years = ['2016','2017','2018']
# (pick any combination from '2016', '2017' and '2018')
flavours = ['electron','muon']
# (pick any combination from 'electron' and 'muon')
runmode = 'condor'
# (pick from 'qsub', 'condor' or 'local')
istestrun = False
# (note that this will not overwrite runmode, 
# only affects number of samples and number of events per sample)
samplelistdirectory = os.path.abspath('sampleListsNew')
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate'

# check if executable exists
if not os.path.exists('./fillMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCFillFakeRateMeasurement first.')
    sys.exit()

# loop over years and flavours
cwd = os.getcwd()
cmds = []
for year in years:
    for flavour in flavours:
	# check number of samples
	samplelist = os.path.join(samplelistdirectory,
		    'samples_tunefoselection_'+flavour+'_'+year+'.txt')
	nsamples = 0
	with open(samplelist) as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		nsamples += 1
	print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s.')
	# loop over samples
        for i in range(nsamples):
	    if(istestrun and i!=11): continue
	    # make the command and add it to the list
	    command = './fillMCFakeRateMeasurement {} {} {} {} {} {}'.format(
                            flavour,year,sampledirectory,samplelist,i,istestrun)
	    cmds.append(command)

# submit the commands as jobs
if( runmode=='qsub' or runmode=='local' ):
    for cmd in cmds:
        script_name = 'qjob_fillMCFakeRateMeasurement.sh'
        with open(script_name,'w') as script:
            initializeJobScript(script)
            script.write('cd {}\n'.format(cwd))
            script.write(cmd+'\n')
	if runmode=='qsub': submitQsubJob(script_name)
	else: os.system('bash '+script_name)
elif( runmode=='condor' ):
    ct.submitCommandsAsCondorCluster('cjob_fillMCFakeRateMeasurement', cmds,
				    docmsenv=True, cmssw_version='CMSSW_10_2_25')
