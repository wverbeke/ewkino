#######################################################
# a python submitter for fillMCFakeRateMeasurement.cc #
#######################################################
# note: within the fakeRateGrid environment, this is no longer a submitter;
#       it supports only local running (as instead it is itself wrapped in a job!)

import sys
import os
import datetime

# read fixed position command line args
outputdir = sys.argv[1]
year = sys.argv[2]
flavour = sys.argv[3]
ptRatioCut = sys.argv[4]
deepFlavorCut = sys.argv[5]
extraCut = sys.argv[6]
samplelistdirectory = os.path.abspath('sampleListsUL')
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL'

# read other command line arguments
testrun = False
nevents = -1
for arg in sys.argv[7:]:
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
print('  - samplelistdirectory {}'.format(samplelistdirectory))
print('  - sampledirectory {}'.format(sampledirectory))
print('  - testrun {}'.format(testrun))
print('  - nevents {}'.format(nevents))

# set global properties
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

# change to dedicated working directory
print('changing working directory to {}'.format(outputdir))
cwd = os.getcwd()
os.chdir(outputdir)

# loop over samples and run c++ executable on each
print('starting ./fillMCFakeRateMeasurement loop...')
for i in range(nsamples):
    if(testrun and i!=nsamples-1): continue
    command = '../fillMCFakeRateMeasurement {} {} {} {} {} {} {} {} {} {}'.format(
		    flavour,year,sampledirectory,
		    samplelist,i,
		    ptRatioCut, deepFlavorCut, extraCut,
		    testrun, nevents)
    print('current time: {}'.format(datetime.datetime.now().strftime("%H:%M:%S")))
    print('running the following command:')
    print(command)
    sys.stdout.flush()
    sys.stderr.flush()
    os.system(command)
print('done with ./fillMCFakeRateMeasurement loop.')

# do hadd on the samples
filename = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
cmd = 'hadd '+filename+' '
cmd += 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms_sample*.root'
print('hadding files with following command:')
print(cmd)
sys.stdout.flush()
sys.stderr.flush()
os.system(cmd)

# run plotting command
print('plotting the fake rate map...')
if not os.path.exists(filename):
    print('ERROR: file '+filename+' not found (maybe something went wrong in hadd?);'
	    +' terminating...')
    sys.exit()
command = '../plotMCFakeRateMeasurement {} {}'.format(flavour,year)
os.system(command)
print('done making fake rate map for {}.'.format(filename))
print('done with fillMCFakeRateMeasurement.py!')
