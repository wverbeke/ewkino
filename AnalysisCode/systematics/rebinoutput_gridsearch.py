#################################################################
# run the analysis chain starting from mergoutput to runcombine #
# in a grid scan over rebin parameters			        #
#################################################################
# because of implementation issues, easiest to run sequentially.
# make sure all subprocesses are set to local running instead of job submission!
# (and run e.g. during the night...)

import os 
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

gridmode = 'parallel' 
# (choose from 'parallel' or 'sequential',
#  note that parallel might require a lot of memory
#  as the entire directory is copied for each gridpoint!)
mode = 'signal'
firstBinCounts = ['0.1f']
binFactors = [1.]
#mode = 'background'
#firstBinCounts = [0.5,1,1.5,2]
#binFactors = [1.1,1.3,1.5,2]
topdir = os.path.abspath('output_tzqidmedium0p4_new')
logdir = os.path.abspath('rebinoutput_gridsearch_log')

# parse arguments
if os.path.exists(logdir):
    print('WARNING: log directory already exists. Continue?')
    go = raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+logdir)
os.makedirs(logdir)

if( gridmode!='parallel' and gridmode!='sequential' ):
    print('### ERROR ###: gridmode not recognized')
    sys.exit()

cwd  = os.getcwd()
commands = []

counter = 0
for firstBinCount in firstBinCounts:
    for binFactor in binFactors:
	thiscommands = []
	thiscommands.append('cd {}'.format(cwd))
	# set up log file for this working point
	logfilename = 'log.txt'
	if gridmode=='parallel': logfilename = logfilename.replace('.txt','_{}.txt'.format(counter))
	logfile = os.path.join(logdir,logfilename)
	# copy topdir to a new topdir (only in case of parallel processing)
	thistopdir = topdir
	if gridmode=='parallel':
	    thistopdir = topdir+'_rebin_gridsearch_{}'.format(counter)
	    cpcommand = 'cp -r {} {}'.format(topdir,thistopdir)
	    thiscommands.append('echo "running copy command..."')
	    thiscommands.append(cpcommand)
	# make the rebin command and re-merge command
	rebincommand = 'python rebinoutput.py '+thistopdir+' '+mode+' '
	rebincommand += str(firstBinCount)+' '+str(binFactor)
	thiscommands.append('echo "running rebinoutput.py command"')
	thiscommands.append(rebincommand)
	mergecommand = 'python mergeoutput.py '+thistopdir
	thiscommands.append('echo "running mergeoutput.py command"')
	thiscommands.append(mergecommand)
	# make the combine commands
	thiscommands.append('cd ../combine')
	datacarddir = 'datacards_gridsearch'
	if gridmode=='parallel': datacarddir += '_{}'.format(counter)
	makecardcommand = 'python makedatacards.py {} {}'.format(thistopdir,datacarddir)
	thiscommands.append('echo "running makedatacards.py command"')
	thiscommands.append(makecardcommand)
	runcombinecommand = 'python runcombine.py {} runlocal'.format(datacarddir)
	thiscommands.append('echo "running runcombine.py command"')
	thiscommands.append(runcombinecommand)
	thiscommands.append('echo "firstBinCount: {}, binFactor: {}" >> {}'
			    .format(firstBinCount,binFactor,logfile))
	readoutputcommand = 'python readoutput.py {} >> {}'.format(datacarddir,logfile)
	thiscommands.append(readoutputcommand)
	# in case of parallel mode, submit thiscommands as a job
	if gridmode=='parallel': 
	    print(thiscommands)
	    ct.submitCommandsAsCondorJob('cjob_rebinoutput_gridsearch',thiscommands)
	# else add commands to all commands
	elif gridmode=='sequential': 
	    for cmd in thiscommands: commands.append(cmd)
	counter += 1

if gridmode=='sequential':
    ct.submitCommandsAsCondorJob('cjob_rebinoutput_gridsearch',commands)

# old qsub way: NOT YET ADAPTED TO BOTH PARALLEL AND SEQUENTIAL GRID MODE
#script_name = 'rebinoutput_gridsearch.sh'
#with open(script_name,'w') as script:
#    initializeJobScript(script)
#    for command in commands: script.write(command+'\n')
#submitQsubJob(script_name)
#os.system('bash '+script_name)
