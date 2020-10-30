############################################
# a python submitter for closureTest_MC.cc #
############################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# set global properties
isMCFR = True
use_mt = False
years = ['2016']
flavours = ['muon','electron','both'] 
# (put any string different from 'muon' or 'electron' to include both)
processes = ['TT','DY']
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/fakerate/'
samplelistdirectory = os.path.abspath('sampleLists')
# (see also below to set correct sample list name in loop!)

# check if executable exists
if not os.path.exists('./closureTest_MC'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeClosureTest_MC first.')
    sys.exit()

# loop and sumbit jobs
cwd = os.getcwd()
for year in years:
    for flavour in flavours:
	for process in processes:
	    script_name = 'closureTest_MC.sh'
	    samplelist = os.path.join(samplelistdirectory,
			    'samples_closureTest_'+process+'_'+year+'.txt')
	    with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './closureTest_MC {} {} {} {} {} {} {}'.format(
			    isMCFR,use_mt,process,year,flavour,
			    sampledirectory,samplelist)
		script.write(command+'\n')
	    submitQsubJob(script_name)
	    # alternative: run locally
	    #os.system('bash '+script_name)
