#####################################################
# a python submitter for fillFakeRateMeasurement.cc #
#####################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# set global properties
years = ['2016','2017','2018']
flavours = ['muon','electron']
samplelistdirectory = os.path.abspath('sampleListsNew')
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate'

# check if executable exists
if not os.path.exists('./fillFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillFakeRateMeasurement first.')
    sys.exit()

# loop and submit jobs
cwd = os.getcwd()
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
	print('start submitting')
	for i in range(nsamples):
	    script_name = 'fillFakeRateMeasurement.sh'
	    with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './fillFakeRateMeasurement {} {} {} {} {}'.format(
			    flavour,year,sampledirectory,samplelist,i)
		script.write(command+'\n')
	    submitQsubJob(script_name)
	    # alternative: run locally
	    #if i==0: os.system('bash '+script_name)

