######################################################
# a python submitter for fillConeCorrectionFactor.cc #
######################################################
import sys
import os
# import job submission tools for qsub
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct

# set global properties
runmode = 'condor'
# (choose from 'local', 'qsub' or 'condor')
flavours = ['electron','muon']
# (choose lepton flavours to include)
leptonMVAs = ['leptonMVATOPUL']
# (pick a lepton MVA identifier, see below for allowed values)
mvathresholds = [0.64,0.64] 
# (tight thresholds; order and length must correspond to flavours)
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (choose the data taking years to include)
samplelistdirectory = os.path.abspath('sampleListsUL/')
# (set the sample list directory, see also below for the expected sample list name!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL/'
# (set the directory where the samples are stored)

# check value of leptonMVA
for leptonMVA in leptonMVAs:
    if not (leptonMVA=='leptonMVAttH' 
	or leptonMVA=='leptonMVAtZq' 
	or leptonMVA=='leptonMVATOP'
	or leptonMVA=='leptonMVATOPUL'
	or leptonMVA=='leptonMVATOPv2UL'):
	raise Exception('ERROR: choice of leptonMVA "{}" not recognized.'.format(leptonMVA))

# check if executable exists
exe = './fillConeCorrectionFactor'
if not os.path.exists(exe):
    raise Exception('ERROR: executable "{}" does not exist.'.format(exe))

# check input directory
if not os.path.exists(sampledirectory):
    raise Exception('ERROR: sample directory {} does not exist.'.format(sampledirectory))

# check sample lists
samplelists = {}
for year in years:
    samplelist = 'samples_conecorrectionfactor_{}.txt'.format(year) 
    samplelist = os.path.join(samplelistdirectory,samplelist)
    if not os.path.exists(samplelist):
	raise Exception('ERROR: sample list {} does not exist.'.format(samplelist))
    samplelists[year] = samplelist

# loop and submit jobs
cwd = os.getcwd()
commands = []
# loop over flavours and corresponding MVA thresholds
for flavour,mvathreshold in zip(flavours,mvathresholds):
    # loop over years
    for year in years:
	# check number of samples
	samplelist = samplelists[year]
	samplecounter = 0
	with open(samplelist) as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		samplecounter += 1
	print('found {} samples for {} {}s'.format(samplecounter, year, flavour))
	# loop over MVAs
	for leptonMVA in leptonMVAs:
	    # loop over samples
	    for i in range(samplecounter):
		# make the command
		command = exe +' {} {} {} {} {} {} {}'.format(flavour, year,
                                                          leptonMVA, mvathreshold,
                                                          sampledirectory, samplelist, i)
		commands.append(command)
		# run local or submit using qsub
		if( runmode=='local' or runmode=='qsub' ):
		    script_name = 'qjob_fillConeCorrectionFactor.sh'
		    with open(script_name,'w') as script:
			initializeJobScript(script, cmssw_version='CMSSW_10_6_29')
			script.write('cd {}\n'.format(cwd))
			script.write(command+'\n')
		    if runmode=='qsub': submitQsubJob(script_name)
		    elif runmode=='local': os.system('bash '+script_name)

# submit using condor
if runmode=='condor':
    ct.submitCommandsAsCondorCluster('cjob_fillConeCorrectionFactor', commands,
				     docmsenv=True, cmssw_version='~/CMSSW_10_6_29')
