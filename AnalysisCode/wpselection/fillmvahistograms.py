#####################################################################
# a python submission script for lepton MVA working point selection #
#####################################################################
import sys
import os
import ROOT
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# define input files
basedir = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_new/'
inputfiles = ([
		basedir+'2016MC/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_Summer16.root',
		basedir+'2017MC/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_Fall17.root',
		basedir+'2018MC/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-madgraph-pythia8_Autumn18.root'
	     ])

# define output directory
outputdir = os.path.abspath('fillmvahistograms_output')
if not os.path.exists(outputdir):
    os.makedirs(outputdir)
cwd = os.getcwd()

# check input files
for f in inputfiles:
    if(not os.path.exists(f)):
        print('### ERROR ###: requested inputfile '+f+' does not seem to exist')
        sys.exit()

# check if executable is present
exe_name = 'fillmvahistograms'
if not os.path.exists('./'+exe_name):
    print('### ERROR ###: executable not found')
    sys.exit()

# run over all input files
for f in inputfiles:
    
    # set output file name
    outputfilename = os.path.join(outputdir,f.split('/')[-1])
    if os.path.exists(outputfilename):
	os.system('rm '+outputfilename)
    # submit a job
    script_name = 'fillmvahistograms.sh'
    with open(script_name,'w') as script:
	initializeJobScript(script)
	script.write('cd {}\n'.format(cwd))
	command = './'+exe_name+' {} {}'.format(f,outputfilename)
	script.write(command+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name) 
