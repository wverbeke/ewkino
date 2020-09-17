############################################################################
# python script to run the triggerefficiency executable via job submission #
############################################################################
import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

### command-line arguments:
# - input_directory: dir containing input root files for trigger efficiency measurement
# - output_directory: dir where trigger efficiency histograms will be stored (1 file per input file)

# check number of command line arguments
if len(sys.argv) != 4:
    print('### ERROR ###: triggerefficiency.py requires three command-line arguments.')
    print('Normal usage from the command line:')
    print('python triggerefficiency.py <input_directory> <output_directory> <event_selection>')
    # maybe add more command line arguments later, keep hard coded for now
    sys.exit()

# parse command line arguments
input_directory = os.path.abspath(sys.argv[1])
output_directory = sys.argv[2]
event_selection = sys.argv[3]

# check validity
if not event_selection in ['full','3tight','3tight_ptcuts','3fo','3fo_recoptcuts','3fo_coneptcuts',
			    '3fo_3tightveto_recoptcuts','3fo_3tightveto_coneptcuts']:
    print('### ERROR ###: event_selection '+event_selection+' not recognized')
    sys.exit()

# create output directory
if os.path.exists(output_directory):
    print('### WARNING ###: output direcory already exists. Clean it? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
os.makedirs(output_directory)
output_directory = os.path.abspath(output_directory)
cwd = os.getcwd()


# define tags to identify files to process 
# (files must contain at least one tag in list, if list is empty then take all files)
keeptags = []
keeptags.append('TTTo2L2Nu')
keeptags.append('tZq')
keeptags.append('data_combined')

# fill list of inputfiles
inputfiles = []
for f in os.listdir(input_directory):
    if f[-5:]!='.root': continue
    keep = True if len(keeptags)==0 else False
    for tag in keeptags:
	if tag in f:
	    keep = True
    if not keep: continue
    inputfiles.append(os.path.join(input_directory,f))

# check if executable is present
if not os.path.exists('./triggerefficiency'):
    print('### ERROR ###: triggerefficiency executable was not found.')
    print('Run make -f makeTriggerEfficiency before running this script.')
    sys.exit()

def submitjob(cwd,inputfile,output_directory,event_selection):
    script_name = 'triggerefficiency.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
	output_file_name = inputfile.split('/')[-1].rstrip('.root')+'_triggerHistograms.root'
	output_file_path = os.path.join(output_directory,output_file_name)
        command = './triggerefficiency {} {} {}'.format(inputfile, output_file_path, event_selection)
        script.write(command+'\n')
	print(command)
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

# loop over input files and submit jobs
inputfiles = sorted(inputfiles)
#inputfiles = [inputfiles[-1]]
for f in inputfiles:
    submitjob(cwd,f,output_directory,event_selection)
