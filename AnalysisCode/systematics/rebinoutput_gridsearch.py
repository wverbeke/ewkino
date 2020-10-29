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

firstBinCounts = [0.5,1,2,3,4,5,10]
binFactors = [1.1,1.3,1.5,2,3,4]
topdir = os.path.abspath('output_tzqid_copyforrebinning')
logfile = os.path.abspath('rebinoutput_gridsearch.txt')

cwd  = os.getcwd()
script_name = 'rebinoutput_gridsearch.sh'
with open(script_name,'w') as script:
    initializeJobScript(script)
    for firstBinCount in firstBinCounts:
	for binFactor in binFactors:
	    script.write('cd {}\n'.format(cwd))
	    rebincommand = 'python rebinoutput.py '+topdir+' '+str(firstBinCount)+' '+str(binFactor)
	    script.write(rebincommand+'\n')
	    script.write('cd ../combine\n')
	    makecardcommand = 'python makedatacards.py'
	    script.write(makecardcommand+'\n')
	    runcombinecommand = 'python runcombine.py'
	    script.write(runcombinecommand+'\n')
	    script.write('echo "firstBinCount: {}, binFactor: {}" >> {}'.format(firstBinCount,binFactor,logfile)+'\n')
	    readoutputcommand = 'python readoutput.py datacards_newbins >> '+logfile
	    script.write(readoutputcommand+'\n')
submitQsubJob(script_name)
#os.system('bash '+script_name)
