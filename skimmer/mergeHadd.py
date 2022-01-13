##########################################
# a simple submiter for the hadd command #
##########################################

import sys
import os
sys.path.append('../jobSubmission')
import condorTools as ct

if __name__=='__main__':

    # read command line args
    if len(sys.argv)<4:
        print('ERROR: need at least three command line arguments:')
        print('- target file')
        print('- at least two files to merge')
        sys.exit()

    targetfile = sys.argv[1]
    inputfiles = sys.argv[2:]
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
    print('continue? (y/n)')
    go = raw_input()
    if go != 'y': sys.exit()

    # make the command
    cmd = '{} {}'.format(exe,targetfile)
    for f in inputfiles: cmd += ' {}'.format(f)

    # submit the command
    ct.submitCommandAsCondorJob( 'cjob_mergeHadd', cmd )
