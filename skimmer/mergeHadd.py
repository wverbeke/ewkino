##########################################
# a simple submiter for the hadd command #
##########################################
# this script behaves just like the normal hadd command,
# with same input and output, 
# but wraps it in a condor job.

import sys
import os
sys.path.append('../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION

if __name__=='__main__':

    # read optional command line args
    argscopy = sys.argv[:]
    force = False
    for arg in sys.argv:
	if( arg=='-f' or arg=='--force' ): 
	    force = True
	    argscopy.remove(arg)
	elif( arg[0]=='-' or arg[:2]=='--' ):
	    raise Exception('ERROR: unrecognized optional argument: {}'.format(arg))

    # read command line args
    if len(argscopy)<4:
        print('ERROR: need at least three command line arguments:')
        print('- target file')
        print('- at least two files to merge')
        sys.exit()

    targetfile = argscopy[1]
    inputfiles = argscopy[2:]
    exe = 'hadd'

    # check if input files exist
    for f in inputfiles:
        if not os.path.exists(f):
            raise Exception('ERROR: input file {} does not exist'.format(f))

    # check if target file exists
    if os.path.exists(targetfile):
        raise Exception('ERROR: target file {} already exist,'.format(targetfile)
                        +' please remove it manually before recreating')

    # print and ask for confirmation
    print('will merge the following files using {}:'.format(exe))
    for f in inputfiles: print('- {}'.format(f))
    print('into the target file {}'.format(targetfile))
    if not force:
	print('continue? (y/n)')
	go = raw_input()
	if go != 'y': sys.exit()

    # make the command
    cmd = '{} {}'.format(exe,targetfile)
    for f in inputfiles: cmd += ' {}'.format(f)

    # submit the command
    ct.submitCommandAsCondorJob( 'cjob_mergeHadd', cmd,
				 cmssw_version=CMSSW_VERSION )
