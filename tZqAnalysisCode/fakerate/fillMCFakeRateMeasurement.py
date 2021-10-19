#######################################################
# a python submitter for fillMCFakeRateMeasurement.cc #
#######################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

years = ['2016','2017','2018']
flavours = ['electron','muon']
istestrun = False

# check if executable exists
if not os.path.exists('./fillMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCFillFakeRateMeasurement first.')
    sys.exit()

cwd = os.getcwd()

commands = []
for year in years:
    for flavour in flavours:
	# check number of samples
	nsamples = 0
	with open('../samplelists/samples_tunefoselection_'+flavour+'_'+year+'.txt') as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		nsamples += 1
	print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s.')
        for i in range(nsamples):
	    if(istestrun and i!=11): continue
	    command = './fillMCFakeRateMeasurement {} {} {} {}'.format(
                            flavour,year,i,istestrun)
	    # old qsub way
            #script_name = 'fillMCFakeRateMeasurement.sh'
            #with open(script_name,'w') as script:
            #    initializeJobScript(script)
            #    script.write('cd {}\n'.format(cwd))
            #    script.write(command+'\n')
	    #if not istestrun:
	    #	submitQsubJob(script_name)
            # alternative: run locally
	    #else:
	    #	 os.system('bash '+script_name)
	    commands.append(command)
# new condor way:
ct.submitCommandsAsCondorCluster('fillMCFakeRateMeasurement_cjob', commands)    
