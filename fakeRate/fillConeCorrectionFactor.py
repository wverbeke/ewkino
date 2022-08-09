######################################################
# a python submitter for fillConeCorrectionFactor.cc #
######################################################

# Notes on usage:
# - Basic usage: 
#   - Run "make -f makeFillConeCorrectionFactor" to compile the c++ executable.
#   - Modify the settings below to your needs.
#   - Simply run "python fillConeCorrectionFactor.py".
# - For the measurement to make sense, it is important that all chosen leptonMVA
#   working points are at a bin boundary.
#   The current default is 200 bins between -1 and 1, 
#   so any working point up to 0.01 precision will work.
#   The binning can only be modified hardcoded (for now)
#   in fillConeCorrectionFactor.cc
# - Dependencies: for this measurement, the loose lepton ID should be correctly configured.
#   The FO and Tight ID are not used in this measurement.

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
mvadict = ({'leptonMVATOPUL':
		{ 
		    'electron':
		     	{ 
		    	  'VLoose': 0.20,
		    	  'Loose': 0.41,
		    	  'Medium': 0.64,
		    	  'Tight': 0.81 
		    	},
		    'muon':
			{ 
			  'VLoose': 0.20, 
			  'Loose': 0.41,
			  'Medium': 0.64,
			  'Tight': 0.81 
			} 
		}
	    })
# (define a three-level dictionary to define the lepton MVAs, lepton flavours,
#  and lepton MVA working points;
#  for allowed keys corresponding to lepton MVAs, see below;
#  allowed keys corresponding to lepton flavours are "electron" and "muon";
#  allowed keys and values corresponding to working points are free to choose.)
# (choose lepton flavours to include)
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (choose the data taking years to include)
samplelistdirectory = os.path.abspath('sampleListsUL/')
# (set the sample list directory, see also below for the expected sample list name!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL/'
# (set the directory where the samples are stored)

# check value of leptonMVA
for leptonmva in sorted(mvadict.keys()):
    if not (leptonmva=='leptonMVAttH' 
	or leptonmva=='leptonMVAtZq' 
	or leptonmva=='leptonMVATOP'
	or leptonmva=='leptonMVATOPUL'
	or leptonmva=='leptonMVATOPv2UL'):
	raise Exception('ERROR: choice of leptonMVA "{}" not recognized.'.format(leptonmva))

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
    for leptonmva in sorted(mvadict.keys()):
	for flavour in sorted(mvadict[leptonmva].keys()):
	    for wpname in sorted(mvadict[leptonmva][flavour].keys()):
		wpvalue = mvadict[leptonmva][flavour][wpname]
		# loop over samples
		for i in range(samplecounter):
		    # make the command
		    command = exe +' {} {} {} {} {} {} {} {}'.format(
				flavour, year, leptonmva, wpname, wpvalue,
                                sampledirectory, samplelist, i)
		    commands.append(command)
		    print('added job for year "{}",'.format(year)
			    +' lepton MVA "{}",'.format(leptonmva)
			    +' flavour "{}",'.format(flavour)
			    +' working point "{}" ({})'.format(wpname, wpvalue)
			    +' sample {}/{}'.format(i+1,samplecounter))
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
