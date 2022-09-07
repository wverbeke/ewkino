#########################################
# a submitter for run2ulReweighter_test #
#########################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct
from jobSettings import CMSSW_VERSION

if __name__=='__main__':

    inputfile = os.path.abspath(sys.argv[1])
    outputfile = sys.argv[2]
    nevents = sys.argv[3]
    exe = './run2ulReweighter_test'

    if not os.path.exists(inputfile):
	raise Exception('ERROR: input file does not exist.')
    if not os.path.exists(exe):
	raise Exception('ERROR: executable does not exist.')

    (inputdir,inputfile) = os.path.split(inputfile)
    command = '{} {} {} {} {}'.format(exe, inputdir, inputfile, outputfile, nevents)
    ct.submitCommandAsCondorJob( 'cjob_run2ulReweighter_test', command,
				    cmssw_version=CMSSW_VERSION )
