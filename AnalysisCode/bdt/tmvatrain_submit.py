##################################################################
# A python script to submit tmvatrain.py as a job to the cluster #
##################################################################
import sys
import os
import json
sys.path.append(os.path.abspath('../../skimmer/'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

### Define configurations
combine = 'all'
varlist = (['\"_abs_eta_recoil\"','\"_Mjj_max\"','\"_lW_asymmetry\"',
                '\"_deepFlavor_max\"','\"_lT\"','\"_MT\"','\"_dPhill_max\"',
                '\"_pTjj_max\"','\"_dRlb_min\"','\"_HT\"','\"_dRlWrecoil\"',
		'\"_dRlWbtagged\"','\"_M3l\"','\"_abs_eta_max\"'])
varlist += (['\"_nJets\"','\"_nBJets\"']) # parametrized learning
indirs = ['\"/user/llambrec/Files/tzqidmedium0p4_forbdt/2016MC/signalregion_3tight_flat\"']
indirs.append('\"/user/llambrec/Files/tzqidmedium0p4_forbdt/2017MC/signalregion_3tight_flat\"')
indirs.append('\"/user/llambrec/Files/tzqidmedium0p4_forbdt/2018MC/signalregion_3tight_flat\"')
sidebanddirs = [d.replace('3tight','2tight') for d in indirs]
#sidebanddirs = []
treenames = ['\"blackJackAndHookers/treeCat1\"']
treenames.append('\"blackJackAndHookers/treeCat2\"')
treenames.append('\"blackJackAndHookers/treeCat3\"')
lopts = 'SplitMode=Random:NormMode=None'
fopts = "!H:!V" # help and verbosity level
fopts += ":NTrees=1500:BoostType=Grad" # options for ensemble
fopts += ":MinNodeSize=1%:MaxDepth=6:nCuts=200" # options for single tree
fopts += ":UseBaggedGrad=True:BaggedSampleFraction=1"
fopts += ":Shrinkage=0.03"
config = ({'combine':combine,
	    'indirs':json.dumps(indirs,separators=(',',':')),
	    'sidebanddirs':json.dumps(sidebanddirs,separators=(',',':')),
	    'treenames':json.dumps(treenames,separators=(',',':')),
	    'sigtag':'tZq',
	    'lopts':lopts,
	    'fopts':fopts,
	    'varlist':json.dumps(varlist,separators=(',',':'))
	    })

# make tmvatrain command and submit script
commands = []
tmvacommand = 'python tmvatrain.py'
for key in config.keys(): tmvacommand += ' '+key+'='+config[key]
commands.append(tmvacommand)
commands.append('echo ---command---')
commands.append('echo {}'.format(tmvacommand))
# old qsub way
#script_name = 'tmvatrain.sh'
#with open(script_name,'w') as script:
#    initializeJobScript( script )
#    for c in commands: script.write(c+'\n')
# for testing: run sequentially on m-machine
#os.system('bash '+script_name)
# actual submission
#submitQsubJob(script_name)
# new condor way
ct.submitCommandsAsCondorJob( 'cjob_tmvatrain', commands )
