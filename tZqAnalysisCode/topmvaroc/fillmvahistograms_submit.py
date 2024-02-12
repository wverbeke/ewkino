################################################################
# Make MVA score histograms: submission with default arguments #
################################################################

import sys
import os
import argparse
sys.path.append('../../jobSubmission')
import condorTools as ct


if __name__=='__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-n', '--nentries', type=int, default=-1)
    parser.add_argument('--runmode', default='condor', choices=['local', 'condor'])
    args = parser.parse_args()

    inputdir = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_final/'
    fdict = {
      'tZq_2016.root': inputdir+'2016MC/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_Summer16.root',
      'tZq_2017.root': inputdir+'2017MC/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_Fall17.root',
      'tZq_2018.root': inputdir+'2018MC/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-madgraph-pythia8_Autumn18.root'
    }

    cmds = []
    for key,val in fdict.items():
        cmd = './fillmvahistograms {} {} {}'.format(val, key, args.nentries)
        cmds.append(cmd)

    # submit the job
    if args.runmode=='local':
        for cmd in cmds:
            print('Now running {}...'.format(cmd))
            os.system(cmd)
    else: ct.submitCommandsAsCondorCluster( 'cjob_roc.sh', cmds, cmssw_version='CMSSW_12_4_6')
