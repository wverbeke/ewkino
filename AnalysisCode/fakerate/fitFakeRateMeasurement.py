######################################################################
# a python looper over years and flavours to determine the fake rate #
######################################################################
import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

years = ['2016','2017','2018']
flavours = ['muon','electron']
use_mT = True
allowedmethods = ['subtraction','templatefit']
method = allowedmethods[1] # choose a valid index from the list above

# hadd files if needed
for year in years:
    for flavour in flavours:
	basename = 'fakeRateMeasurement_data_'+flavour+'_'+year
	if use_mT: basename += '_mT'
	else: basename += '_met'
	basename += '_histograms'
	filename = basename + '.root'
	if os.path.exists(filename): continue
	subfolder = 'FakeRateMeasurementSubFiles'
	cmd = 'hadd '+filename+' '
	cmd += os.path.join(subfolder,basename+'_sample*.root')
	print(cmd)
	os.system(cmd)

cwd = os.getcwd()
commands = []
for year in years:
    for flavour in flavours:
	# check file
	basename = 'fakeRateMeasurement_data_'+flavour+'_'+year
        if use_mT: basename += '_mT'
        else: basename += '_met'
        basename += '_histograms'
        filename = basename + '.root'
	if not os.path.exists(filename):
	    print('### ERROR ###: file '+filename+' not found, skipping it.')
	    continue
	command = ''
	### method 1: simple prompt subtraction
	if method=='subtraction':
	    # check if executable exists
	    if not os.path.exists('./fitFakeRateMeasurement'):
		print('### ERROR ###: executable does not seem to exist...')
		print('               run make -f makeFitFakeRateMeasurement first.')
		sys.exit()
	    command = './fitFakeRateMeasurement {} {} {}'.format(str(use_mT),flavour,year)
	    os.system(command)
	### method 2: template fits using combine
	elif method=='templatefit':
	    # check if plotting executable exists
	    if not os.path.exists('./plotHistogramsInFile'):
		print('### ERROR ###: executable does not seem to exist...')
                print('               run make -f makePlotHistogramsInFile first.')
                sys.exit()
	    # set and make output folders
	    fitplotdir = 'fakeRateMeasurementPlots'
	    if not os.path.exists(fitplotdir): os.makedirs(fitplotdir)
	    frmapdir = 'fakeRateMaps'
	    if not os.path.exists(frmapdir): os.makedirs(frmapdir)
	    var = 'mT' if use_mT else 'met'
	    command = 'python fitTemplates.py {} {} {} {} {}'.format(var,flavour,year,
								fitplotdir,frmapdir)
	    commands.append(command)
	    # old qsub way:
	    #script_name = 'fitFakeRateMeasurement.sh'
            #with open(script_name,'w') as script:
            #    initializeJobScript(script)
            #    script.write(command+'\n')
            #submitQsubJob(script_name) 
	    # for testing: run locally
	    #os.system('bash '+script_name)
	else:
	    print('### ERROR ###: method '+method+' not recognized')
	    continue
# new condor way:
ct.submitCommandsAsCondorCluster('fitFakeRateMeasurement_cjob',commands)
