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

# read command line arguments
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
event_selection = sys.argv[4]
selection_type = sys.argv[5]
variation = 'all' 
cwd = os.getcwd()

# argument checks and parsing
if not os.path.exists(input_directory):
    raise Exception('ERROR: input directory '+input_directory+' does not seem to exist...')
if not os.path.exists(sample_list):
    raise Exception('ERROR: requested sample list does not seem to exist...')
if os.path.exists(output_directory):
    print('WARNING: output directory already exists. Clean it? (y/n)')
    go=raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
if event_selection not in (['wzcontrolregion','zzcontrolregion','zgcontrolregion']):
    raise Exception('ERROR: event_selection not in list of recognized event selections')
if variation not in ['nominal','all','JECDown','JECUp','JERDown','JERUp','UnclDown','UnclUp']:
    raise Exception('ERROR ###: variation not in list of recognized scale variations')

# check if executable is present
if not os.path.exists('./eventselector'):
    raise Exception('ERROR: eventselector executable was not found.')

def submitjob_condor(args):
    cmds = ['./eventselector '+argstring for argstring in args]
    ct.submitCommandsAsCondorCluster('eventselector_cjob',cmds)

# create output directory
os.makedirs(output_directory)

# check number of samples
nsamples = 0
with open(sample_list) as sf:
    for sl in sf:
        if(sl[0] == '#'): continue
        if(sl[0] == '\n'): continue
        if(not '.root' in sl): continue
        nsamples += 1
print('found '+str(nsamples)+' samples')

# loop over input files and submit jobs
commands = []
for i in range(nsamples):
    # make the command
    command = './eventselector {} {} {} {} {} {} {}'.format(
                    input_directory, sample_list, i, output_directory,
                    event_selection, selection_type, variation )

    commands.append(command)

# submit the jobs
ct.submitCommandsAsCondorCluster('cjob_eventselector',commands)
