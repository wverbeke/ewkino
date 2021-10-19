###################################################################
# simple submitter to make datacards for top and antitop channels #
###################################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript,submitQsubJob

if __name__=="__main__":

    if len(sys.argv)!=3:
        print('### ERROR ###: need different number of command line args:')
        print('               <input directory> and <datacard directory>')
        sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]
    runcondor = False

    commands = []
    commands.append('python topchannels_makedatacards.py {} {}'.format(topdir,datacarddir))

    # run via condor
    if runcondor:
        ct.submitCommandsAsCondorCluster('cjob_topchannels_makedatacards',commands)

    # run via qsub
    else:
        script_name = 'qsub_topchannels_makedatacards.sh'
        with open(script_name,'w') as script:
            initializeJobScript(script)
            for command in commands:
                script.write(command+'\n')
        submitQsubJob( script_name )
