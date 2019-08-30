
#import python library classes
import os
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
