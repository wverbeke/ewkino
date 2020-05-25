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

if len( sys.argv ) < 3:
    print( '### ERROR ###: merge.py requires additional command-line arguments.' )
    print( 'Usage : python merge.py <input_directory> <output_directory> <mode>' )
    print( 'The input directory should be the directory which contains the unmerged samples as subdirectories' )
    sys.exit()

# parse mode command line argument
mode = 'default'
if len(sys.argv) == 4:
    mode = sys.argv[3]
else:
    print('### WARNING ###: use default mode? (y/n)')
    go = raw_input()
    if not go == 'y': sys.exit()
if not mode in ['default','trigger']:
    print('### ERROR ###: mode not recognized: '+str(mode))

# set input and output location
input_directory = os.path.abspath( sys.argv[1] )
output_directory = os.path.abspath( sys.argv[2] )
if not os.path.exists(output_directory):
    os.makedirs(output_directory)
print('searching for skimmed samples in '+input_directory)
inputlist = list(listSkimmedSampleDirectories( input_directory ))
print('found '+str(len(inputlist))+' input folders')

def sampleName_override(sample,mode):
    # override default sampleName function
    # default:
    name = sampleName(sample)
    # merge samples which only differ by '_extx'
    if '_ext' in name:
	name = name[:name.find('_ext')]+name[name.find('_ext')+5:]
    if mode=='trigger':
	if '_Run' in sample:
	    name = 'data' + sample[sample.find('_Run'):sample.find('_Run')+9]
    return name

# reshape list into dict with unique sample names
# structure: {sampleName1:[sample1,sample2,...],sampleName2:[sample3,...]}
inputstruct = {}
for sample in inputlist:
    samplename = sampleName_override(sample,mode)
    if samplename in inputstruct:
        inputstruct[samplename].append(sample)
    else:
        inputstruct[samplename] = [sample]
print('found '+str(len(inputstruct))+' unique samples')
for s in inputstruct:
    print(s+': '+str(inputstruct[s]))

# run hadd command for each element in input structure
counter = 0
for samplename in inputstruct:
    samplelist = inputstruct[samplename]
    outfile = os.path.join(output_directory,sampleName_override(
			    samplelist[0],mode)+'.root')
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
