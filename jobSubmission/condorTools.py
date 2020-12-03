###################################################################################
# functionality for making job description files for the condor submission system #
###################################################################################
# documentation: 
# - T2B wiki: https://t2bwiki.iihe.ac.be/HTCondor
# - HTCondor manual: https://htcondor.readthedocs.io/en/latest/users-manual/index.html

# general use:
# two ingredients are needed for a condor job:
# - a job description (.txt) file
# - an executable
# the functions in this tool allow creating an executable bash script (similar as for qsub)
# and its submission via a job description file

import os
import sys

def initJobScript(name, cmssw_version='CMSSW_10_2_16_patch1'):
    ### initialize an executable bash script by setting correct cms env
    ### note: similar to ewkino/skimmer/jobSubmission.py/initializeJobScript
    ### but copied here to be more standalone
    # parse argument
    name = name.split('.')[0]
    fname = name+'.sh'
    if os.path.exists(fname): os.system('rm {}'.format(fname))
    cwd = os.path.abspath(os.getcwd())
    # write script
    with open(fname,'w') as script:
	script.write('#!/bin/bash\n')
	script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
	script.write('cd {}/src\n'.format( cmssw_version ) )
	script.write('eval `scram runtime -sh`\n')
	script.write('cd {}\n'.format( cwd ) )
    print('initJobScript created {}'.format(fname))

def makeJobDescription(name, exe, argstring=None, stdout=None, stderr=None, log=None,
			cpus=1, mem=1024, disk=10240):
    ### create a single job description txt file
    ### note: exe can for example be a runnable bash script
    ### note: argstring is a single string containing the arguments to exe (space-separated)
    # parse arguments:
    name = name.split('.')[0]
    fname = name+'.txt'
    if os.path.exists(fname):
	os.system('rm {}'.format(fname))
    if stdout is None: stdout = name+'_out_$(ClusterId)_$(ProcId)'
    if stderr is None: stderr = name+'_err_$(ClusterId)_$(ProcId)'
    if log is None: log = name+'_log_$(ClusterId)_$(ProcId)'
    # write file
    with open(fname,'w') as f:
	f.write('executable = {}\n'.format(exe))
	if argstring is not None: f.write('arguments = "{}"\n\n'.format(argstring))
	f.write('output = {}\n'.format(stdout))
	f.write('error = {}\n'.format(stderr))
	f.write('log = {}\n\n'.format(log))
	f.write('request_cpus = {}\n'.format(cpus))
	f.write('request_memory = {}\n'.format(mem))
	f.write('request_disk = {}\n\n'.format(disk))
	#f.write('should_transfer_files = yes\n\n') 
	# (not fully sure whether to put 'yes', 'no' or omit it completely)
	f.write('queue\n\n')
    print('makeJobDescription created {}'.format(fname))

def submitCondorJob(jobDescription):
    ### submit a job description file as a condor job
    fname = jobDescription.split('.')[0]+'.txt'
    if not os.path.exists(fname):
	print('### ERROR ###: job description file {} not found'.format(fname))
	sys.exit()
    # maybe later extend this part to account for failed submissions etc!
    os.system('condor_submit {}'.format(fname))

def submitCommandAsCondorJob(name, command, stdout=None, stderr=None, log=None,
                        cpus=1, mem=1024, disk=10240, 
			cmssw_version='CMSSW_10_2_16_patch1'):
    ### combination of functions above to submit a single command as a single job
    # parse arguments
    name = name.split('.')[0]
    shname = name+'.sh'
    jdname = name+'.txt'
    [exe,argstring] = command.split(' ',1)
    nargs = len(argstring.split(' '))
    # first make the executable
    initJobScript(name, cmssw_version=cmssw_version)
    with open(shname,'a') as script:
        script.write(exe)
	for i in range(nargs): script.write(' ${}'.format(i+1))
	script.write('\n')
    # then make the job description
    makeJobDescription(name,shname,argstring=argstring,stdout=stdout,stderr=stderr,log=log,
                            cpus=cpus,mem=mem,disk=disk)
    # finally submit the job
    submitCondorJob(jdname)

def submitCommandsAsCondorCluster(name, commands, stdout=None, stderr=None, log=None,
                        cpus=1, mem=1024, disk=10240,
                        cmssw_version='CMSSW_10_2_16_patch1'):
    ### combination of functions above to submit a series of jobs in a single cluster
    ### (equivalent to running submitCommandAsCondorJob for each command but more efficient)
    ### note: each command must have the same executable, only args can differ!
    # parse arguments
    name = name.split('.')[0]
    shname = name+'.sh'
    jdname = name+'.txt'
    [exe,argstring] = commands[0].split(' ',1) # exe must be the same for all commands
    nargs = len(argstring.split(' ')) # nargs must be the same for all commands
    # first make the executable
    initJobScript(name, cmssw_version=cmssw_version)
    with open(shname,'a') as script:
        script.write(exe)
        for i in range(nargs): script.write(' ${}'.format(i+1))
        script.write('\n')
    # then make the job description
    # first job:
    makeJobDescription(name,shname,argstring=argstring,stdout=stdout,stderr=stderr,log=log,
                            cpus=cpus,mem=mem,disk=disk)
    # add other jobs:
    with open(jdname,'a') as script:
	for command in commands[1:]:
	    [thisexe,thisargstring] = command.split(' ',1)
	    thisnargs = len(thisargstring.split(' '))
	    if( thisexe!=exe or thisnargs!=nargs):
		print('### ERROR ###: commands are not compatible to put in same cluster')
		return
	    script.write('arguments = "{}"\n'.format(thisargstring))
	    script.write('queue\n\n')

    # finally submit the job
    submitCondorJob(jdname)
