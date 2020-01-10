#########################################################################################
# python script to use functionality from ewkino/skimmer and perform trilepton skimming #
#########################################################################################

import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from fileListing import listSampleDirectories, listFiles, listParts
from skimTuples import yearIdentifierFromPath
from jobSubmission import initializeJobScript, submitQsubJob

# this script reproduces the functionality of ewkino/skimmer/skimTuples.py,
# but with usage of a sampleList instead of automatically running over all samples.
# command line arguments (in sequence:)
# - input directory (e.g. /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino)
# - ntuple version (e.g. singlelepton_MC_2016_v1)
# - sample list (e.g. tzq_2016MC_samplelist.txt)
# - output directory (e.g. /storage_mnt/storage/user/llambrec/Files)
# - optional: files per job
# - optional: wall time per job

# default arguments (used for testing and debugging):
input_directory = '/storage_mnt/storage/user/llambrec/ewkino/test/testData/unskimmed'
version_name = 'Run2017E_test'
sample_list = 'samplelist_all.txt'
output_directory_base = '/storage_mnt/storage/user/llambrec/ewkino/test/testData/skimmed'
# fixed argument for now, but should be easy to adapt to allow different skimming conditions
skim_condition = 'trilepton'

# if too few command line args, check with the user if default arguments can be used
if len(sys.argv) < 5:
        print('### ERROR ###: trilepskim.py found too few command line arguments.')
	print('Normal usage from the command line:')
        print('python skimTuples.py < input_directory > < ntuple_version > < sample_list  > < output_directory > < files_per_job > < wall_time >')
        print('(files_per_job and wall_time have default values.)')
        print('Continue using only hard-coded arguments (e.g. for testing)? (y/n)')
	go = raw_input()
	if not go=='y':
		sys.exit()

# else, overwrite default arguments
else:
	input_directory = sys.argv[1] 
	version_name = sys.argv[2] 
	sample_list = sys.argv[3]
	output_directory_base = sys.argv[4] 
	files_per_job = int(sys.argv[5]) if len(sys.argv) > 5 else 50
    	wall_time = sys.argv[6] if len(sys.argv) > 6 else '24:00:00'

# set skimmer directory
skimmer_directory = os.path.abspath('../../skimmer')

# check if ./skimmer executable exists
if not os.path.exists('../../skimmer/skimmer'):
        print('### ERROR ###: skimmer executable does not seem to exist.')
        print('Go to ewkino/skimmer and run make -f makeSkimmer.')
        sys.exit()

# make a list of sample names to use
samples_to_use = []
useall = False
with open(sample_list) as f:
	for line in f:
		if(line[0] != '#' and len(line)>1):
			line = line.split(' ')[0]
			samples_to_use.append(line.rstrip('\n'))
if(len(samples_to_use)==1 and samples_to_use[0]=='all'):
	useall = True

# make a list of samples (the main directories) 
# and an equally long list of subdirectories with the latest version of the ntuples 
# (one for each main directory)
sample_directories = []
sample_sub_directories = []
for sample_directory, subdirectory in listSampleDirectories( input_directory, version_name ):
	# extra selection using samplelist:
	sample_name = sample_directory.rstrip( os.path.sep ).split( os.path.sep )[-1]
        if(useall or sample_name in samples_to_use):
                sample_directories.append( sample_directory )
                sample_sub_directories.append( subdirectory )

print('found '+str(len(sample_directories))+' valid sample directories.')

for asample in sample_directories:
	print(asample)

# make output directory for each sample
sample_names = []
for directory, subdirectory in zip(sample_directories, sample_sub_directories):
	sample_name = directory.rstrip( os.path.sep ).split( os.path.sep )[-1] 
	sample_name += '_' + yearIdentifierFromPath( subdirectory )
	sample_names.append(sample_name)
sample_output_directories = []
for sample in sample_names:
        output_directory = os.path.join( output_directory_base, 'ntuples_skimmed_{}_version_{}'.format( sample, version_name ) )
        if not os.path.exists( output_directory ):
        	os.makedirs( output_directory )
        sample_output_directories.append( output_directory )

for sample_directory, sub_directory, output_directory in zip( sample_directories, sample_sub_directories, sample_output_directories ):

        #print('processing the following sample:')
        #print('    sample directory: '+sample_directory)
        #print('    sub directory: '+sub_directory)
        #print('    output directory: '+output_directory)

        #identify locations of files to process for a given sample 
        root_files = list( listFiles( os.path.join( sample_directory, sub_directory ), '.root' ) )
        #print ('   number of files: '+str(len(root_files)))

	#split_files in lists of files_per_job
        for chunk in listParts( root_files, files_per_job ):

            #make a job script 
            script_name = 'trileptonskim.sh'
            with open( script_name, 'w') as script:
                initializeJobScript( script )
                script.write('cd {}\n'.format( skimmer_directory ) )
                for f in chunk :
                    skim_command = './skimmer {}'.format(f)
		    skim_command += ' {}'.format(output_directory)
		    skim_command += ' {}\n'.format(skim_condition)
                    script.write( skim_command )

            # submit job and catch errors 
            #submitQsubJob( script_name, wall_time )
            # alternative: run locally (for testing and debugging)
            #os.system('bash '+script_name)
