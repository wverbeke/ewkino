
#include python library classes
import subprocess
import time
import os

#submit script of given name as a job with given wall-time ( Copied from my DeepLearning repository, consider making a submodule for jobsubmission )
def submitQsubJob( script_name, wall_time = '24:00:00', num_threads = 1, high_memory = False):

    #keep attempting submission until it succeeds
    submission_command = 'qsub {0} -l walltime={1}'.format( script_name, wall_time )

    if num_threads > 1:
        submission_command += ' -lnodes=1:ppn={0}'.format(num_threads)

    if high_memory :
        submission_command += ' -q highmem'

    while True:
        try:
	    # for python 2.7 and above
            qsub_output = subprocess.check_output( submission_command, shell=True, stderr=subprocess.STDOUT )
	    # else:
	    #subprocess.Popen( submission_command, shell=True, stderr=subprocess.STDOUT )
	    #qsub_output = ''

        #submission failed, try again after one second 
        except subprocess.CalledProcessError as error:
            print('Caught error : "{0}".\t Attempting resubmission.'.format( error.output.split('\n')[0] ) )
            time.sleep( 1 )

        #submission succeeded 
        else:
            first_line = qsub_output.split('\n')[0]
            print( first_line )

            #break loop by returning job id when submission was successful 
            return first_line.split('.')[0]


def initializeJobScript( script, docmsenv=True, cmssw_version = 'CMSSW_10_2_16_patch1' ):

    working_directory = os.path.abspath( os.getcwd() )
    script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
    if docmsenv:
	script.write('cd ~/{0}/src\n'.format( cmssw_version ) )
	script.write('eval `scram runtime -sh`\n')
    script.write('cd {0}\n'.format( working_directory ) )

def submitCommandsAsQsubJob( commands, script_name, wall_time = '24:00:00', 
				num_threads = 1, high_memory = False, 
				docmsenv=True, cmssw_version = 'CMSSW_10_2_16_patch1' ):
    with open( script_name, 'w' ) as script:
        initializeJobScript( script, docmsenv=docmsenv, cmssw_version=cmssw_version )
        for c in commands: script.write( c + '\n' )
    submitQsubJob( script_name, wall_time, num_threads, high_memory )
