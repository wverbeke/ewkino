#####################################################
# a python submitter for fillPrescaleMeasurement.cc #
#####################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# set global properties
years = ['2016','2017','2018']
samplelistdirectory = os.path.abspath('../AnalysisCode/samplelists')
# (see also below to set correct sample list name in the loop!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate' 

# check if executable exists
if not os.path.exists('./fillPrescaleMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillPrescaleMeasurement first.')
    sys.exit()

# loop and submit jobs
cwd = os.getcwd()
for year in years:
    samplelist = os.path.join(samplelistdirectory,
				'samples_fakeratemeasurement_'+year+'.txt')
    # check number of samples
    nsamples = 0
    with open(samplelist) as sf:
	for sl in sf: 
	    if not sl[0] == '#': nsamples += 1
    print('found '+str(nsamples)+' samples for '+year)
    print('start submitting')
    for i in range(nsamples):
	script_name = 'fillPrescaleMeasurement.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script)
	    script.write('cd {}\n'.format(cwd))
	    command = './fillPrescaleMeasurement {} {} {} {}'.format(
			year,sampledirectory,samplelist,i)
	    script.write(command+'\n')
	submitQsubJob(script_name)
	# alternative: run locally
	#os.system('bash '+script_name)
	
