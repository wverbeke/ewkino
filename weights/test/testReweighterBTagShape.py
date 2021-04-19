###################################################
# python submitter for testReweighterBTagShape.cc #
###################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

# define input files and other arguments
inputfiles = ([
		'/pnfs/iihe/cms/store/user/llambrec/trileptonskim/2016MC/tZq_ll_4f_PSweights_13TeV-amcatnlo-pythia8_Summer16.root',
		'/pnfs/iihe/cms/store/user/llambrec/trileptonskim/2017MC/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_Fall17.root',
		'/pnfs/iihe/cms/store/user/llambrec/trileptonskim/2018MC/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-madgraph-pythia8_Autumn18.root'
		])
outputdir = 'testoutput'
modes = ['shape','wp']
event_selection = 'signalregion'
do_bdt = True
path_to_xml_file = os.path.abspath('../../AnalysisCode/bdt/out_all_data/weights/tmvatrain_BDT.weights.xml')

# check if executable exists
if not os.path.exists('./testReweighterBTagShape'):
    print('### ERROR ###: executable does not seem to exist...')
    sys.exit()

commands = []
for inputfile in inputfiles:
    for mode in modes:
	command = './testReweighterBTagShape'
	command += ' '+inputfile
	command += ' '+outputdir
	command += ' '+mode
	command += ' '+event_selection
	command += ' '+str(do_bdt)
	command += ' '+path_to_xml_file
	commands.append(command)
ct.submitCommandsAsCondorCluster('cjob_testReweighterBTagShape', commands)
