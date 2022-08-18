###########################################################
# a python submitter for fillDataChargeFlipMeasurement.cc #
###########################################################

import sys
import os
# import job submission tools for condor
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
from jobSettings import CMSSW_VERSION

# set global properties
years = ['2016']
# (pick any combination from '2016', '2017' and '2018')
flavours = ['electron']
# (only 'electron supported for now')
runmode = 'local'
# (pick from 'condor' or 'local')
nentries = 1000
# (number of entries to use per file)
samplelistdirectory = os.path.abspath('sampleListsOld')
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_chargeflips'

# check if executable exists
exe = './fillDataChargeFlipMeasurement'
if not os.path.exists(exe):
    raise Exception('ERROR: executable {} does not seem to exist...'.format(exe))

# loop over years and flavours
cwd = os.getcwd()
cmds = []
for year in years:
    for flavour in flavours:
	samplelist = os.path.join(samplelistdirectory,
	    'samples_chargeFlips_MC_'+year+'.txt')  # which sample lists to use?
	# make the command and add it to the list
	command = '{} {} {} {} {} {}'.format(exe,
                    flavour,year,samplelist,sampledirectory,nentries)
	cmds.append(command)

# submit the commands as jobs
if( runmode=='local' ):
    for cmd in cmds: os.system(cmd)
elif( runmode=='condor' ):
    ct.submitCommandsAsCondorCluster('cjob_fillDataChargeFlipMeasurement', cmds,
		    cmssw_version=CMSSW_VERSION)
