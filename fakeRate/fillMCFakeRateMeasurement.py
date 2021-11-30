#######################################################
# a python submitter for fillMCFakeRateMeasurement.cc #
#######################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# read command line arguments
runlocal = False
testrun = False
for arg in sys.argv[1:]:
    if arg=='runlocal': runlocal = True
    if arg=='testrun': testrun = True

# set global properties
years = ['2016','2017','2018']
flavours = ['electron','muon']
samplelistdirectory = os.path.abspath('sampleListsNew')
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate'

# check if executable exists
if not os.path.exists('./fillMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCFillFakeRateMeasurement first.')
    sys.exit()

# loop and submit jobs
cwd = os.getcwd()
for year in years:
    for flavour in flavours:
	samplelist = os.path.join(samplelistdirectory,
		    'samples_tunefoselection_'+flavour+'_'+year+'.txt')
	# check number of samples
	nsamples = 0
	with open(samplelist) as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		nsamples += 1
	print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s.')
        for i in range(nsamples):
	    if(testrun and i!=11): continue
            script_name = 'fillMCFakeRateMeasurement.sh'
            with open(script_name,'w') as script:
                initializeJobScript(script)
                script.write('cd {}\n'.format(cwd))
                command = './fillMCFakeRateMeasurement {} {} {} {} {} {}'.format(
			    flavour,year,sampledirectory,samplelist,i,testrun)
                script.write(command+'\n')
	    # for a test run: run locally
	    if testrun: os.system('bash '+script_name)
	    else:
		if runlocal: os.system('bash '+script_name)
		else: submitQsubJob(script_name)
