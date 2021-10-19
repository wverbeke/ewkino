#####################################################
# submit a single ./runsystematics command in a job #
#####################################################

import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer')) # old qsub way
from jobSubmission import submitQsubJob, initializeJobScript # old qsub way
sys.path.append(os.path.abspath('../../jobSubmission')) # new condor alternative
import condorTools as ct # new condor alternative

exe = './runsystematics_temp'
 
#input_file_path = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_new/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-madgraph-pythia8_Autumn18.root'
#input_file_path = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_new/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_Fall17.root'
input_file_path = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_new/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-madgraph-pythia8_Autumn18.root'
norm = '1.'
output_file_path = 'output_tzqsample_2018_electronscale.root'
nentries = '-1'
process_name = 'tZq'
event_selection = 'signalregion'
selection_type = '3prompt'
signal_categories = '123'
split_samples = 'False'
signal_channels = '4' 
topcharge = 'all'
readBDT = 'True'
bdtCombineMode = 'all'
pathToXMLFile = os.path.abspath('../bdt/out_all_data/weights/tmvatrain_BDT.weights.xml')
bdtCut = '-99 '
systematics = ['electronScale','electronRes']

cmd = exe
cmd += ' ' + input_file_path
cmd += ' ' + norm
cmd += ' ' + output_file_path
cmd += ' ' + nentries
cmd += ' ' + process_name
cmd += ' ' + event_selection
cmd += ' ' + selection_type
cmd += ' ' + signal_categories
cmd += ' ' + split_samples
cmd += ' ' + signal_channels
cmd += ' ' + topcharge
cmd += ' ' + readBDT
cmd += ' ' + bdtCombineMode
cmd += ' ' + pathToXMLFile
cmd += ' ' + bdtCut
for sys in systematics:
    cmd += ' ' + sys

cwd = os.getcwd()
script_name = 'qjob_{}.sh'.format(exe.strip('./'))
with open(script_name,'w') as script:
    initializeJobScript(script)
    script.write('cd {}\n'.format(cwd))
    script.write(cmd+'\n')
submitQsubJob(script_name, wall_time='48:00:00')

