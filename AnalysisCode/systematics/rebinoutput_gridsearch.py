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

mode = 'signal'
firstBinCounts = ['0.1f','0.2f','0.3f','0.4f','0.5f']
binFactors = [1.,0.95,0.9,0.8,0.7]
#mode = 'background'
#firstBinCounts = [0.5,1,1.5,2]
#binFactors = [1.1,1.3,1.5,2]
topdir = os.path.abspath('output_tzqid_newfoid')
logfile = os.path.abspath('rebinoutput_gridsearch.txt')

cwd  = os.getcwd()
script_name = 'rebinoutput_gridsearch.sh'
with open(script_name,'w') as script:
    initializeJobScript(script)
    for firstBinCount in firstBinCounts:
	for binFactor in binFactors:
	    script.write('cd {}\n'.format(cwd))
	    rebincommand = 'python rebinoutput.py '+topdir+' '+mode+' '
	    rebincommand += str(firstBinCount)+' '+str(binFactor)
	    script.write(rebincommand+'\n')
	    mergecommand = 'python mergeoutput.py '+topdir
	    script.write(mergecommand+'\n')
	    script.write('cd ../combine\n')
	    makecardcommand = 'python makedatacards.py '+topdir+' datacards_gridsearch'
	    script.write(makecardcommand+'\n')
	    runcombinecommand = 'python runcombine.py datacards_gridsearch runlocal'
	    script.write(runcombinecommand+'\n')
	    script.write('echo "firstBinCount: {}, binFactor: {}" >> {}'
			    .format(firstBinCount,binFactor,logfile)+'\n')
	    readoutputcommand = 'python readoutput.py datacards_gridsearch >> '+logfile
	    script.write(readoutputcommand+'\n')
submitQsubJob(script_name)
#os.system('bash '+script_name)
