######################################################
# submitting a single test job to the condor cluster #
######################################################

import sys
import os
sys.path.append(os.path.abspath('..'))
import condorTools as ct

# define the command to be submitted as a job
exe = '/user/llambrec/ewkino/skimmer/skimmer'
infile = '/pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino/SingleMuon/crab_Run2016B-17Jul2018_ver2-v1_singlelepton_data_2016_iTuple/200429_171629/0000/singlelep_1.root'
outdir = os.path.abspath('.')
cmd = exe+' '+infile+' '+outdir+' trilepton'

# make and submit the job
ct.submitCommandAsCondorJob('test',cmd)
