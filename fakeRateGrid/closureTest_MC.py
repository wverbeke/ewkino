############################################
# a python submitter for closureTest_MC.cc #
############################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# prntouts for logging
print('arguments provided on the command line:')
print(sys.argv)

# read fixed command line arguments
workingdir = sys.argv[1]
year = sys.argv[2]
flavour = sys.argv[3]
process = sys.argv[4]
ptRatioCut = sys.argv[5]
deepFlavorCut = sys.argv[6]
extraCut = sys.argv[7]

# read command line arguments
runlocal = False
testrun = False
nevents = -1
for arg in sys.argv[1:]:
    if arg=='runlocal': runlocal = True
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
print('  - runlocal {}'.format(runlocal))
print('  - istestrun {}'.format(testrun))
print('  - nevents {}'.format(nevents))

# set global properties
isMCFR = True
use_mt = False
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate/'
samplelistdirectory = os.path.abspath('sampleLists')
samplelist = os.path.join(samplelistdirectory,
                    'samples_closureTest_'+process+'_'+year+'.txt')

# check if executable exists
if not os.path.exists('./closureTest_MC'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeClosureTest_MC first.')
    sys.exit()

# sumbit jobs
cwd = os.getcwd()
script_name = 'qjob_closureTest_MC_{}_{}_{}_{}.sh'.format(workingdir,year,flavour,process)
with open(script_name,'w') as script:
    initializeJobScript(script)
    script.write('cd {}\n'.format(cwd))
    script.write('cd {}\n'.format(workingdir))
    command = '../closureTest_MC {} {} {} {} {} {} {} {} {} {} {} {}'.format(
		    isMCFR, use_mt, process, year, flavour,
		    sampledirectory,samplelist,
		    ptRatioCut, deepFlavorCut, extraCut,
		    testrun, nevents )
    script.write(command+'\n')
if testrun: os.system('bash '+script_name)
elif runlocal: os.system('bash '+script_name)
else: submitQsubJob(script_name)
