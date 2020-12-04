#######################################################
# submitting multiple test jobs to the condor cluster #
#######################################################

import sys
import os
sys.path.append(os.path.abspath('..'))
import condorTools as ct

# define the commands to be submitted as a jobs
exe = '/user/llambrec/ewkino/skimmer/skimmer'
infiles = ['/pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino/SingleMuon/crab_Run2016B-17Jul2018_ver2-v1_singlelepton_data_2016_iTuple/200429_171629/0000/singlelep_{}.root'.format(i) for i in range(1,10)]
outdir = os.path.abspath('.')
cmds = [exe+' '+infile+' '+outdir+' trilepton' for infile in infiles]

# make and submit the jobs
# method 1: different cluster/job for each command
for cmd in cmds:
    ct.submitCommandAsCondorJob('test',cmd)

# method 2: group jobs in a cluster
ct.submitCommandsAsCondorCluster('test',cmds)

# method 3: group into larger jobs
job1 = cmds[:4]
job2 = cmds[4:]
ct.submitCommandsAsCondorJobs('test',[job1,job2])
