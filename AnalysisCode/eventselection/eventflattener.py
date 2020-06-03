##############i###########################################################
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
from extendsamplelist import extendsamplelist
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, an event flattening executable is called.
# command line arguments (in sequence):
# - input directory (containing samples of pre-selected events (e.g. using eventselector.py))
# - sample list (needed to compute weight from cross-sections)
# - output directory
# - name of the event selection to be applied; usually the same as used to create the input file!

nonpromptfromdata = True
frdir = os.path.abspath('../fakerate/fakeRateMaps')
# maybe later add as a command line argument
# but for now process both nonprompt simulation as estimate from data

if len(sys.argv) != 6:
    print('### ERROR ###: eventflattener.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python eventflattener.py input_directory samplelist output_directory event_selection')
    print('variation')
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
variation = sys.argv[5]
cwd = os.getcwd()

# check command line arguments
if event_selection not in (['signalregion','signalsideband_noossf','signalsideband_noz',
                            'wzcontrolregion','zzcontrolregion','zgcontrolregion']):
    print('### ERROR ###: event_selection not in list of recognized event selections')
    sys.exit()
if variation not in ['nominal','JECDown','JECUp','JERDown','JERUp','UnclDown','UnclUp']:
    print('### ERROR ###: variation not in list of recognized scale variations')
    sys.exit()

# determine data taking year and luminosity from sample list name.
(year,lumi) = tls.year_and_lumi_from_samplelist(samplelist)
if lumi<0. : sys.exit()

# determine data type from sample name.
dtype = tls.data_type_from_samplelist(samplelist)    
if len(dtype)==0 : sys.exit()

# make a list of input files in samplelist and compare to content of input directory 
# note: the names of the files in input directory can be more extended with respect to the names
# in the sample list, for example they can have year extensions etc.
# in case of data, simply use all files in the directory
if dtype=='MC':
    inputfiles = extendsamplelist(samplelist,input_directory)
else:
    inputfiles = []
    for f in os.listdir(input_directory):
	inputfiles.append({'file':os.path.join(input_directory,f)})

# check if executable is present
if not os.path.exists('./eventflattener'):
    print('### ERROR ###: eventflattener executable was not found.')
    print('Run make -f makeEventFlattener before running this script.')
    sys.exit()

def submitjob(cwd,inputfile,norm,output_directory,event_selection,variation,
		isnpbackground, muonfrmap, electronfrmap):
    script_name = 'eventflattener.sh'
    if isnpbackground: 
	output_directory = os.path.join(output_directory,'nonprompt_background')
	if not os.path.exists(output_directory): os.makedirs(output_directory)
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './eventflattener {} {} {} {} {} {} {} {} {}'.format(
		    inputfile,norm,output_directory,inputfile.split('/')[-1],event_selection,
		    variation,
		    isnpbackground,muonfrmap,electronfrmap)
        script.write(command+'\n')
	print(command)
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

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
    if(not 'nonprompt_background' in inputfile):
	submitjob(cwd,inputfile,norm,output_directory,event_selection,variation,False,'.','.')
    else:
	year = tls.year_from_filepath(inputfile)
	frmap_muon = os.path.join(frdir,'fakeRateMap_data_muon_'+year+'_mT.root')
	frmap_electron = os.path.join(frdir,'fakeRateMap_data_electron_'+year+'_mT.root')
        submitjob(cwd,inputfile,norm,output_directory,event_selection,variation,
		    True,frmap_muon,frmap_electron)
