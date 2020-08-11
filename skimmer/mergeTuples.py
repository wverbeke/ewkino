
#import python library classes 
import os
import sys

#import other parts of code 
from jobSubmission import submitQsubJob, initializeJobScript
from fileListing import walkLimitedDepth, listSampleDirectories


#list all directories containing unmerged skimmed ntuples
def listSkimmedSampleDirectories( input_directory ):
    for _, sample in listSampleDirectories( input_directory, 'ntuples_skimmed' ):
        yield  sample


#extract sample name
def sampleName( directory_name ):
    sample_name = directory_name.split( 'ntuples_skimmed_' )[-1]
    sample_name = sample_name.split( '_version_' )[0] 
    return sample_name


def mergeCommand( sample_directory, output_directory ):
    if not os.path.exists( output_directory ):
        os.makedirs( output_directory )
    
    output_path = os.path.join( output_directory, sampleName( sample_directory ) + '.root' )
    mergeCommand = 'hadd {} {}'.format( output_path, '{}/*root'.format( sample_directory ) )
    return mergeCommand 


def mergeSample( sample_directory, output_directory ):
    script_name = 'mergedTuples.sh'
    with open( script_name, 'w' ) as script:
        initializeJobScript( script )

        #merge sample
        script.write( '{}\n'.format( mergeCommand( sample_directory, output_directory ) ) )

        #remove unmerged directory
        script.write( 'rm -r {}\n'.format( sample_directory ) )
    
    submitQsubJob( script_name )


if __name__ == '__main__':

    if len( sys.argv ) != 3:
        print( 'Error : mergeTuples requires additional command-line arguments.' )
        print( 'Usage : python mergeTuples.py <input_directory> <output_directory>' )
        print( 'The input directory should be the directory which contains the unmerged samples as subdirectories' )
        sys.exit()


    input_directory = sys.argv[1]
    output_directory = sys.argv[2]
    for sample in listSkimmedSampleDirectories( input_directory ):
        mergeSample( os.path.join( input_directory, sample ), output_directory )
