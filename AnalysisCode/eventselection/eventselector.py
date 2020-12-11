########################################################################
# python script to run the eventselector executable via job submission #
########################################################################

import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer')) # old qsub 
from jobSubmission import submitQsubJob, initializeJobScript # old qsub
sys.path.append('../../jobSubmission') # new condor
import condorTools as ct # new condor
sys.path.append(os.path.abspath('../samplelists'))
from extendsamplelist import extendsamplelist
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, an event selection executable is called.
# command line arguments (in sequence):
# - input directory (containing skimmed and merged root files, usually one per sample)
# - output directory (containing analogous files but holding only selected events)
# - name of the event selection to be applied

if len(sys.argv) != 6:
	print('### ERROR ###: eventselector.py requires a different number of command-line arguments.')
	print('Normal usage from the command line:')
	print('python eventselector.py input_directory sample_list output_directory')
	print('event_selection selection_type')
	sys.exit()

input_directory = os.path.abspath(sys.argv[1])
sample_list = os.path.abspath(sys.argv[2])
output_directory = sys.argv[3]
output_directory = os.path.abspath(output_directory)
if os.path.exists(output_directory):
    os.system('rm -r '+output_directory)
event_selection = sys.argv[4]
selection_type = sys.argv[5]
variation = 'all' 
# maybe later add as a command line argument, however at this stage one probably wants
# to keep all variations, to perform a further selection at a later stage.
cwd = os.getcwd()

# check command line arguments
if event_selection not in (['signalregion','signalsideband_noossf','signalsideband_noz',
			    'wzcontrolregion','zzcontrolregion','zgcontrolregion',
			    'npcontrolregion']):
    print('### ERROR ###: event_selection not in list of recognized event selections')
    sys.exit()
if not os.path.exists(sample_list):
    print('### ERROR ###: sample_list does not seem to exist...')
    sys.exit()

# determine data type from sample name.
dtype = tls.data_type_from_samplelist(sample_list)
if len(dtype)==0 : sys.exit()

# make a list of input files in samplelist and compare to content of input directory 
inputfiles = extendsamplelist(sample_list,input_directory)
# the above does not work well for data, as it does not include the data_combined files
if dtype=='data':
    inputfiles = []
    files = [f for f in os.listdir(input_directory) if f[-5:]=='.root']
    for f in files:
        inputfiles.append({'file':os.path.join(input_directory,f),'process_name':'data'})

# check if executable is present
if not os.path.exists('./eventselector'):
    print('### ERROR ###: eventselector executable was not found.')
    print('Run make -f makeEventSelector before running this script.')
    sys.exit()

def submitjob_qsub(cwd,inputfile,output_directory,event_selection,selection_type,variation):
    script_name = 'eventselector.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './eventselector {} {} {} {} {} {}'.format(inputfile,output_directory,
    			inputfile.split('/')[-1],event_selection,selection_type,variation)
        script.write(command+'\n')
    	print(command)
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

def submitjob_condor(args):
    cmds = ['./eventselector '+argstring for argstring in args]
    ct.submitCommandsAsCondorCluster('eventselector_cjob',cmds)

# select input files based on type of selection
inputfiles = tls.subselect_inputfiles(inputfiles,selection_type)
if inputfiles is None: sys.exit()

# create output directory
os.makedirs(output_directory)

# loop over input files and submit jobs (old way)
#for f in inputfiles:
#    inputfile = f['file']
#    submitjob_qsub(cwd,inputfile,output_directory,event_selection,selection_type,variation)
    
# loop over input files and submit jobs (new way)
args = []
for f in inputfiles:
    inputfile = f['file']
    outputfile = inputfile.split('/')[-1]
    args.append('{} {} {} {} {} {}'.format(inputfile,output_directory,outputfile,
		    event_selection,selection_type,variation))
submitjob_condor(args)
