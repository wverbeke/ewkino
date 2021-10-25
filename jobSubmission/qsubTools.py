#############################
# job submission using qsub #
#############################

# include python library classes
import subprocess
import time
import os

def submitQsubJob( script_name, wall_time = '24:00:00', num_threads = 1, high_memory = False):
    ### submit a given script as a job

    # make the qsub command
    submission_command = 'qsub {0} -l walltime={1}'.format( script_name, wall_time )
    if num_threads > 1:
        submission_command += ' -lnodes=1:ppn={0}'.format(num_threads)
    if high_memory :
        submission_command += ' -q highmem'

    # try submission until command executes without errors
    while True:
        try:
	    # for python 2.7 and above
            qsub_output = subprocess.check_output( submission_command, 
			    shell=True, stderr=subprocess.STDOUT )
	    # else:
	    #subprocess.Popen( submission_command, shell=True, stderr=subprocess.STDOUT )
	    #qsub_output = ''
        except subprocess.CalledProcessError as error:
            print('Caught error : "{}".'.format( error.output.split('\n')[0] ) 
		    +'\t Attempting resubmission...')
            time.sleep( 1 )
        else:
            first_line = qsub_output.split('\n')[0]
            print( first_line )
            # break loop by returning job id when submission was successful 
            return first_line.split('.')[0]


def initializeJobScript( script, docmsenv=True, cmssw_version = 'CMSSW_10_2_16_patch1' ):
    ### initialize a job script
    working_directory = os.path.abspath( os.getcwd() )
    script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
    if docmsenv:
	script.write('cd ~/{0}/src\n'.format( cmssw_version ) )
	script.write('eval `scram runtime -sh`\n')
    script.write('cd {0}\n'.format( working_directory ) )

def submitCommandsAsQsubJob( commands, script_name, wall_time = '24:00:00', 
				num_threads = 1, high_memory = False, 
				docmsenv=True, cmssw_version = 'CMSSW_10_2_16_patch1' ):
    ### write a list of commands to a script and submit as a job
    with open( script_name, 'w' ) as script:
        initializeJobScript( script, docmsenv=docmsenv, cmssw_version=cmssw_version )
        for c in commands: script.write( c + '\n' )
    submitQsubJob( script_name, wall_time, num_threads, high_memory )
