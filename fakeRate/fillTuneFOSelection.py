#################################################
# a python submitter for fillTuneFOSelection.cc #
#################################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

flavours = ['muon']
years = ['2016']
samplelistdirectory = os.path.abspath('sampleListsNew')
# (see also below in loop to set the correct sample list name per flavour/year!)
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate'

# check if executable exists
if not os.path.exists('./fillTuneFOSelection'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillTuneFOSelection first.')
    sys.exit()

# loop and submit jobs
cwd = os.getcwd()
for flavour in flavours:
    for year in years:
	script_name = 'fillTuneFOSelection.sh'
	samplelist = os.path.join(samplelistdirectory,
			'samples_tunefoselection_'+flavour+'_'+year+'.txt')
	# check number of samples
	nsamples = 0
	with open(samplelist) as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		nsamples += 1
	print('found '+str(nsamples)+' samples for '+year+' '+flavour+'s.')
	for i in range(nsamples):
	    with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './fillTuneFOSelection {} {} {} {} {}'.format(
			    flavour,year,sampledirectory,samplelist,i)
		script.write(command+'\n')
	    submitQsubJob(script_name)
	    # alternative: run locally
	    #os.system('bash '+script_name)
