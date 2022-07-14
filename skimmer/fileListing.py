###################################
# tools for listing files to skim #
###################################

# import python library classes
import os
import sys

def walkLimitedDepth( input_directory, max_depth ):
    # perform os.walk up to a specified depth
	
    # remove trailing separators 
    input_directory = input_directory.rstrip( os.path.sep )
    # get depth of input directory 
    base_depth = input_directory.count( os.path.sep )
    for directory, subdirectories, files in os.walk( input_directory ):
	yield directory, subdirectories, files
	current_depth = directory.count( os.path.sep )
	# the difference in the depth of the input directory and current directory 
	# is the depth of the walk
	if ( current_depth - base_depth ) >= max_depth : del subdirectories[:]


def listSampleDirectories( input_directory, name_to_search ):
    # given the directory containing the output of all crab jobs, 
    # list the individual samples that contain a certain name (for instance version indicator)
    # note: the search extends only one level deep (usually the sample name level),
    #       but the name_to_search is only applied on one level lower (usually the version level)!
    for directory, subdirectories, files in walkLimitedDepth( input_directory, 1):
        for subdir in subdirectories:
            if name_to_search in subdir:
                yield directory, subdir


def listFiles( input_directory, identifier ):
    # list all files containing an identifier 
    # (used to list all root files in a given sample's directory)
    # note: the search extends arbitrarily deep,
    #       but the identifier is only applied on the file name!
    for directory, subdirectories, files in os.walk( input_directory ):
        for f in files:
            if identifier in f:
                yield os.path.join( directory, f )


def listSampleFiles( input_directory ):
    # more specific version of listFiles, used to list all root files in a sample directory.
    # the input directory should be the full path to a sample + version name!
    # this function checks that only one sample is present in the provided input directory,
    # i.e. that there is only one time stamp folder.
    
    # find time stamp folders (assumed to be one level below version name)
    timestamps = os.listdir(input_directory)
    if len(timestamps)!=1:
	msg = 'ERROR in fileListing.listSampleFiles:'
	msg += ' the sample in directory {}'.format(input_directory)
	msg += ' contains {} time stamp folders, while 1 was expected.'.format(len(timestamps))
	msg += ' please check.'
	raise Exception(msg)
    timestamp = timestamps[0]
    # find all root files in this folder
    return list(listFiles(os.path.join(input_directory,timestamp),'.root'))


def listParts( input_list, chunk_size ):
    # split a list into lists of fixed size
    # note: the last list might be smaller than chunk_size,
    #       if chunk_size is not a divisor of the original list length
    for i in range( 0, len(input_list), chunk_size ):
        yield input_list[ i : i + chunk_size ]
