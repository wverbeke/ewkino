#############################################
# a python submitter for fillMagicFactor.cc #
#############################################
import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# set global properties
flavours = ['muon','electron']
leptonMVA = 'leptonMVATOP'
mvathresholds = [0.9,0.9] # must be of same length as flavours
years = ['2016','2017','2018']
samplelist = os.path.abspath('sampleListsPreUL/samples_magicfactor.txt')
sampledirectory = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate/'

# check value of leptonMVA
if not (leptonMVA=='leptonMVAttH' or leptonMVA=='leptonMVAtZq' or leptonMVA=='leptonMVATOP'):
    print('### ERROR ###: choice of leptonMVA not recognized.')
    sys.exit()
# check if executable exists
if not os.path.exists('./fillMagicFactor'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeFillClosureTest first.')
    sys.exit()

# loop and submit jobs
cwd = os.getcwd()
for flavour,mvathreshold in zip(flavours,mvathresholds):
    for year in years:
	script_name = 'fillMagicFactor.sh'
	# check number of samples
	thisyearindices = []
	samplecounter = 0
	with open(samplelist) as sf:
	    for sl in sf:
		if(sl[0] == '#'): continue
		if(sl[0] == '\n'): continue
		if(year=='2016' and 'Summer16' in sl): thisyearindices.append(samplecounter)
		elif(year=='2017' and 'Fall17' in sl): thisyearindices.append(samplecounter)
		elif(year=='2018' and 'Autumn18' in sl): thisyearindices.append(samplecounter)
		samplecounter += 1
	print('found '+str(len(thisyearindices))+' samples for '+year+' '+flavour+'s.')
	for i in thisyearindices:
	    with open(script_name,'w') as script:
		initializeJobScript(script)
		script.write('cd {}\n'.format(cwd))
		command = './fillMagicFactor {} {} {} {} {} {} {}'.format(flavour,year,
								    leptonMVA,mvathreshold,
								    sampledirectory,samplelist,i)
		script.write(command+'\n')
	    submitQsubJob(script_name)
	    # alternative: run locally
	    #os.system('bash '+script_name)
