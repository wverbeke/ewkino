#########################################################################
# python script to run the eventflattener executable via job submission #
#########################################################################

import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

# read command line args
if len(sys.argv) != 8:
    print('### ERROR ###: eventflattener.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python eventflattener.py <input_directory> <samplelist> <output_directory>') 
    print('<event_selection> <selection_type> <variation> <path_to_frmaps>')
    sys.exit()

input_directory = os.path.abspath(sys.argv[1])
samplelist = os.path.abspath(sys.argv[2])
output_directory = sys.argv[3]
output_directory = os.path.abspath(output_directory)
event_selection = sys.argv[4]
selection_type = sys.argv[5]
variation = sys.argv[6]
frdir = os.path.abspath(sys.argv[7])
cwd = os.getcwd()

# argument checks and parsing
if not os.path.exists(input_directory):
    raise Exception('ERROR: input directory '+input_directory+' does not seem to exist...')
if not os.path.exists(samplelist):
    raise Exception('ERROR: requested sample list does not seem to exist...')
if os.path.exists(output_directory):
    print('WARNING: output directory already exists. Clean it? (y/n)')
    go=raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
if event_selection not in (['wzcontrolregion','zzcontrolregion','zgcontrolregion']):
    raise Exception('ERROR: event_selection not in list of recognized event selections')
if variation not in ['nominal','JECDown','JECUp','JERDown','JERUp','UnclDown','UnclUp']:
    raise Exception('ERROR: variation not in list of recognized scale variations')

# check if executable is present
if not os.path.exists('./eventflattener'):
    raise Exception('ERROR: eventflattener executable was not found.')

# make output directory
os.makedirs(output_directory)

# check number of samples
nsamples = 0
with open(samplelist) as sf:
    for sl in sf:
        if(sl[0] == '#'): continue
        if(sl[0] == '\n'): continue
	if(not '.root' in sl): continue
        nsamples += 1
print('found '+str(nsamples)+' samples')

# loop over input files and submit jobs
commands = []
for i in range(nsamples):
    # modification needed below: give year as command line args to read correct map
    #muonfrmap = os.path.join(frdir,'fakeRateMap_data_muon_'+year+'_mT.root')
    #electronfrmap = os.path.join(frdir,'fakeRateMap_data_electron_'+year+'_mT.root')
    muonfrmap = 'none'
    electronfrmap = 'none'
    # make the command
    command = './eventflattener {} {} {} {} {} {} {} {} {}'.format(
                    input_directory, samplelist, i, output_directory,
                    event_selection, selection_type, variation,
                    muonfrmap, electronfrmap )

    commands.append(command)

# submit the jobs
ct.submitCommandsAsCondorCluster('cjob_eventflattener',commands)
