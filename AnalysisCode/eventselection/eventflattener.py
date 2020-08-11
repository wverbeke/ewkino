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

if len(sys.argv) != 10:
    print('### ERROR ###: eventflattener.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python eventflattener.py <input_directory> <samplelist> <output_directory>') 
    print('<event_selection> <selection_type> <variation> <do_mva> <path_to_xml_file> <path_to_frmaps>')
    sys.exit()

input_directory = os.path.abspath(sys.argv[1])
if not os.path.exists(input_directory):
    print('### ERROR ###: input directory '+input_directory+' does not seem to exist...')
    sys.exit()
samplelist = os.path.abspath(sys.argv[2])
if not os.path.exists(samplelist):
    print('### ERROR ###: requested sample list does not seem to exist...')
    sys.exit()
output_directory = sys.argv[3]
if os.path.exists(output_directory):
    os.system('rm -r '+output_directory)
output_directory = os.path.abspath(output_directory)
event_selection = sys.argv[4]
selection_type = sys.argv[5]
variation = sys.argv[6]
do_mva = sys.argv[7]
path_to_xml_file = os.path.abspath(sys.argv[8])
frdir = os.path.abspath(sys.argv[9])
cwd = os.getcwd()

# check command line arguments
if event_selection not in (['signalregion','signalsideband_noossf','signalsideband_noz',
                            'wzcontrolregion','zzcontrolregion','zgcontrolregion']):
    print('### ERROR ###: event_selection not in list of recognized event selections')
    sys.exit()
if variation not in ['nominal','JECDown','JECUp','JERDown','JERUp','UnclDown','UnclUp']:
    print('### ERROR ###: variation not in list of recognized scale variations')
    sys.exit()
if do_mva not in ['true','True','false','False']:
    print('### ERROR ###: do_mva not in list of recognized values')
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
inputfiles = extendsamplelist(samplelist,input_directory)

# check if executable is present
if not os.path.exists('./eventflattener'):
    print('### ERROR ###: eventflattener executable was not found.')
    print('Run make -f makeEventFlattener before running this script.')
    sys.exit()

# subselect input files based on type of event selection
inputfiles = tls.subselect_inputfiles(inputfiles,selection_type)
if inputfiles is None: sys.exit()

def submitjob(cwd,inputfile,norm,output_directory,
		event_selection,selection_type,variation,
		muonfrmap, electronfrmap,
		do_mva, path_to_xml_file):
    script_name = 'eventflattener.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './eventflattener {} {} {} {} {} {} {} {} {} {} {}'.format(
		    inputfile,norm,output_directory,inputfile.split('/')[-1],
		    event_selection,selection_type,variation,
		    muonfrmap,electronfrmap,
		    do_mva, path_to_xml_file)
        script.write(command+'\n')
	print(command)
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

# make output directory
os.makedirs(output_directory)

# loop over input files and submit jobs
for f in inputfiles:
    inputfile = f['file']
    # temporary selection for testing
    #if not 'tZq' in inputfile: continue
    # create normalization variable:
    norm = 1.
    if dtype == 'MC':
	temp = ROOT.TFile(inputfile)
	hcounter = temp.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
	xsec = f['cross_section']
	norm = xsec*lumi/float(hcounter)
    year = tls.year_from_filepath(inputfile)
    frmap_muon = os.path.join(frdir,'fakeRateMap_data_muon_'+year+'_mT.root')
    frmap_electron = os.path.join(frdir,'fakeRateMap_data_electron_'+year+'_mT.root')
    submitjob(cwd,inputfile,norm,output_directory,
		event_selection,selection_type,variation,
		frmap_muon,frmap_electron,
		    do_mva,path_to_xml_file)
