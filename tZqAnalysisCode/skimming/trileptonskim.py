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
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist

# this script reproduces the functionality of ewkino/skimmer/skimTuples.py,
# but with usage of a sampleList instead of automatically running over all samples.
# command line arguments (in sequence:)
# - input directory (e.g. /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino)
# - sample list (e.g. tzq_2016MC_samplelist.txt)
# - output directory (e.g. /storage_mnt/storage/user/llambrec/Files)
# - optional: ntuple version (e.g. singlelepton_MC_2016_v1) 
#             (overwrites ntuple version that may be present in the sample list!)
# - optional: files per job
# - optional: wall time per job

# default arguments (used for testing and debugging):
input_directory = '/storage_mnt/storage/user/llambrec/ewkino/test/testData/unskimmed'
version_name = 'Run2017E_test'
sample_list = '../samplelists/samplelist_all.txt'
output_directory_base = '/storage_mnt/storage/user/llambrec/ewkino/test/testData/skimmed'
# fixed argument for now, but should be easy to adapt to allow different skimming conditions
skim_condition = 'trilightlepton'

# if too few command line args, check with the user if default arguments can be used
if len(sys.argv) < 4:
    print('### WARNING ###: trilepskim.py found too few command line arguments.')
    print('Normal usage from the command line:')
    print('python trileptonskim.py < input_directory > < sample_list > < output_directory >')
    print('       version=<ntuple version> filesperjob=< files_per_job > walltime=< wall_time >')
    print('(named arguments are optional)')
    print('Continue using only hard-coded arguments (e.g. for testing)? (y/n)')
    go = raw_input()
    if not go=='y':
        sys.exit()
    files_per_job = 1
    wall_time = '24:00:00'

# else, overwrite default arguments
else:
    input_directory = os.path.abspath(sys.argv[1])
    sample_list = os.path.abspath(sys.argv[2])
    output_directory_base = os.path.abspath(sys.argv[3])
    version_name = ''
    files_per_job = 50
    wall_time = '24:00:00'
    if len(sys.argv)>4:
        for sysarg in sys.argv[4:]:
            sysarg = sysarg.split('=')
            if sysarg[0]=='filesperjob': files_per_job = int(sysarg[1])
            elif sysarg[0]=='walltime': walltime = sysarg[1]
            elif sysarg[0]=='version': version_name = sysarg[1]
            else: 
                print('### ERROR ###: optional argument '+sysarg+' not recognized!')
                sys.exit()

# set skimmer directory
skimmer_directory = os.path.abspath('../../skimmer')

# check if ./skimmer executable exists
if not os.path.exists('../../skimmer/skimmer'):
    print('### ERROR ###: skimmer executable does not seem to exist.')
    print('Go to ewkino/skimmer and run make -f makeSkimmer.')
    sys.exit()

# check if sample list exists
if not os.path.exists(sample_list):
    print('### ERROR ###: sample list does not seem to exist.')
    sys.exit()
else:
    sample_list = os.path.abspath(sample_list)

# check if output directory is empty and ask permission to clean it
if os.path.exists(output_directory_base):
    if not len(os.listdir(output_directory_base))==0:
	print('### WARNING ###: output directory not empty. Clean it? (y/n)')
	go = raw_input()
	if not go=='y': sys.exit()
	os.system('rm -r {}'.format(os.path.join(output_directory_base,'*')))

# make a list of sample names to use
samples_to_use = []
useall = False
samples_dict = readsamplelist(sample_list)
for sample in samples_dict:
    samples_to_use.append(sample['sample_name'])
if(len(samples_to_use)==1 and samples_to_use[0]=='all'):
	useall = True

print('extracted the following sample names from the sample list:')
print(samples_to_use)

# make a list of samples (the main directories) 
# and an equally long list of subdirectories with the latest version of the ntuples 
# (one for each main directory)
sample_directories = []
sample_sub_directories = []
# first case where version name is given as fixed argument for all samples
if not version_name=='':
    for sample_directory, subdirectory in listSampleDirectories( input_directory, version_name ):
        # extra selection using samplelist:
        sample_name = sample_directory.rstrip( os.path.sep ).split( os.path.sep )[-1]
        if(useall or sample_name in samples_to_use):
	    print(sample_directory)
            sample_directories.append( sample_directory )
            sample_sub_directories.append( subdirectory )
# now case where version name is extracted from the sample list
else:
    for sample in samples_dict:
        if os.path.exists(input_directory+'/'+sample['sample_name']+'/'+sample['version_name']):
            sample_directories.append(input_directory+'/'+sample['sample_name'])
            sample_sub_directories.append(sample['version_name'])
        else:
            print('### WARNING ###: sample '+sample['sample_name']+'/'+sample['version_name']
		    +' not found')
            print('Continue without? (y/n)')
            go = raw_input()
            if not go=='y': sys.exit()

print('found '+str(len(sample_directories))+' valid sample directories.')

print(sample_directories) # for testing
print(sample_sub_directories) # for testing
#sys.exit() # for testing

# make output directory for each sample
sample_output_directories = []
for directory, subdirectory in zip(sample_directories, sample_sub_directories):
    sample_name = directory.rstrip( os.path.sep ).split( os.path.sep )[-1] 
    sample_name += '_' + yearIdentifierFromPath( subdirectory )
    output_directory = os.path.join( output_directory_base, 'ntuples_skimmed_{}_version_{}'
				    .format( sample_name, subdirectory ) )
    if not os.path.exists( output_directory ):
        os.makedirs( output_directory )
    sample_output_directories.append( output_directory )

for sample_directory, sub_directory, output_directory in zip( sample_directories, sample_sub_directories, sample_output_directories ):
    #identify locations of files to process for a given sample 
    root_files = list( listFiles( os.path.join( sample_directory, sub_directory ), '.root' ) )
    print('----------------------------')
    print('processing the following sample:')
    print(sample_directory)
    print(sub_directory)
    print('number of root files: '+str(len(root_files)))
    # split files in lists of files_per_job
    chunks = list(listParts( root_files, files_per_job ))
    #chunks = chunks[:int(len(chunks)/2)] # TEMPORARY TO PROCESS ONLY PART OF HUGE SAMPLE
    for chunk in chunks:
	commands = []
	commands.append( 'cd {}'.format( skimmer_directory ) )
	for f in chunk:
	    command = './skimmer {} {} {}'.format(f,output_directory,skim_condition)
	    commands.append(command)
        # old qsub way
        script_name = 'trileptonskim.sh'
        with open( script_name, 'w') as script:
            initializeJobScript( script )
            for command in commands:
                script.write( command+'\n' )
        # submit job and catch errors 
        #submitQsubJob( script_name, wall_time )
        # alternative: run locally (for testing and debugging)
        #os.system('bash '+script_name)
	# new condor way
	ct.submitCommandsAsCondorJob( 'cjob_trileptonskim', commands )
