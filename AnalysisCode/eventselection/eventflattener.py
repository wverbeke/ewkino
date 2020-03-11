#########################################################################
# python script to run the eventflattener executable via job submission #
#########################################################################
import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, an event flattening executable is called.
# command line arguments (in sequence):
# - input directory (containing samples of pre-selected events (e.g. using eventselector.py))
# - sample list (needed to compute weight from cross-sections)
# - output directory
# - name of the event selection to be applied; usually the same as used to create the input file!

if len(sys.argv) != 5:
    print('### ERROR ###: eventflattener.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python eventflattener.py <input_directory> <samplelist> <output_directory> <event_selection>')
    sys.exit()

input_directory = os.path.abspath(sys.argv[1])
samplelist = os.path.abspath(sys.argv[2])
if not os.path.exists(samplelist):
    print('### ERROR ###: requested sample list does not seem to exist.')
    sys.exit()
output_directory = sys.argv[3]
if os.path.exists(output_directory):
    os.system('rm -r '+output_directory)
os.makedirs(output_directory)
output_directory = os.path.abspath(output_directory)
event_selection = sys.argv[4]
cwd = os.getcwd()

# make a list of input files in samplelist and compare to content of input directory 
# note: the names of the files in input directory can be more extended with respect to the names
# in the sample list, for example they can have year extensions etc.
inputfiles = readsamplelist(samplelist,unique=True)
inputfilesnew = []
missinglist = []
for f in inputfiles:
    fname = f['sample_name']
    indir = False
    for findir in os.listdir(input_directory):
	if fname in findir:
	    indir = True
	    f['file'] = os.path.join(input_directory,findir)
	    inputfilesnew.append(f)
	    break
    if not indir: missinglist.append(fname)
inputfiles = inputfilesnew
if len(missinglist)>0:
    print('### WARNING ###: the following samples are in sample list but not in input directory:')
    print(missinglist)

# determine data taking year and luminosity from sample list name.
slname = samplelist[samplelist.rfind('/'):]
year = ''
lumi = 0.
if '2016' in slname:
    year = '2016'
    lumi = 35900
elif '2017' in slname:
    year = '2017'
    lumi = 41500
elif '2018' in slname:
    year = '2018'
    lumi = 59700
else: 
    print('### ERROR ###: year not recognized from samplelist '+str(slname))
    sys.exit()

# determine data type from sample name.
dtype = ''
if 'MC' in slname:
    dtype = 'MC'
elif 'data' in slname:
    dtype = 'data'
else:
    print('### ERROR ###: data type not recognized from samplelist '+str(slname))
    sys.exit()

# check if executable is present
if not os.path.exists('./eventflattener'):
    print('### ERROR ###: eventflattener executable was not found.')
    print('Run make -f makeEventFlattener before running this script.')
    sys.exit()

# loop over input files and submit jobs
for f in inputfiles:
    inputfile = f['file']
    # create normalization variable:
    norm = 1.
    if dtype == 'MC':
	temp = ROOT.TFile(inputfile)
	hcounter = temp.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
	xsec = f['cross_section']
	norm = xsec*lumi/float(hcounter)
    script_name = 'eventflattener.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './eventflattener {} {} {} {}'.format(inputfile,norm,output_directory,event_selection)
        script.write(command+'\n')
        # change name of output file to correspond to the name of the input file
        outname = inputfile[inputfile.rfind('/')+1:]
        script.write('cd '+output_directory+'\n')
        script.write('mv *'+outname+' '+outname+'\n')
        script.write('cd '+cwd+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)
