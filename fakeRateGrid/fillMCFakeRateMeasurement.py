#######################################################
# a python submitter for fillMCFakeRateMeasurement.cc #
#######################################################

import sys
import os
import datetime
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# read fixed position command line args
outputdir = sys.argv[1]
year = sys.argv[2]
flavour = sys.argv[3]
ptRatioCut = sys.argv[4]
deepFlavorCut = sys.argv[5]
extraCut = sys.argv[6]

# read other command line arguments
runlocal = False
testrun = False
nevents = -1
for arg in sys.argv[3:]:
    if arg=='runlocal': runlocal = True
    if arg=='testrun': testrun = True
    if arg.split('=',1)[0]=='nevents': nevents = arg.split('=',1)[1]

# do some printouts for logging
print('starting fillMCFakeRateMeasurement.py...')
print('command line arguments:')
print('  - outputdir {}'.format(outputdir))
print('  - year {}'.format(year))
print('  - flavour {}'.format(flavour))
print('  - ptRatioCut {}'.format(ptRatioCut))
print('  - deepFlavorCut {}'.format(deepFlavorCut))
print('  - extraCut {}'.format(extraCut))
print('  - runlocal {}'.format(runlocal))
print('  - testrun {}'.format(testrun))
print('  - nevents {}'.format(nevents))

# set global properties
samplelistdirectory = os.path.abspath('sampleListsNew')
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate'
samplelist = os.path.join(samplelistdirectory,
                'samples_tunefoselection_'+flavour+'_'+year+'.txt')

# check if executable exists
if not os.path.exists('./fillMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCFillFakeRateMeasurement first.')
    sys.exit()

# check number of samples
nsamples = 0
with open(samplelist) as sf:
    for sl in sf:
	if(sl[0] == '#'): continue
	if(sl[0] == '\n'): continue
	nsamples += 1
print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s.')
# loop and submit jobs
cwd = os.getcwd()
for i in range(nsamples):
    if(testrun and i!=nsamples-1): continue
    script_name = 'qjob_fillMCFakeRateMeasurement_{}_{}_{}.sh'.format(outputdir,year,flavour)
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
	script.write('cd {}\n'.format(outputdir))
        command = '../fillMCFakeRateMeasurement {} {} {} {} {} {} {} {} {} {}'.format(
		    flavour,year,sampledirectory,
		    samplelist,i,
		    ptRatioCut, deepFlavorCut, extraCut,
		    testrun, nevents)
        script.write(command+'\n')
    print('created job script for sample {} of {}'.format(i+1,nsamples))
    print('current time: {}'.format(datetime.datetime.now().strftime("%H:%M:%S")))
    # for a test run: run locally
    if testrun: os.system('bash '+script_name)
    elif runlocal: os.system('bash '+script_name)
    else: submitQsubJob(script_name)
