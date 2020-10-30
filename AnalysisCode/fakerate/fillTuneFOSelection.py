#################################################
# a python submitter for fillTuneFOSelection.cc #
#################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

flavours = ['muon','electron']
years = ['2016','2017','2018']

# check if executable exists
if not os.path.exists('./fillTuneFOSelection'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillTuneFOSelection first.')
    sys.exit()

cwd = os.getcwd()

for flavour in flavours:
    for year in years:
	script_name = 'fillTuneFOSelection.sh'
	# check number of samples
	nsamples = 0
	with open('../samplelists/samples_tunefoselection_'+flavour+'_'+year+'.txt') as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		nsamples += 1
	print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s.')
	for i in range(nsamples):
	    with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './fillTuneFOSelection {} {} {}'.format(flavour,year,i)
		script.write(command+'\n')
	    submitQsubJob(script_name)
	    # alternative: run locally
	    #os.system('bash '+script_name)