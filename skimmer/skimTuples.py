
#import python library classes
import os
import subprocess
import time
import sys


#perform os.walk up to a specified depth
def walkLimitedDepth( input_directory, max_depth ):
	
	#remove trailing separators 
	input_directory = input_directory.rstrip( os.path.sep )

	#depth of input directory 
	base_depth = input_directory.count( os.path.sep )

	for directory, subdirectories, files in os.walk( input_directory ):
		yield directory, subdirectories, files
		current_depth = directory.count( os.path.sep )

        #the difference in the depth of the input directory and current directory is the depth of the walk
		if ( current_depth - base_depth ) >= max_depth :
			del subdirectories[:]


#given the directory containing the output of all crab jobs, list the individual samples that contain a certain name (for instance version indicator)
def listSampleDirectories( input_directory, name_to_search ):
    for directory, subdirectories, files in walkLimitedDepth( input_directory, 1):
        for subdir in subdirectories:
            if name_to_search in subdir:
                yield directory, subdir


#list all files containing an identifier ( used to list all root files in a given sample's directory )
def listFiles( input_directory, identifier ):
    for directory, subdirectories, files in os.walk( input_directory ):
        for f in files:
            if identifier in f:
                yield os.path.join( directory, f )


def listParts( input_list, chunk_size ):
    for i in range( 0, len(input_list), chunk_size ):
        yield input_list[ i : i + chunk_size ]


#submit script of given name as a job with given wall-time ( Copied from my DeepLearning repository, consider making a submodule for jobsubmission )
def submitQsubJob( script_name, wall_time = '24:00:00', num_threads = 1, high_memory = False):

    #keep attempting submission until it succeeds
    submission_command = 'qsub {} -l walltime={}'.format( script_name, wall_time )

    if num_threads > 1:
        submission_command += ' -lnodes=1:ppn={}'.format(num_threads)

    if high_memory :
        submission_command += ' -q highmem'

    while True:
        try:
            qsub_output = subprocess.check_output( submission_command, shell=True, stderr=subprocess.STDOUT )

        #submission failed, try again after one second 
        except subprocess.CalledProcessError as error:
            print('Caught error : "{}".\t Attempting resubmission.'.format( error.output.split('\n')[0] ) )
            time.sleep( 1 )

        #submission succeeded 
        else:
            first_line = qsub_output.split('\n')[0]
            print( first_line )

            #break loop by returning job id when submission was successful 
            return first_line.split('.')[0]
 
 

if __name__ == '__main__' :

    #TO DO : make code to extract CMSSW directory in a general way
    #this is already available in DeepLearning repository, submodule would be a good solution
    current_CMSSW_version = 'CMSSW_10_2_16'
    
    input_directory = sys.argv[1] 
    print( input_directory )
    version_name = sys.argv[2] 
    output_directory_base = sys.argv[3] 
    skim_condition = sys.argv[4]
    files_per_job = int(sys.argv[5]) if len( sys.argv ) >= 6 else 50
    wall_time = sys.argv[6] if len( sys.argv ) >= 7 else '24:00:00' 
    if len( sys.argv ) <= 4:
    	print( 'Error: skimTuples.py requires additional command-line arguments.' )
        print( 'Usage: python skimTuples.py < input_directory > < ntuple_version > < output_directory > < skim_condition > < files_per_job > < wall_time >' )
        print( 'files_per_job and wall_time have default values of 50 and 24:00:00' )
    
    #make a list of samples (the main directories) and the subdirectories with the latest version of the ntuples ( one for each main directory )
    sample_directories = []
    sample_sub_directories = []
    for sample_directory, subdirectory in listSampleDirectories( input_directory, version_name ):
        sample_directories.append( sample_directory )
        sample_sub_directories.append( subdirectory )
    
    sample_names = [ directory.rstrip( os.path.sep ).split( os.path.sep )[-1] for directory in sample_directories ]
    for sample in sample_names :
        print( sample )
    
    #output directory for each sample
    sample_output_directories = []
    for sample in sample_names:
        output_directory = os.path.join( output_directory_base, 'ntuples_skimmed_{}_version_{}'.format( sample, version_name ) )
        if not os.path.exists( output_directory ):
            os.makedirs( output_directory )
        sample_output_directories.append( output_directory )
    
    
    for sample_directory, sub_directory, output_directory in zip( sample_directories, sample_sub_directories, sample_output_directories ):

        #identify locations of files to process for a given sample 
        root_files = list( listFiles( os.path.join( sample_directory, sub_directory ), '.root' ) )
        
        #split_files in lists of files_per_job
        for chunk in listParts( root_files, files_per_job ):
        
            #make a job script 
            script_name = 'skimmer.sh'
            with open( script_name, 'w') as script:
                script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
                script.write('cd {}/src\n'.format( current_CMSSW_version ) )
                script.write('eval `scram runtime -sh`\n') 
                script.write('cd {}\n'.format( output_directory ) ) 
                for f in chunk :
                    skim_command = './skimmer {} {} {}\n'.format( f, output_directory, skim_condition )
                    script.write( skim_command )
            
            #submit job and catch errors 
            submitQsubJob( script_name, wall_time )
