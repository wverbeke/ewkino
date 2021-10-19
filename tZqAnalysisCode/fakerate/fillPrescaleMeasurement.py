#####################################################
# a python submitter for fillPrescaleMeasurement.cc #
#####################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

years = ['2016','2017','2018']

# check if executable exists
if not os.path.exists('./fillPrescaleMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillPrescaleMeasurement first.')
    sys.exit()

cwd = os.getcwd()

for year in years:
    # check number of samples
    nsamples = 0
    with open('../samplelists/samples_fakeratemeasurement_'+year+'.txt') as sf:
	for sl in sf: 
	    if not sl[0] == '#': nsamples += 1
    print('found '+str(nsamples)+' samples for '+year)
    print('start submitting')
    for i in range(nsamples):
    #for i in [0]:
	script_name = 'fillPrescaleMeasurement.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script)
	    script.write('cd {}\n'.format(cwd))
	    command = './fillPrescaleMeasurement {} {}'.format(year,i)
	    script.write(command+'\n')
	submitQsubJob(script_name)
	# alternative: run locally
	#os.system('bash '+script_name)
	
