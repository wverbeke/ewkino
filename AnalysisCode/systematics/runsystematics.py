#########################################################################
# python script to run the runsystematics executable via job submission #
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
from extendsamplelist import extendsamplelist
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, a systematics executable is called.
# command line arguments: see below

if len(sys.argv) != 5:
    print('### ERROR ###: runsystematics.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python eventflattener.py input_directory samplelist output_directory, leptonID')
    # maybe add more command line arguments later, keep hard coded for now
    sys.exit()

# parse command line arguments
input_directory = os.path.abspath(sys.argv[1])
samplelist = os.path.abspath(sys.argv[2])
if not os.path.exists(samplelist):
    print('### ERROR ###: requested sample list does not seem to exist.')
    sys.exit()
output_directory = sys.argv[3]
if os.path.exists(output_directory):
    print('### WARNING ###: output direcory already exists. Clean it? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
os.makedirs(output_directory)
output_directory = os.path.abspath(output_directory)
leptonID = sys.argv[4]

# hard-code rest of arguments
variable = '_abs_eta_recoil'
path_to_xml_file = "apath" # only needed if variable == '_eventBDT'
xlow = 0.
xhigh = 5.
nbins = 20
systematics = ['JEC','pileup']
cwd = os.getcwd()

# check validity of arguments
if leptonID not in ['tth','tzq']:
    print('### ERROR ###: leptonID not in list of recognized lepton IDs')
    sys.exit()

# determine data taking year and luminosity from sample list name
(year,lumi) = tls.year_and_lumi_from_samplelist(samplelist)
if lumi<0. : sys.exit()

# determine data type from sample name
# (note: not sure if runsystematics will work on data, usually apply to MC only (?))
dtype = tls.data_type_from_samplelist(samplelist)    
if len(dtype)==0 : sys.exit()

# make a list of input files in samplelist and compare to content of input directory 
if dtype=='MC':
    inputfiles = extendsamplelist(samplelist,input_directory)
else:
    inputfiles = []
    for f in os.listdir(input_directory):
	inputfiles.append({'file':os.path.join(input_directory,f)})

# check if executable is present
if not os.path.exists('./runsystematics'):
    print('### ERROR ###: runsystematics executable was not found.')
    print('Run make -f makeRunSystematics before running this script.')
    sys.exit()

def submitjob(cwd,inputfile,norm,output_directory,
		leptonID, variable, path_to_xml_file,
		xlow, xhigh, nbins, systematics):
    script_name = 'runsystematics.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
	output_file_path = os.path.join(output_directory,inputfile.split('/')[-1])
        command = './runsystematics {} {} {} {} {} {} {} {} {}'.format(
		    inputfile, norm, output_file_path,
		    leptonID, variable, path_to_xml_file,
		    xlow, xhigh, nbins)
	for systematic in systematics:
	    command += ' {}'.format(systematic)
        script.write(command+'\n')
    #submitQsubJob(script_name)
    # alternative: run locally
    os.system('bash '+script_name)

# loop over input files and submit jobs
for f in inputfiles[:1]:
    inputfile = f['file']
    # create normalization variable:
    norm = 1.
    if dtype == 'MC':
	temp = ROOT.TFile(inputfile)
	hcounter = temp.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
	xsec = f['cross_section']
	norm = xsec*lumi/float(hcounter)
    submitjob(cwd,inputfile,norm,output_directory,
		    leptonID,variable,path_to_xml_file,
		    xlow,xhigh,nbins,systematics)
