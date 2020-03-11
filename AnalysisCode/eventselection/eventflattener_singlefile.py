#################################################################
# Make an eventflattener.cc submission script for a single file #
#################################################################
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

# do not use this script lightly as it can create files with different flattening in the same folder...

input_directory = '/storage_mnt/storage/user/llambrec/Files/trileptonskim/2016MC'
input_file = 'WZTo3LNu_mllmin01_13TeV-powheg-pythia8_wext1_Summer16.root'
output_directory = '/storage_mnt/storage/user/llambrec/Files/signalregion/2016MC_flat'
event_selection = 'signalregion'

if not len(sys.argv)==4:
    print('### WARNING ###: use default arguments? (y/n)')
    go = raw_input()
    if not go=='y':
        sys.exit()
else:
    input_directory = os.path.abspath(sys.argv[1][:sys.argv[1].rfind('/')])
    input_file = sys.argv[1][sys.argv[1].rfind('/')+1:]
    output_directory = os.path.abspath(sys.argv[2])
    event_selection = sys.argv[3]

if os.path.exists(os.path.join(output_directory,input_file)):
    os.system('rm '+os.path.join(output_directory,input_file))
if not os.path.exists(output_directory):
    os.makedirs(output_directory)
cwd = os.getcwd()
input_file = os.path.join(input_directory,input_file)

# check if input file is present
if not os.path.exists(input_file):
    print('### ERROR ###: input file was not found.')
    sys.exit()

# check if executable is present
if not os.path.exists('./eventflattener'):
    print('### ERROR ###: eventflattener executable was not found.')
    print('Run make -f makeEventFlattener before running this script.')
    sys.exit()

script_name = 'eventflattener.sh'
with open(script_name,'w') as script:
    initializeJobScript(script)
    script.write('cd {}\n'.format(cwd))
    command = './eventflattener {} {} {}'.format(input_file,output_directory,event_selection)
    script.write(command+'\n')
    # change name of output file to correspond to the name of the input file
    outname = input_file[input_file.rfind('/')+1:]
    script.write('cd '+output_directory+'\n')
    script.write('mv *'+outname+' '+outname+'\n')
    script.write('cd '+cwd+'\n')
submitQsubJob(script_name)
# alternative: run locally
#os.system('bash '+script_name)

