##########################################
# a python looper for plotMagicFactor.cc #
##########################################
import os
import sys
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

flavours = ['electron','muon']
years = ['2016','2017','2018']

# check if executable exists
if not os.path.exists('./plotTuneFOSelection'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makePlotTuneFOSelection first.')
    sys.exit()

# hadd files if needed
for flavour in flavours:
    for year in years:
	filename = 'tuneFOSelection_'+flavour+'_'+year+'_histograms.root'
	if os.path.exists(filename): continue
	subfolder = 'TuneFOSelectionSubFiles'
	cmd = 'hadd '+filename+' '+subfolder
	cmd += '/tuneFOSelection_'+flavour+'_'+year+'_histograms_sample*.root'
	#print(cmd)
	os.system(cmd)

cwd = os.getcwd()
for flavour in flavours:
    for year in years:
	# check file
	filename  = 'tuneFOSelection_'+flavour+'_'+year+'_histograms.root'
	if not os.path.exists(filename):
	    print('### ERROR ###: file '+filename+' not found, skipping it.')
	    continue
	script_name = 'plotTuneFOSelection.sh'
	with open(script_name,'w') as script:
            initializeJobScript(script)
            script.write('cd {}\n'.format(cwd))
            command = './plotTuneFOSelection {} {}'.format(flavour,year)
            script.write(command+'\n')
        submitQsubJob(script_name)
        # alternative: run locally
        #os.system('bash '+script_name)

