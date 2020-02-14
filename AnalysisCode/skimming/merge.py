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

# set input and output location
input_directory = os.path.abspath( sys.argv[1] )
output_directory = sys.argv[2]
if not os.path.exists(output_directory):
    os.makedirs(output_directory)
output_directory = os.path.abspath( sys.argv[2] )
print('searching for skimmed samples in '+input_directory)
inputlist = list(listSkimmedSampleDirectories( input_directory ))
print('found '+str(len(inputlist))+' input folders')

# reshape list into dict with unique sample names
# structure: {sampleName1:[sample1,sample2,...],sampleName2:[sample3,...]}
inputstruct = {}
for sample in inputlist:
    if sampleName(sample) in inputstruct:
        inputstruct[sampleName(sample)].append(sample)
    else:
        inputstruct[sampleName(sample)] = [sample]
print('found '+str(len(inputstruct))+' unique samples')
for s in inputstruct:
    print(s+': '+str(inputstruct[s]))

# run hadd command for each element in input structure
counter = 0
for samplename in inputstruct:
    samplelist = inputstruct[samplename]
    outfile = os.path.join(output_directory,sampleName(samplelist[0])+'.root')
    script_name = 'merge.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        command = 'hadd {}'.format(outfile)
        command2 = 'rm -r'
        for sample in samplelist:
            sample = os.path.join(input_directory,sample)
            command += ' {}/*.root'.format(sample)
            command2 += ' {}'.format(sample)
        script.write(command+'\n')
        script.write(command2+'\n')
    submitQsubJob(script_name)
    counter += 1
    print('submitted merging command for '+str(counter)+' sample directories.')	    
