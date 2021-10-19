#########################################################################
# python script to run the runsystematics executable via job submission #
#########################################################################
import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../samplelists'))
from readsamplelist import readsamplelist
from extendsamplelist import extendsamplelist
sys.path.append(os.path.abspath('../../tools'))
import smalltools as tls

if len(sys.argv) != 4:
    print('### ERROR ###: fillpileup.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python runsystematics.py input_directory samplelist output_directory,')
    # maybe add more command line arguments later, keep hard coded for now
    sys.exit()

# parse command line arguments
input_directory = os.path.abspath(sys.argv[1])
if not os.path.exists(input_directory):
    print('### ERROR ###: input directory '+input_directory+' does not seem to exist...')
    sys.exit()
samplelist = os.path.abspath(sys.argv[2])
if not os.path.exists(samplelist):
    print('### ERROR ###: requested sample list does not seem to exist...')
    sys.exit()
output_directory = sys.argv[3]

cwd = os.getcwd()

if os.path.exists(output_directory):
    #print('### WARNING ###: output direcory already exists. Clean it? (y/n)')
    #go = raw_input()
    #if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
output_directory = os.path.abspath(output_directory)

# determine data taking year and luminosity from sample list name
(year,lumi) = tls.year_and_lumi_from_samplelist(samplelist)
if lumi<0. : sys.exit()

# determine data type from sample name
dtype = tls.data_type_from_samplelist(samplelist)
if len(dtype)==0 : sys.exit()

# make a list of input files in samplelist and compare to content of input directory 
inputfiles = extendsamplelist(samplelist,input_directory)
# the above does not work well for data, as it does not include the data_combined files
if dtype=='data':
    inputfiles = []
    files = [f for f in os.listdir(input_directory) if f[-5:]=='.root']
    for f in files: 
	inputfiles.append({'file':os.path.join(input_directory,f),'process_name':'data'})

# subselect input files based on type of event selection
inputfiles = tls.subselect_inputfiles(inputfiles,'3tight')
# (note: not really selecting 3 tight leptons, but using same files)
if inputfiles is None: sys.exit()

# check if executable is present
if not os.path.exists('./fillpileup'):
    print('### ERROR ###: checkpileup executable was not found.')
    print('Run make -f makeFillPileup before running this script.')
    sys.exit()

def parseprocessname(orig):
    res = orig
    res = res.replace(' ','')
    # move other parsing to .cc file in order to use full name as hist title!
    return res

def submitjob(cwd,inputfile,norm,output_directory,process_name):
    script_name = 'fillpileup.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
	output_file_path = os.path.join(output_directory,inputfile.split('/')[-1])
        command = './fillpileup {} {} {} {}'.format(
		    inputfile, norm, output_file_path, process_name)
        script.write(command+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

# make output directory
os.makedirs(output_directory)

# loop over input files and submit jobs
#inputfiles = [f for f in inputfiles if 'ttHToNonbb' in f['sample_name']] # temp for testing
#inputfiles = [f for f in inputfiles if 'combined' in f['file']] # temp for testing
#print(inputfiles)
for f in inputfiles:
    # get name and tag
    inputfile = f['file']
    process_name = parseprocessname(f['process_name'])
    # create normalization variable:
    norm = 1.
    if dtype == 'MC':
	temp = ROOT.TFile(inputfile)
	hcounter = temp.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
	xsec = f['cross_section']
	norm = xsec*lumi/float(hcounter)
    # set path to fake rate maps if needed
    submitjob(cwd, inputfile, norm, output_directory, process_name)
