###################################################
# python submitter for testReweighterBTagShape.cc #
###################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

# define input files and other arguments
inputfiles = ([
		'/user/llambrec/Files/tzqidmedium0p4/2016MC/signalregion_3prompt/WZTo3LNu_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_Summer16.root',
		'/user/llambrec/Files/tzqidmedium0p4/2017MC/signalregion_3prompt/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Fall17.root',
		'/user/llambrec/Files/tzqidmedium0p4/2018MC/signalregion_3prompt/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Autumn18.root'
		])
event_selections = ['signalregion']
event_categories = [0]

# check if executable exists
if not os.path.exists('./checkJetFlavourContent'):
    print('### ERROR ###: executable does not seem to exist...')
    sys.exit()

# check if input files exist
for f in inputfiles:
    if not os.path.exists(f):
	print('### ERROR ###: inputfile {} does not seem to exist...'.format(f))
	sys.exit()

commands = []
for inputfile in inputfiles:
    for event_selection in event_selections:
	for event_category in event_categories:
	    command = './checkJetFlavourContent'
	    command += ' '+inputfile
	    command += ' '+event_selection
	    command += ' '+str(event_category)
	    commands.append(command)
ct.submitCommandsAsCondorCluster('cjob_checkJetFlavourContent', commands)
