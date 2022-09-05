###########################################################################
# Merge separate folders containing to recovery tasks for the same sample #
###########################################################################

# Use case: if the crab resubmission of some sample does not finish in time,
# a so-called 'recovery task' can be created,
# which is basically just resubmission but the output is collected
# in a different directory (with appendix _recoveryTask).
# It is best to run this script after skimming and before merging,
# in order to put the recovery task files in the original sample folder 
# and delete the recovery task folders.

import sys
import os
import argparse

if __name__=='__main__':

    # parse command line arguments
    parser = argparse.ArgumentParser(description='Move recovery task files.')
    parser.add_argument('--sampledir', required=True,
                        help='Directory to scan for sample folders')
    args = parser.parse_args()

    # print arguments
    print('running with following configuration:')
    for arg in vars(args):
        print('  - {}: {}'.format(arg,getattr(args,arg)))

    # some more parsing
    sampledir = os.path.abspath(args.sampledir)

    # find folders corresponding to recovery tasks
    recodirs = ([os.path.join(sampledir,d) for d in os.listdir(sampledir) 
		 if 'recoveryTask' in d])
    if len(recodirs)==0:
	msg = 'No recovery task sample folders found in {}; exiting.'.format(sampledir)
	print(msg)
	sys.exit()

    # find corresponding sample folders
    sdirs = []
    validrecodirs = []
    for recodir in recodirs:
	sdir = recodir.replace('_recoveryTask','')
	if not os.path.exists(sdir):
	    msg = 'WARNING: recovery task folder {}'.format(recodir)
	    msg += ' does not seem to have an original equivalent'
	    msg += ' (i.e. {} does not exist).'.format(sdir)
	    print(msg)
	    continue
	sdirs.append(sdir)
	validrecodirs.append(recodir)
    
    # make the move commands
    mvcmds = []
    for sdir,recodir in zip(sdirs,recodirs):
	cmd = 'mv {} {}'.format(os.path.join(recodir,'*'),sdir)
	mvcmds.append(cmd)

    # printouts for checking
    print('will run the following mv commands:')
    for cmd in mvcmds:
	print('  - {}'.format(cmd))
    print('coninue? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()

    # execute the move commands
    for cmd in mvcmds:
	os.system(cmd)
