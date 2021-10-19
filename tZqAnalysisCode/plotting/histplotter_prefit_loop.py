###############################################################
# run histplotter_prefit.py on all files in a given directory #
###############################################################
# note: not yet fully tested since some modifications,
#       maybe input/output filenames are not yet fully correctly configured

import sys
import os
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

# 1 command line argument (a folder), run on all files in it
if(len(sys.argv)==2):
    topdir = os.path.abspath(sys.argv[1])
    if not os.path.exists(topdir):
        raise Exception('ERROR: requested to run on '+topdir
                        +' but it does not seem to exist...')
    histfiles = []
    for root,dirs,files in os.walk(topdir):
        for fname in files:
            if fname=='combined.root': # depends on naming convention of file
                histfiles.append(os.path.join(root,fname))
    print('histplotter_prefit.py will run on the following files:')
    print(histfiles)
    commands = []
    for f in histfiles:
	inputfile = f
	outputfile = f.replace('.root','')
        command = 'python histplotter_prefit.py {} {}'.format(inputfile,outputfile)
        # old qsub way:
        script_name = 'qsub_histplotter_prefit.sh'
        with open(script_name,'w') as script:
            initializeJobScript( script )
            script.write(command)
        #submitQsubJob(script_name)
        # alternative: run locally
        #os.system('bash '+script_name)
        commands.append(command)
    #ct.submitCommandsAsCondorCluster('cjob_histplotter_prefit',commands)

else:
    print('### ERROR ###: unrecognized command line arguments.')
    sys.exit()

