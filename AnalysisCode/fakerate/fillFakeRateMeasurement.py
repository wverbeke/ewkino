#####################################################
# a python submitter for fillFakeRateMeasurement.cc #
#####################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

years = ['2016','2017','2018']
flavours = ['muon','electron']

# check if executable exists
if not os.path.exists('./fillFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillFakeRateMeasurement first.')
    sys.exit()

cwd = os.getcwd()

for year in years:
    # check number of samples
    nsamples = 0
    with open('../../fakeRate/sampleLists/samples_fakeRateMeasurement_'+year+'.txt') as sf:
        for sl in sf:
            if not sl[0] == '#': nsamples += 1
    print('found '+str(nsamples)+' samples for '+year)
    print('start submitting')
    for flavour in flavours:
	for i in range(nsamples):
	    script_name = 'fillFakeRateMeasurement.sh'
	    with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './fillFakeRateMeasurement {} {} {}'.format(flavour,year,i)
		script.write(command+'\n')
	    submitQsubJob(script_name)
	    # alternative: run locally
	    #os.system('bash '+script_name)

