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

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, a systematics executable is called.
# command line arguments: see below

if len(sys.argv) != 3:
    print('### ERROR ###: triggerefficiency.py requires two command-line arguments.')
    print('Normal usage from the command line:')
    print('python eventflattener.py <input_directory> <output_directory>')
    # maybe add more command line arguments later, keep hard coded for now
    sys.exit()

# parse command line arguments
input_directory = os.path.abspath(sys.argv[1])
output_directory = sys.argv[2]
if os.path.exists(output_directory):
    print('### WARNING ###: output direcory already exists. Clean it? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
os.makedirs(output_directory)
output_directory = os.path.abspath(output_directory)
cwd = os.getcwd()

# fill list of inputfiles
inputfiles = [os.path.join(input_directory,f) 
		for f in os.listdir(input_directory)
		if f[-5:]=='.root']

# check if executable is present
if not os.path.exists('./triggerefficiency'):
    print('### ERROR ###: triggerefficiency executable was not found.')
    print('Run make -f makeTriggerEfficiency before running this script.')
    sys.exit()

def submitjob(cwd,inputfile,output_directory):
    script_name = 'triggerefficiency.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
	output_file_name = inputfile.split('/')[-1].rstrip('.root')+'_triggerHistograms.root'
	output_file_path = os.path.join(output_directory,output_file_name)
        command = './triggerefficiency {} {}'.format(inputfile, output_file_path)
        script.write(command+'\n')
	print(command)
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

# loop over input files and submit jobs
inputfiles = sorted(inputfiles)
#inputfiles = [inputfiles[-1]]
for f in inputfiles:
    submitjob(cwd,f,output_directory)
