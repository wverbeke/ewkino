########################################################################
# python script to run the eventselector executable via job submission #
########################################################################

import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, an event selection executable is called.
# command line arguments (in sequence):
# - input directory (containing skimmed and merged root files, usually one per sample)
# - output directory (containing analogous files but holding only selected events)
# - name of the event selection to be applied

if len(sys.argv) != 4:
	print('### ERROR ###: eventselector.py requires a different number of command-line arguments.')
	print('Normal usage from the command line:')
	print('python eventselector.py <input_directory> <output_directory> <event_selection>')
	sys.exit()

input_directory = os.path.abspath(sys.argv[1])
output_directory = sys.argv[2]
if not os.path.exists(output_directory):
	os.makedirs(output_directory)
output_directory = os.path.abspath(output_directory)
event_selection = sys.argv[3]
cwd = os.getcwd()

# make a list of input files
inputfiles = []
os.chdir(input_directory)
for inputfile in glob.glob('*.root'):
	inputfiles.append(os.path.join(input_directory,inputfile))
os.chdir(cwd)
print('found '+str(len(inputfiles))+' root files in input directory.')

# check if executable is present
if not os.path.exists('./eventselector'):
	print('### ERROR ###: eventselector executable was not found.')
	print('Run make -f makeEventSelector before running this script.')
	sys.exit()

# loop over input files and submit jobs
for inputfile in inputfiles:
	script_name = 'eventselector.sh'
	with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './eventselector {} {} {}'.format(inputfile,output_directory,event_selection)
		script.write(command+'\n')
		# change name of output file to correspond to the name of the input file
		outname = inputfile[inputfile.rfind('/')+1:]
		script.write('cd '+output_directory+'\n')
		script.write('mv *'+outname+' '+outname+'\n')
		script.write('cd '+cwd+'\n')
	submitQsubJob(script_name)
	# alternative: run locally
	#os.system('bash '+script_name)
