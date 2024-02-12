#############################
# Make MVA score histograms #
#############################

import sys
import os
import argparse
sys.path.append('../../jobSubmission')
import condorTools as ct


if __name__=='__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfile', type=os.path.abspath, required=True)
    parser.add_argument('-o', '--outputfile', type=os.path.abspath, required=True)
    parser.add_argument('-n', '--nentries', type=int, default=-1)
    parser.add_argument('--runmode', default='local', choices=['local', 'condor'])
    args = parser.parse_args()

    # parse input arguments
    if os.path.splitext(args.outputfile)[1]!='.root':
        print('WARNING: adding .root to output file name {}'.format(args.outputfile))
        args.outputfile = os.path.splitext(args.outputfile)[0]+'.root'
    outputdir = os.path.dirname(args.outputfile)
    if not os.path.exists(outputdir):
        os.makedirs(outputdir)

    # make the commands
    cmds = []
    cmd = './fillmvahistograms {} {} {}'.format(args.inputfile, args.outputfile, args.nentries)
    cmds.append( cmd )

    # submit the job
    if args.runmode=='local':
        for cmd in cmds:
            print('Now running {}...'.format(cmd))
            os.system(cmd)
    else: ct.submitCommandsAsCondorJob( 'cjob_roc.sh', cmds, cmssw_version='CMSSW_12_4_6')
