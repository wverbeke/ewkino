########################################################################
# python script to run the eventselector executable via job submission #
########################################################################

import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls

# this script is analogous to ewkino/AnalysisCode/skimming/trileptonskim.py
# but instead of a skimmer, an event selection executable is called.
# command line arguments (in sequence):
# - input directory (containing skimmed and merged root files, usually one per sample)
# - output directory (containing analogous files but holding only selected events)
# - name of the event selection to be applied

nonpromptfromdata = True 
# maybe later add as a command line argument
# but for now process both nonprompt simulation as estimate from data

if len(sys.argv) != 5:
	print('### ERROR ###: eventselector.py requires a different number of command-line arguments.')
	print('Normal usage from the command line:')
	print('python eventselector.py input_directory output_directory event_selection')
	print('leptonID')
	sys.exit()

input_directory = os.path.abspath(sys.argv[1])
output_directory = sys.argv[2]
output_directory = os.path.abspath(output_directory)
if os.path.exists(output_directory):
    os.system('rm -r '+output_directory)
os.makedirs(output_directory)
event_selection = sys.argv[3]
leptonID = sys.argv[4]
cwd = os.getcwd()

# check command line arguments
if event_selection not in (['signalregion','signalsideband_noossf','signalsideband_noz',
			    'wzcontrolregion','zzcontrolregion','zgcontrolregion']):
    print('### ERROR ###: event_selection not in list of recognized event selections')
    sys.exit()
if leptonID not in ['tth','tzq']:
    print('### ERROR ###: leptonID not in list of recognized lepton IDs')
    sys.exit()

# make a list of input files
inputfiles = []
os.chdir(input_directory)
for inputfile in glob.glob('*.root'):
    inputfiles.append(os.path.join(input_directory,inputfile))
os.chdir(cwd)
print('found '+str(len(inputfiles))+' root files in input directory.')

# check if executable is present
if not os.path.exists('./eventselector'):
    print('### ERROR ###: eventselector executable was not found.')
    print('Run make -f makeEventSelector before running this script.')
    sys.exit()

def submitjob(cwd,inputfile,output_directory,event_selection,leptonID,isnpbackground):
    script_name = 'eventselector.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './eventselector {} {} {} {} {} {}'.format(inputfile,output_directory,
			inputfile.split('/')[-1],event_selection,leptonID,isnpbackground)
        script.write(command+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

# loop over input files and submit jobs
for inputfile in inputfiles:
    submitjob(cwd,inputfile,output_directory,event_selection,leptonID,False)
    if( nonpromptfromdata and tls.isdata_from_filepath(inputfile)):
	submitjob(cwd,inputfile,output_directory,event_selection,leptonID,True)
    
