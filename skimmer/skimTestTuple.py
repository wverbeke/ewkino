################################################################
# run a skim on a single test file (for testing and debugging) #
################################################################

import sys
import os

if len(sys.argv)==1:
    print('Use with following command line args:')
    print(' - input_file_path')
    print(' - skim_condition')
    sys.exit()

# parse input arguments
input_file_path = sys.argv[1]
skim_condition = sys.argv[2]
output_dir = 'testing'

# check if input file exists
if not os.path.exists(input_file_path):
    raise Exception('ERROR: input file does not seem to exist.')

# check if skim condition is valid
if not skim_condition in (["noskim", "singlelepton", 
			"dilepton", "trilepton", 
			"fourlepton", "fakerate"]):
    raise Exception('ERROR: skim condition not recognized.')

# check if executable exists
exe = './skimmer'
if not os.path.exists(exe):
    raise Exception('ERROR: executable {} does not seem to exist'.format(exe))

# create the output directory if it does not exist
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# run the skim job
cmd = exe + ' ' + input_file_path + ' ' + output_dir + ' ' + skim_condition
print('running command: {}'.format(cmd))
os.system(cmd)
