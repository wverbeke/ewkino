############################################
# a python submitter for closureTest_MC.cc #
############################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

'''if not len(sys.argv)==3:
    print('### ERROR ###: found wrong number of command line arguments.')
    print('		  usage: python closureTest_MC.py isMCFR use_mt')
    sys.exit()

isMCFR = sys.argv[1]
use_mt = sys.argv[2]'''

isMCFR = True
use_mt = False

years = ['2016','2017','2018','all']
flavours = ['both'] 
# (put any string different from 'muon' or 'electron' to include both)
processes = ['TT','DY','all']
# (use 'TT', 'DY', 'all')
selections = ['default','onebjet','twojets','signalregion'] 
# (use 'default', 'noossf', 'noz', 'signaljets', 'signalregion')
path_to_xml_file = '/user/llambrec/ewkino/AnalysisCode/bdt/bdts_tzqtight/out_all_data/weights/'
path_to_xml_file += 'tmvatrain_BDT.weights.xml'

# check if executable exists
if not os.path.exists('./closureTest_MC'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeClosureTest_MC first.')
    sys.exit()

cwd = os.getcwd()

commands = []
for year in years:
    for flavour in flavours:
	for process in processes:
	    for selection in selections:
		command = './closureTest_MC {} {} {} {} {} {} {}'.format(
                            isMCFR,use_mt,selection,process,year,flavour,path_to_xml_file)
		# old qsub way:
		#script_name = 'closureTest_MC.sh'
		#with open(script_name,'w') as script:
		#	initializeJobScript(script)
		#	script.write('cd {}\n'.format(cwd))
		#	script.write(command+'\n')
		#submitQsubJob(script_name)
		# alternative: run locally
		#os.system('bash '+script_name)
		commands.append(command)

# new condor way:
ct.submitCommandsAsCondorCluster('closureTest_MC_cjob', commands)
