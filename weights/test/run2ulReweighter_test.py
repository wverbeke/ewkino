############################################
# a submitter for ReweighterBTagShape_test #
############################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct

if __name__=='__main__':

    inputfile = os.path.abspath(sys.argv[1])
    nevents = sys.argv[2]
    exe = './run2ulReweighter_test'

    if not os.path.exists(inputfile):
	raise Exception('ERROR: input file does not exist.')
    if not os.path.exists(exe):
	raise Exception('ERROR: executable does not exist.')

    (inputdir,inputfile) = os.path.split(inputfile)
    command = '{} {} {} {}'.format(exe, inputdir, inputfile, nevents)
    ct.submitCommandAsCondorJob( 'cjob_run2ulReweighter_test', command )
