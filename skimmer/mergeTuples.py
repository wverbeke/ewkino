#########################################
# merge tuples into one file per sample #
#########################################
# note: this functionality is supposed to be run on the output of skimTuples.py 
#       or skimTuplesFromList.py (important to extract sample name from folder name)
# note: the merging is done using simple hadd; it results in one file per sample / primary dataset.
#       for merging primary datasets, a more involved procedure should be applied
#	(see mergeAndRemoveOverlap)

# import python library classes 
import os
import sys

# import other parts of code 
from jobSubmission import submitQsubJob, initializeJobScript
from fileListing import walkLimitedDepth, listSampleDirectories
sys.path.append(os.path.abspath('../jobSubmission'))
import condorTools as ct


# help functions

def listSkimmedSampleDirectories( input_directory ):
    # list all directories containing unmerged skimmed ntuples
    # note: naming convention depends on skimTuples.py / skimTuplesFromList.py
    for _, sample in listSampleDirectories( input_directory, 'ntuples_skimmed' ):
        yield  sample


def sampleNameIsData( sample_name ):
    # check if a sample name is data (as opposed to simulation).
    # it is checked whether the sample name starts with the name of a primary dataset.
    tags = ['SingleElectron', 'SingleMuon', 'EGamma',
	    'DoubleEG', 'DoubleMuon',
	    'MuonEG',
	    'JetHT', 'MET', 'HTMHT']
    for tag in tags: 
	if sample_name.startswith(tag): return True
    return False


def sampleName( directory_name ):
    # extract sample name
    # note: naming convention depends on skimTuples.py / skimTuplesFromList.py
    # for simulation, the part between "ntuples_skimmed_" and "_version_" is used as sample name;
    # for data, it is extended by an era identifier. 
    sample_name = directory_name.split( 'ntuples_skimmed_' )[-1]
    [sample_name, version] = sample_name.split( '_version_' )
    # in case of simulation: return current sample name
    if not sampleNameIsData(sample_name): return sample_name
    # in case of data: append era identifier
    idx = version.find('Run')
    era = version[idx:idx+8]
    sample_name = sample_name+'_'+era
    return sample_name


def mergeCommand( sample_directory, output_file ):
    # make the merge command (i.e. simple hadd command)
    # input arguments:
    # - sample_directory: directory containing root files for one sample
    #                     (the files are assumed to be one level down)
    # - output_file: path to the output root file
    output_directory = os.path.dirname( output_file )
    if not os.path.exists( output_directory ):
        os.makedirs( output_directory )
    mergeCommand = 'hadd {} {}'.format( output_file, '{}/*root'.format( sample_directory ) )
    return mergeCommand 


def mergeSample( sample_directory, output_file, runmode='condor', rmunmerged=False ):
    # submit a merging job for one sample
    # input arguments:
    # - sample_directory: directory containing root files for one sample
    #                     (the files are assumed to be one level down)
    # - output_file: path to the output root file
    # - runmode: either "condor", "qsub" or "local"
    # - rmunmerged: remove the unmerged sample directory
    cmds = []
    cmds.append( mergeCommand( sample_directory, output_file ) )
    if rmunmerged: cmds.append( 'rm -r {}'.format( sample_directory ) )
    if runmode=='local':
	for cmd in cmds: os.system(cmd)
    elif runmode=='qsub':
	script_name = 'qjob_mergedTuples.sh'
	with open( script_name, 'w' ) as script:
	    initializeJobScript( script )
	    for cmd in cmds: script.write( cmd )
	submitQsubJob( script_name )
    elif runmode=='condor':
	ct.submitCommandsAsCondorJob( 'cjob_mergeTuples', cmds )

if __name__ == '__main__':

    # read command line arguments
    if len( sys.argv ) != 3:
        print( 'Error: mergeTuples requires additional command-line arguments.' )
        print( 'Use with the following arguments:') 
	print( '- input directory' )
	print( '- output_directory' )
        print( 'where the input directory should be the directory'
		+' which contains the unmerged samples as subdirectories' )
        sys.exit()
    input_directory = sys.argv[1]
    output_directory = sys.argv[2]

    # other arguments (hard-coded for now)
    runmode = 'condor'
    rmunmerged = False

    # define the samples to merge
    inputdirs = []
    outputfiles = []
    for sample_directory in listSkimmedSampleDirectories( input_directory ):
	inputdir = os.path.join( input_directory, sample_directory )
	outputfile = os.path.join( output_directory, sampleName(sample_directory)+'.root' )
	inputdirs.append( inputdir )
	outputfiles.append( outputfile )

    # print before continuing
    print('found following tuples to merge:')
    for inputdir, outputfile in zip(inputdirs, outputfiles):
	print('{} --> {}'.format(inputdir,outputfile))
    print('continue? (y/n)')
    go = raw_input()
    if go!='y': sys.exit()

    # continue with the submission
    for inputdir, outputfile in zip(inputdirs, outputfiles):
        mergeSample( inputdir, outputfile,
			runmode=runmode, rmunmerged=rmunmerged )
