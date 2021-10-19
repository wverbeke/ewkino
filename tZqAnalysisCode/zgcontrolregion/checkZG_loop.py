###################################################
# python submitter for testReweighterBTagShape.cc #
###################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
sys.path.append('../../skimmer')
from jobSubmission import initializeJobScript, submitQsubJob

# define input files and other arguments
inputfiles = ({
		'/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_final/2016MC/ZGToLLG_01J_5f_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_Summer16.root':'ZG_2016',
		'/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_final/2017MC/ZGToLLG_01J_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Fall17.root':'ZG_2017',
		'/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_final/2018MC/ZGToLLG_01J_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Autumn18.root':'ZG_2018'
		})
event_selections = ['signalregion','zgcontrolregion']

# check if executable exists
if not os.path.exists('./checkZG'):
    print('### ERROR ###: executable does not seem to exist...')
    sys.exit()

# check if input files exist
for f in inputfiles.keys():
    if not os.path.exists(f):
	print('### ERROR ###: inputfile {} does not seem to exist...'.format(f))
	sys.exit()

commands = []
for inputfile,tag in inputfiles.items():
    for event_selection in event_selections:
	# make command
	outputfile = 'output_'+tag+'_'+event_selection+'.root'
	command = './checkZG'
	command += ' '+inputfile
	command += ' '+event_selection
	command += ' -1'
	command += ' '+outputfile
	commands.append(command)
	# submit via qsub
	script_name = 'qsub_checkZG.sh'
        with open( script_name, 'w') as script:
            initializeJobScript( script )
            script.write( command+'\n' )
        submitQsubJob( script_name )
#ct.submitCommandsAsCondorCluster('cjob_checkJetFlavourContent', commands)
