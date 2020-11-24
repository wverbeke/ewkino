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

if len(sys.argv) != 8:
    print('### ERROR ###: runsystematics.py requires a different number of command-line arguments.')
    print('Normal usage from the command line:')
    print('python runsystematics.py input_directory samplelist output_directory,')
    print('                         event_selection, selection_type, event_category,')
    print('			    bdt_combine_mode')
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
event_selection = sys.argv[4]
selection_type = sys.argv[5]
signal_category = int(sys.argv[6]) # put 0 for control regions
bdt_combine_mode = sys.argv[7]

systematics = (['JEC','JER','Uncl', # acceptance
		#'muonID','electronID', # (for ttH ID)
                'muonIDSyst','muonIDStat', # weight (for TOP ID)
		'electronIDSyst','electronIDStat', # weight (for TOP ID)
		'pileup','bTag_heavy','bTag_light','prefire', # weight
                'fScale','rScale','rfScales','isrScale','fsrScale', # scale
		'electronReco', # electronreco
                'pdfShapeVar','pdfNorm', # lhe
		'qcdScalesShapeVar','qcdScalesNorm' # lhe
		]) 
#systematics = ['JEC','pileup','fScale'] # smaller test set of systematics
cwd = os.getcwd()

# check  arguments
if event_selection not in (['signalregion','signalsideband_noossf','signalsideband_noz',
                            'wzcontrolregion','zzcontrolregion','zgcontrolregion']):
    print('### ERROR ###: event_selection not in list of recognized event selections')
    sys.exit()
if os.path.exists(output_directory):
    #print('### WARNING ###: output direcory already exists. Clean it? (y/n)')
    #go = raw_input()
    #if not go=='y': sys.exit()
    os.system('rm -r '+output_directory)
output_directory = os.path.abspath(output_directory)
if bdt_combine_mode not in (['all','years','regions','years']):
    print('### ERROR ###: bdt_combine_mode not in list of recognized modes')
    sys.exit()

# determine data taking year and luminosity from sample list name
(year,lumi) = tls.year_and_lumi_from_samplelist(samplelist)
if lumi<0. : sys.exit()

# determine data type from sample name
dtype = tls.data_type_from_samplelist(samplelist)
if len(dtype)==0 : sys.exit()

# set path to BDT:
# (update: moved bdt's to subdirectories within bdt folder)
# (        keep here same code (apart from existence check) but add subdirectories in .cc)
# (        cannot do it here since C++ function needed to decide on subdirectory...)
path_to_xml_file = ''
if( bdt_combine_mode=='all' or signal_category not in [1,2,3] ):
    path_to_xml_file = os.path.abspath('../bdt/out_all_data/weights/tmvatrain_BDT.weights.xml')
    bdt_combine_mode='all' # reset bdt_combine_mode to 'all' for control regions
elif( bdt_combine_mode=='years' ):
    path_to_xml_file = os.path.abspath('../bdt/out_treeCat'+str(signal_category)+'_data/'
					+'weights/tmvatrain_BDT.weights.xml')
elif( bdt_combine_mode=='regions' ):
    path_to_xml_file = os.path.abspath('../bdt/out_'+year+'_data/'
					+'weights/tmvatrain_BDT.weights.xml')
elif( bdt_combine_mode=='none' ):
    path_to_xml_file = os.path.abspath('../bdt/out_'+year+'_treeCat'+str(signal_category)+'_data/'
					+'weights/tmvatrain_BDT.weights.xml')
#if( not os.path.exists( path_to_xml_file ) or path_to_xml_file=='' ):
if( path_to_xml_file=='' ):
    print('### ERROR ###: requested xml file '+path_to_xml_file+' does not seem to exist...')
    sys.exit()

# make a list of input files in samplelist and compare to content of input directory 
inputfiles = extendsamplelist(samplelist,input_directory)
# the above does not work well for data, as it does not include the data_combined files
if dtype=='data':
    inputfiles = []
    files = [f for f in os.listdir(input_directory) if f[-5:]=='.root']
    for f in files: 
	inputfiles.append({'file':os.path.join(input_directory,f),'process_name':'data'})

# subselect input files based on type of event selection
inputfiles = tls.subselect_inputfiles(inputfiles,selection_type)
if inputfiles is None: sys.exit()

# check if executable is present
if not os.path.exists('./runsystematics'):
    print('### ERROR ###: runsystematics executable was not found.')
    print('Run make -f makeRunSystematics before running this script.')
    sys.exit()

def parseprocessname(orig):
    res = orig
    res = res.replace(' ','')
    # move other parsing to .cc file in order to use full name as hist title!
    return res

def submitjob(cwd,inputfile,norm,output_directory,process_name,
		event_selection,signal_category,selection_type,
		bdt_combine_mode,path_to_xml_file,systematics):
    script_name = 'runsystematics.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
	output_file_path = os.path.join(output_directory,inputfile.split('/')[-1])
        command = './runsystematics {} {} {} {} {} {} {} {} {}'.format(
		    inputfile, norm, output_file_path, process_name, 
		    event_selection, signal_category, selection_type,
		    bdt_combine_mode, path_to_xml_file)
	for systematic in systematics:
	    command += ' {}'.format(systematic)
        script.write(command+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

# make output directory
os.makedirs(output_directory)

# loop over input files and submit jobs
#inputfiles = [f for f in inputfiles if 'tZq' in f['sample_name']] # temp for testing
#inputfiles = [f for f in inputfiles if 'combined' in f['file']] # temp for testing
#print(inputfiles)
for f in inputfiles:
    # get name and tag
    inputfile = f['file']
    process_name = parseprocessname(f['process_name'])
    if selection_type == 'fakerate': process_name = 'nonprompt'
    # create normalization variable:
    norm = 1.
    if dtype == 'MC':
	temp = ROOT.TFile(inputfile)
	hcounter = temp.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
	xsec = f['cross_section']
	norm = xsec*lumi/float(hcounter)
    # set path to fake rate maps if needed
    submitjob(cwd, inputfile, norm, output_directory, process_name,
		    event_selection, signal_category, selection_type, 
		    bdt_combine_mode, path_to_xml_file, systematics)
