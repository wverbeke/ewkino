###################
# skim all tuples #
###################
# note: The main function has not been used / updated since some time
#       (for example it is not updated to use condor instead of qsub),
#       since skimTuplesFromList.py was used instead, 
#       using a sample list instead of skimming all tuples in an input directory.
#       So it is not recommended to call this script directly,
#       but some of the functions defined here are reused in skimTuplesFromList.py.

# import python library classes
import os
import sys

# import other code from framework
from jobSubmission import submitQsubJob, initializeJobScript
from fileListing import *

# help functions
# note: these should be kept more or less synchronized with the ones in
#       Tools/src/analysisTools.cc, to avoid mismatching naming conventions! 

def is2017( sample_path ):
    tags = ['MiniAOD2017', # pre-UL simulation 
	    'Summer20UL17', # UL simulation
	    'Run2017' ] # data
    for tag in tags: 
	if tag in sample_path: return True
    return False

def is2018( sample_path ):
    tags = ['MiniAOD2018', # pre-UL simulation
	    'Summer20UL18', # UL simulation
	    'Run2018' ] # data
    for tag in tags:
	if tag in sample_path: return True
    return False

def is2016( sample_path ):
    # note: for simulation this is only true for files covering all of 2016 (legacy sim),
    #       ultra-legacy sim is split in 2016pre and 2016post;
    #       for data this is true if the era belongs to 2016 (both for legacy and ultra-legacy)
    tags = ['MiniAOD2016', # pre-UL simulation
	    'Run2016' ] # data
    for tag in tags:
	if tag in sample_path: return True
    return False

def is2016pre( sample_path ):
    # note: for simulation this is only true for ultra-legacy files from 2016 preVFP;
    #       for data this is true if the era belongs to 2016 B, C, D, E or F 
    #       (both for legacy and ultra-legacy) but note that is2016 is also true in those cases! 
    tags = ['Run2016B', 'Run2016C', 'Run2016D', 'Run2016E', 'Run2016F', # data
	    'Summer20UL16MiniAODAPV'] # UL simulation
    for tag in tags:
	if tag in sample_path: return True
    return False

def is2016post( sample_path ):
    # note: for simulation this is only true for ultra-legacy files from 2016 postVFP
    #       for data this is true if the era belongs to 2016 G or H 
    #       (both for legacy and ultra-legacy) but not that is2016 is also true in those cases!
    tags = ['Run2016G', 'Run2016H', # data
            'Summer20UL16MiniAOD'] # UL simulation
    # need to check 2016pre first since overlapping names for simulation
    if is2016pre( sample_path ): return False
    # now check tags
    for tag in tags:
        if tag in sample_path: return True
    return False

def yearIdentifierFromPath( sample_path ):
    # return a year identifier string based on a sample path
    # (used in naming output directories)
    # note on 2016: for simulation, this will give Summer16 for pre-UL, 
    #		    Summer16PreVFP for UL preVFP, and Summer16PostVFP for UL VFP.
    #		    for data, this will give Summer16 for all eras for pre-UL and UL.
    if is2018( sample_path ): return 'Autumn18'
    elif is2017( sample_path ): return 'Fall17'
    elif is2016( sample_path ): return 'Summer16'
    elif is2016pre( sample_path ): return 'Summer16PreVFP'
    elif is2016post( sample_path ): return 'Summer16PostVFP'
    else: raise Exception('ERROR: could not identify year from sample {}.'.format(sample_path)
		    +' Check the naming conventions in skimmer/skimTuples.py!')


if __name__ == '__main__' :

    #WARNING : it is assumed this script is run from the 'skimmer' directory
    current_directory = os.path.dirname( os.path.abspath( __file__ ) )
    
    # check amount of command line args
    if len( sys.argv ) <= 4:
        print( 'Error: too few command-line arguments. Use with following arguments:' )
        print( ' - input directory' )
        print( ' - ntuple version' )
        print( ' - output directory' )
        print( ' - skim condition' )
        print( ' - files per job (default 50)')
        print( ' - maximum wall time per job (default 24:00:00)' )
        sys.exit()

    # read command line args
    input_directory = sys.argv[1] 
    version_name = sys.argv[2] 
    output_directory_base = sys.argv[3] 
    skim_condition = sys.argv[4]
    files_per_job = int(sys.argv[5]) if len( sys.argv ) >= 6 else 50
    wall_time = sys.argv[6] if len( sys.argv ) >= 7 else '24:00:00' 
    
    # make a list of samples (the main directories) and the subdirectories 
    # with the latest version of the ntuples ( one for each main directory )
    sample_directories = []
    sample_sub_directories = []
    for sample_directory, subdirectory in listSampleDirectories( input_directory, version_name ):
        sample_directories.append( sample_directory )
        sample_sub_directories.append( subdirectory )
    
    # define output directory for each sample
    sample_output_directories = []
    for directory, subdirectory in zip( sample_directories, sample_sub_directories ):
	sample_name = directory.rstrip( os.path.sep ).split( os.path.sep )[-1] 
	sample_name += '_' + yearIdentifierFromPath( subdirectory )
        output_directory = os.path.join( output_directory_base, 
			    'ntuples_skimmed_{}_version_{}'.format( sample_name, version_name ) )
        if not os.path.exists( output_directory ):
            os.makedirs( output_directory )
        sample_output_directories.append( output_directory )
    
    
    for sample_directory, sub_directory, output_directory in zip( sample_directories, sample_sub_directories, sample_output_directories ):
    
        # identify locations of files to process for a given sample 
        root_files = list( listFiles( os.path.join( sample_directory, sub_directory ), '.root' ) )
        
        # split_files in lists of files_per_job
        for chunk in listParts( root_files, files_per_job ):
        
            # make a job script 
            script_name = 'qjob_skimmer.sh'
            with open( script_name, 'w') as script:
                initializeJobScript( script )
                script.write('cd {}\n'.format( current_directory ) ) 
                for f in chunk :
                    skim_command = './skimmer {} {} {}\n'.format( 
				    f, output_directory, skim_condition )
                    script.write( skim_command )
            
            # submit job and catch errors 
            submitQsubJob( script_name, wall_time )
