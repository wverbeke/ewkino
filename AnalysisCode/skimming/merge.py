####################################################################################
# python script to use functionality from ewkino/skimmer and merge skimmed ntuples #
####################################################################################

import sys 
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
import mergeTuples as mt
sys.path.append('../../jobSubmission')
import condorTools as ct
# import functions from the merge and remove overlap script
from mergeAndRemoveOverlap import mergefilesinfolders, samplenameisdata

# this scipt reproduces and extends the functionality of ewkino/skimmer/mergeTuples.py
# command line arguments (in sequence):
# - input directory
# - output directory

def sampleName(sample,mode):
    ### override default sampleName function (in ewkino/skimmer/mergeTuples)
    # usage: merging extensions of MC samples and keeping eras apart for data samples
    name = mt.sampleName(sample)
    # for data, default sampleName is of the form SingleElectron_Summer16
    # for sim, default sampleName is of the form tZq_ll_4f_PSweights_13TeV-amcatnlo-pythia8_Summer16

    # merge samples which only differ by '_extx' by removing that part from the name
    if '_ext' in name:
        name = name[:name.find('_ext')]+name[name.find('_ext')+5:]
    # for data files used in trigger measurement, keep eras apart
    if( mode=='trigger' and '_Run' in sample ):
            name = name + sample[sample.find('_Run'):sample.find('_Run')+9]
    return name

if len( sys.argv ) < 3:
    print( '### ERROR ###: merge.py requires additional command-line arguments.' )
    print( 'Usage : python merge.py <input_directory> <output_directory> <mode>' )
    print( 'The input directory should be the directory which contains the unmerged samples as subdirectories' )
    sys.exit()

# parse mode command line argument
# 'mode' can be either 'default' or 'trigger'
# does not affect MC, only data:
# different eras are merged into one for default mode, kept apart for 'trigger' mode
mode = 'default'
if len(sys.argv) == 4:
    mode = sys.argv[3]
else:
    print('### WARNING ###: use default mode? (y/n)')
    go = raw_input()
    if not go == 'y': sys.exit()
if not mode in ['default','trigger']:
    print('### ERROR ###: mode not recognized: '+str(mode))

# set input and output location
input_directory = os.path.abspath( sys.argv[1] )
output_directory = os.path.abspath( sys.argv[2] )
if not os.path.exists(output_directory):
    os.makedirs(output_directory)
print('searching for skimmed samples in '+input_directory)
inputlist = list( mt.listSkimmedSampleDirectories( input_directory ) )
print('found '+str(len(inputlist))+' input folders')

# reshape list of all skimmed samples in a directory into dict with unique sample names
# structure: {sampleName1:[sample1,sample2,...],sampleName2:[sample3,...]}
inputstruct = {}
for sample in inputlist:
    samplename = sampleName( sample, mode )
    if samplename in inputstruct:
        inputstruct[samplename].append(sample)
    else:
        inputstruct[samplename] = [sample]

# for data files, also merge different primary datasets
inputstruct_ext = {}
for samplename in inputstruct:
    if not samplenameisdata(samplename): continue
    # data sample names are expected to be of the form <dataset>_<yeartag>
    # or potentially <dataset>_<yeartag>_<eratag>
    samplename_period = samplename.split('_',1)[1] # remove dataset name
    samplename_onlyyear = samplename_period.split('_',1)[0] # remove era tag
    # in case no eratag is present:
    if samplename_period==samplename_onlyyear:
	if not samplename_period in inputstruct_ext: inputstruct_ext[samplename_period] = []
	for sample in inputstruct[samplename]: inputstruct_ext[samplename_period].append(sample)
    # in case there is one, add both total year and era separately:
    else:
	if not samplename_period in inputstruct_ext: inputstruct_ext[samplename_period] = []
        for sample in inputstruct[samplename]: inputstruct_ext[samplename_period].append(sample)
	if not samplename_onlyyear in inputstruct_ext: inputstruct_ext[samplename_onlyyear] = []
	for sample in inputstruct[samplename]: inputstruct_ext[samplename_onlyyear].append(sample)
    print(inputstruct_ext)

# temp: additional selection
#newinputstruct = {}
#for samplename in inputstruct:
#    if not ( 'DYJets' in samplename or 'WJets' in samplename or 'TTTo2L2Nu' in samplename
#	or 'ZGTo2LG' in samplename or 'ST_t-channel' in samplename
#	or 'ST_tW' in samplename or 'TTJets' in samplename or 'TTToSemiLeptonic' in samplename
#	or 'WWTo2L2Nu' in samplename
#	or ('WZTo3LNu' in samplename and 'powheg' in samplename) ): continue
#    newinputstruct[samplename] = inputstruct[samplename]
#inputstruct = newinputstruct

# print
print('found '+str(len(inputstruct))+' unique samples')
print('samples will be merged as follows:')
for s in inputstruct:
    print('  '+s+':')
    for sf in inputstruct[s]:
	print('   - '+sf.split('/')[-1])
if len(inputstruct_ext)>0:
    print('additionally, will do the following data mergings:')
    for s in inputstruct_ext:
	print('  '+s+':')
	for sf in inputstruct_ext[s]:
	    print('   - '+sf.split('/')[-1])

# run hadd command for each element in inputstruct
counter = 0
for samplename in inputstruct:
    samplelist = inputstruct[samplename]
    outfile = os.path.join(output_directory,sampleName(samplelist[0],mode)+'.root')
    commands = []
    commands.append('echo "###starting###" >&2')
    command = 'hadd {}'.format(outfile)
    for sample in samplelist: 
	command += ' {}/*.root'.format(os.path.join(input_directory,sample))
    commands.append(command)
    print(command)
    commands.append('echo "###done###" >&2')
    # old qsub way:
    #script_name = 'merge.sh'
    #with open(script_name,'w') as script:
    #    initializeJobScript(script)
    #	print(command)
    #    script.write(command+'\n')
    #submitQsubJob(script_name)
    #counter += 1
    print('submitted hadd command for '+str(counter)+' sample directories.')
    # new condor way
    ct.submitCommandsAsCondorJob( 'cjob_merge', commands )

# run mergeAndRemoveOverlap command for each element in extension of input struct
counter = 0
for samplename in inputstruct_ext:
    samplelist = inputstruct_ext[samplename]
    samplelist = [os.path.join(input_directory,s) for s in samplelist]
    outfile = os.path.join(output_directory,'data_combined_'+samplename+'.root')
    mergefilesinfolders(outfile,samplelist)
    counter += 1
    print('submitted hadd command for '+str(counter)+' sample directories.')
