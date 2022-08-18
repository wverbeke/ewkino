#####################################################
# a python submitter for fillCheckConeCorrection.cc #
#####################################################

# Notes on usage:
# - Basic usage: 
#   - Run "make -f makeFillCheckConeCorrection" to compile the c++ executable.
#   - Modify the settings below to your needs.
#   - Simply run "python fillCheckConeCorrection.py".

import sys
import os
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

flavours = ['electron','muon']
# (choose lepton flavours to include)
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (choose the data taking years to include)
samplelistdirectory = os.path.abspath('sampleLists/')
# (set the sample list directory, see also below for the expected sample list name!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL_ttmompt/'
# (set the directory where the samples are stored)

# check if executable exists
exe = './fillCheckConeCorrection'
if not os.path.exists(exe):
    raise Exception('ERROR: executable "{}" does not exist.'.format(exe))

# check input directory
if not os.path.exists(sampledirectory):
    raise Exception('ERROR: sample directory {} does not exist.'.format(sampledirectory))

# check sample lists
samplelists = {}
for year in years:
    samplelist = 'samples_checkconecorrection_{}.txt'.format(year) 
    samplelist = os.path.join(samplelistdirectory,samplelist)
    if not os.path.exists(samplelist):
	raise Exception('ERROR: sample list {} does not exist.'.format(samplelist))
    samplelists[year] = samplelist

# loop and submit jobs
cwd = os.getcwd()
commands = []
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
    print('found {} samples for {}'.format(samplecounter, year))
    # loop over lepton MVAs and flavours and working points
    for flavour in flavours:
	# loop over samples
	for i in range(samplecounter):
	    # make the command
	    command = exe +' {} {} {} {} {}'.format(
			flavour, year, sampledirectory, samplelist, i)
	    commands.append(command)
	    print('added job for year "{}",'.format(year)
		  +' flavour "{}",'.format(flavour)
		  +' sample {}/{}'.format(i+1,samplecounter))

ct.submitCommandsAsCondorCluster('cjob_fillCheckConeCorrection', commands,
                                 cmssw_version=CMSSW_VERSION)
