#####################################
# a python submitter for scanner.cc #
#####################################

import sys
import os
from fileListing import listParts
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct

if __name__=="__main__":

    sampledir = sys.argv[1]
    nfilesperjob = int(sys.argv[2])
    runmode = 'condor'

    # check if executable exists
    exe = './scanner'
    if not os.path.exists(exe):
	raise Exception('ERROR: executable {} does not exist.'.format(exe))

    # check if input directory exists
    if not os.path.exists(sampledir):
	raise Exception('ERROR: input directory {} does not exist.'.format(sampledir))

    # find all root files in sampledir
    rfiles = [os.path.join(sampledir,f) for f in os.listdir(sampledir) if f[-5:]=='.root']
    if len(rfiles)==0:
	raise Exception('ERROR: no root files found in {}.'.format(sampledir))
   
    # split files in lists of files_per_job
    chunks = list(listParts( rfiles, nfilesperjob ))

    # submit jobs
    cwd = os.getcwd()
    for chunk in chunks:
        commands = []
        commands.append( 'cd {}'.format(cwd) )
        for f in chunk:
            command = './scanner {}'.format(f)
            commands.append(command)
        # run locally (for testing and debugging)
        if runmode=='local':
	    for cmd in commands: os.system(cmd)
        # or submit condor job
        if runmode=='condor': ct.submitCommandsAsCondorJob( 'cjob_scanner', commands )
