##################################
# skim tuples from a sample list #
##################################
# functionality is similar to skimTuples.py, 
# but with usage of a sample list instead of automatically running over all samples in a directory.
# note: copied (with some updates and modifications) from tZq analysis code!

# command line arguments (in sequence):
# - input directory (e.g. /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino)
# - sample list (e.g. tzq_2016MC_samplelist.txt)
# - output directory (e.g. /storage_mnt/storage/user/llambrec/Files)
# - skim condition (see src/skimSelections.cc for valid options)
# - optional: ntuple version (e.g. singlelepton_MC_2016_v1) 
#             (overwrites ntuple version that may be present in the sample list!)
# - optional: files per job
# - optional: wall time per job

import sys
import os
from fileListing import listSampleDirectories, listFiles, listParts
from skimTuples import yearIdentifierFromPath
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct
sys.path.append(os.path.abspath('../Tools/python'))
from samplelisttools import readsamplelist

# check number of command line args
if len(sys.argv) < 5:
    print('### WARNING ###: too few command line arguments.')
    print('Usage the following arguments:')
    print('  - input directory')
    print('  - sample_list')
    print('  - output_directory')
    print('  - skim condition')
    print('  - version = ntuple version')
    print('  - filesperjob = number of files per job')
    print('  - walltime = maximum wall time')
    print('  - runmode = run mode (condor, qsub or local)')
    print('(named arguments are optional)')
    sys.exit()

# read required command line args
input_directory = os.path.abspath(sys.argv[1])
sample_list = os.path.abspath(sys.argv[2])
output_directory_base = os.path.abspath(sys.argv[3])
skim_condition = sys.argv[4]

# read optional command line args
version_name = ''
files_per_job = 50
wall_time = '24:00:00'
runmode = 'condor'
if len(sys.argv)>5:
    for sysarg in sys.argv[5:]:
        sysarg = sysarg.split('=')
        if sysarg[0]=='filesperjob': files_per_job = int(sysarg[1])
        elif sysarg[0]=='walltime': wall_time = sysarg[1]
        elif sysarg[0]=='version': version_name = sysarg[1]
	elif sysarg[0]=='runmode': runmode = sysarg[1]
        else: 
            raise Exception('ERROR: optional argument '+sysarg+' not recognized!')

# check if executable exists
exe = './skimmer'
if not os.path.exists(exe):
    raise Exception('ERROR: executable {} does not exist.'.format(exe))

# check if input directory exists
if not os.path.exists(input_directory):
    raise Exception('ERROR: input directory {} does not exist.'.format(input_directory))

# check if sample list exist
if not os.path.exists(sample_list):
    raise Exception('ERROR: sample list {} does not exist.'.format(sample_list))

# check skim condition
allowed_skim_conditions = ["noskim", "singlelepton", "dilepton", 
			    "trilepton", "fourlepton", "fakerate"]
if skim_condition not in allowed_skim_conditions:
    raise Exception('ERROR: skim condition {} not valid. Options are {}'.format(
			skim_condition, allowed_skim_conditions))

# check if output directory is empty and ask permission to clean it
if os.path.exists(output_directory_base):
    if not len(os.listdir(output_directory_base))==0:
	print('WARNING: output directory not empty. Clean it? (y/n)')
	go = raw_input()
	if not go=='y': sys.exit()
	os.system('rm -r {}'.format(os.path.join(output_directory_base,'*')))
else: os.makedirs(output_directory_base)

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
# case 1: version name is given as fixed argument for all samples
if version_name!='':
    for sample_directory, subdirectory in listSampleDirectories( input_directory, version_name ):
        # extra selection using samplelist:
        sample_name = sample_directory.rstrip( os.path.sep ).split( os.path.sep )[-1]
        if(useall or sample_name in samples_to_use):
            sample_directories.append( sample_directory )
            sample_sub_directories.append( subdirectory )
# case 2: version name is extracted from the sample list
else:
    for sample in samples_dict:
        if os.path.exists(input_directory+'/'+sample['sample_name']+'/'+sample['version_name']):
            sample_directories.append(input_directory+'/'+sample['sample_name'])
            sample_sub_directories.append(sample['version_name'])
        else:
            print('WARNING: sample {} not found in input directory {}'.format(
		    sample['sample_name']+'/'+sample['version_name'], input_directory))
            print('Continue without? (y/n)')
            go = raw_input()
            if not go=='y': sys.exit()

# printouts for testing
print('found {} valid sample directories.'.format(len(sample_directories)))
for directory,subdirectory in zip(sample_directories,sample_sub_directories):
    print('  - {}/{}'.format(directory,subdirectory))

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

# loop over samples and submit skimming jobs
cwd = os.getcwd()
itlist = zip(sample_directories, sample_sub_directories, sample_output_directories)
for sample_directory, sub_directory, output_directory in itlist:
    #identify locations of files to process for a given sample 
    root_files = list( listFiles( os.path.join( sample_directory, sub_directory ), '.root' ) )
    print('----------------------------')
    print('processing the following sample:')
    print(sample_directory)
    print(sub_directory)
    print('number of root files: {}'.format(len(root_files)))
    # split files in lists of files_per_job
    chunks = list(listParts( root_files, files_per_job ))
    for chunk in chunks:
	commands = []
	commands.append( 'cd {}'.format(cwd) )
	for f in chunk:
	    command = './skimmer {} {} {}'.format(f,output_directory,skim_condition)
	    commands.append(command)
        # old qsub way
        script_name = 'qjob_skimTuplesFromList.sh'
        with open( script_name, 'w') as script:
            initializeJobScript( script )
            for command in commands:
                script.write( command+'\n' )
        # submit job and catch errors 
        if runmode=='qsub': submitQsubJob( script_name, wall_time )
        # alternative: run locally (for testing and debugging)
        if runmode=='local': os.system('bash '+script_name)
	# new condor way
	if runmode=='condor': ct.submitCommandsAsCondorJob( 'cjob_skimTuplesFromList', commands )
