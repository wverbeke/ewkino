################################################################
# simple submitter to make datacards for inclusive measurement #
################################################################

import sys
import os
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

if __name__=="__main__":

    if len(sys.argv)!=3:
        print('### ERROR ###: need different number of command line args:')
        print('               <input directory> and <datacard directory>')
        sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]
    runcondor = False

    commands = []
    commands.append('python inclusive_makedatacards.py {} {}'.format(topdir,datacarddir))

    # run via condor
    if runcondor:
	ct.submitCommandsAsCondorCluster('cjob_makedatacards',commands)

    # run via qsub
    else:
	script_name = 'qsub_inclusive_makedatacards.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script)
	    for command in commands:
		script.write(command+'\n')
	submitQsubJob( script_name )
