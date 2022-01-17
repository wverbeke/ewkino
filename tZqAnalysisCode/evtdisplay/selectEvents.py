##################################################
# submitter and argument parser for selectEvents #
##################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission')) # new condor alternative
import condorTools as ct # new condor alternative

nargs = 4
if len(sys.argv) != nargs+1:
    print('### ERROR ###: runsystematics.py requires {} command-line arguments.'.format(nargs))
    print('Normal usage from the command line:')
    print('python runsystematics.py input_file output_file, bdt_cut, nentries')
    sys.exit()

# hard-coded arguments
run_mode = 'condor' # choose from 'condor' or 'local'
event_selection = 'signalregion' # not expected to be changed
selection_type = '3tight' # not expected to be changed
signal_category = '0' # put cat number (1-3), 0 for no nJets/nBJets cut
signal_channel = '4' # put number of muons, 4 for all channels
topcharge = 'all' # put 'all', 'top' or 'antitop'

# parse command line arguments
input_file = os.path.abspath(sys.argv[1])
if not os.path.exists(input_file):
    raise Exception('ERROR: input file '+input_file+' does not seem to exist...')
output_file = os.path.abspath(sys.argv[2])
bdt_cut = sys.argv[3]
nentries = sys.argv[4]

# set path to BDT:
# (update: moved bdt's to subdirectories within bdt folder)
# (        keep here same code (apart from existence check) but add subdirectories in .cc)
# (        cannot do it here since C++ function needed to decide on subdirectory...)
path_to_xml_file = os.path.abspath('../bdt/out_all_data/weights/tmvatrain_BDT.weights.xml')

# check if executable is present
exe = './selectEvents'
if not os.path.exists(exe):
    raise Exception('ERROR: selectEvents executable was not found.'
		     +' Run make -f makeSelectEvents before running this script.')

# make the command
cmd = exe+' {} {} {} {} {} {} {} {} {} {}'.format(
	    input_file, output_file, nentries, 
	    event_selection, selection_type, 
	    signal_category, signal_channel, topcharge,
	    path_to_xml_file, bdt_cut)

# run the command
if run_mode == 'local': os.system(cmd)
elif run_mode == 'condor': ct.submitCommandAsCondorJob('cjob_selectEvents', cmd)
