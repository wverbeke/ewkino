##############################################
# a python submitter for runMCClosureTest.cc #
##############################################
# note: within the fakeRateGrid environment, this is no longer a submitter;
#       it supports only local running (as instead it is itself wrapped in a job!)


import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# read fixed command line arguments
workingdir = sys.argv[1]
year = sys.argv[2]
flavour = sys.argv[3]
process = sys.argv[4]
ptRatioCut = sys.argv[5]
deepFlavorCut = sys.argv[6]
extraCut = sys.argv[7]
samplelistdirectory = os.path.abspath('sampleListsUL')
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL/'

# read command line arguments
testrun = False
nevents = -1
for arg in sys.argv[1:]:
    if arg=='testrun': testrun = True
    if arg.split('=',1)[0]=='nevents': nevents = arg.split('=',1)[1]

# printouts for logging
print('starting closureTest_MC.py...')
print('command line arguments:')
print('  - workingdir {}'.format(workingdir))
print('  - year {}'.format(year))
print('  - flavour {}'.format(flavour))
print('  - process {}'.format(process))
print('  - ptRatioCut {}'.format(ptRatioCut))
print('  - deepFlavorCut {}'.format(deepFlavorCut))
print('  - extraCut {}'.format(extraCut))
print('  - samplelistdirectory {}'.format(samplelistdirectory))
print('  - sampledirectory {}'.format(sampledirectory))
print('  - istestrun {}'.format(testrun))
print('  - nevents {}'.format(nevents))

# set global properties
isMCFR = True
use_mt = False
samplelist = os.path.join(samplelistdirectory,
                    'samples_closuretest_'+process+'_'+year+'.txt')

# check if executable exists
if not os.path.exists('./runMCClosureTest'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeRunMCClosureTest first.')
    sys.exit()

# change to dedicated working directory
print('changing working directory to {}'.format(workingdir))
cwd = os.getcwd()
os.chdir(workingdir)

# run c++ executable on each
print('starting ./runMCClosureTest...')
command = '../runMCClosureTest {} {} {} {} {} {} {} {} {} {} {} {}'.format(
                    isMCFR, use_mt, process, year, flavour,
                    sampledirectory,samplelist,
                    ptRatioCut, deepFlavorCut, extraCut,
                    testrun, nevents )    
print('running the following command:')
print(command)
sys.stdout.flush()
sys.stderr.flush()
os.system(command)
print('done with ./runMCClosureTest.')
print('done with runMCClosureTest.py!')
