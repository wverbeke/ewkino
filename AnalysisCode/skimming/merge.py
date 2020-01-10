####################################################################################
# python script to use functionality from ewkino/skimmer and merge skimmed ntuples #
####################################################################################

import sys 
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
from mergeTuples import listSkimmedSampleDirectories, sampleName

# this scipt reproduces the functionality of ewkino/skimmer/mergeTuples.py
# command line arguments (in sequence):
# - input directory
# - output directory

if len( sys.argv ) != 3:
    print( '### ERROR ###: merge.py requires additional command-line arguments.' )
    print( 'Usage : python merge.py <input_directory> <output_directory>' )
    print( 'The input directory should be the directory which contains the unmerged samples as subdirectories' )
    sys.exit()

input_directory = os.path.abspath( sys.argv[1] )
output_directory = sys.argv[2]
if not os.path.exists(output_directory):
    os.makedirs(output_directory)
output_directory = os.path.abspath( sys.argv[2] )
counter = 0
print('searching for skimmed samples in '+input_directory)
for sample in listSkimmedSampleDirectories( input_directory ):
    sample = os.path.join(input_directory,sample)
    script_name = 'merge.sh'
    with open(script_name,'w') as script:
	initializeJobScript(script)
	outfile = os.path.join(output_directory,sampleName(sample)+'.root')
	command = 'hadd {} {}/*.root'.format(outfile,sample)
	script.write(command+'\n')
	script.write('rm -r {}\n'.format(sample))
    submitQsubJob(script_name)
    counter += 1
print('submitted merging command for '+str(counter)+' sample directories.')
	    
