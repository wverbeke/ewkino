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

mode = 'signal'
firstBinCounts = ['0.1f','0.2f','0.3f','0.4f','0.5f']
binFactors = [1.,0.95,0.9,0.8,0.7]
#mode = 'background'
#firstBinCounts = [0.5,1,1.5,2]
#binFactors = [1.1,1.3,1.5,2]
topdir = os.path.abspath('output_tzqidmedium0p4')
logfile = os.path.abspath('rebinoutput_gridsearch.txt')

cwd  = os.getcwd()
commands = []

for firstBinCount in firstBinCounts:
    for binFactor in binFactors:
	commands.append('cd {}'.format(cwd))
	rebincommand = 'python rebinoutput.py '+topdir+' '+mode+' '
	rebincommand += str(firstBinCount)+' '+str(binFactor)
	commands.append(rebincommand)
	mergecommand = 'python mergeoutput.py '+topdir
	commands.append(mergecommand)
	commands.append('cd ../combine')
	makecardcommand = 'python makedatacards.py '+topdir+' datacards_gridsearch'
	commands.append(makecardcommand)
	runcombinecommand = 'python runcombine.py datacards_gridsearch runlocal'
	commands.append(runcombinecommand)
	commands.append('echo "firstBinCount: {}, binFactor: {}" >> {}'
	            .format(firstBinCount,binFactor,logfile))
	readoutputcommand = 'python readoutput.py datacards_gridsearch >> '+logfile
	commands.append(readoutputcommand)

# old qsub way:
#script_name = 'rebinoutput_gridsearch.sh'
#with open(script_name,'w') as script:
#    initializeJobScript(script)
#    for command in commands: script.write(command+'\n')
#submitQsubJob(script_name)
#os.system('bash '+script_name)

# new condor way:
ct.submitCommandsAsCondorJob('rebinoutput_gridsearch_cjob',commands)
