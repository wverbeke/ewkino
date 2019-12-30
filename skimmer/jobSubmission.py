
#include python library classes
import subprocess
import time

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


def initializeJobScript( script ):

   	#TO DO : make code to extract CMSSW directory in a general way
    #this is already available in DeepLearning repository, submodule would be a good solution 
    current_CMSSW_version = 'CMSSW_10_2_20'
    script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
    script.write('cd {}/src\n'.format( current_CMSSW_version ) )
    script.write('eval `scram runtime -sh`\n')
