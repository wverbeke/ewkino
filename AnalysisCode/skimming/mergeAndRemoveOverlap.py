######################################################################
# a python script to call mergeAndRemoveOverlap for data directories #
######################################################################
# This script can be used as standalone caller, in which case only a top-level input dir is needed.
# All subdirs will be scanned for occurences of '201?data' and all files within are merged.
# The functions mergefiles and mergefilesinfolder can also be called from outside this script.

import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript

def samplenameisdata(sample):
    # local variant of isData function, purely based on occurence of certain tags in the name
    if('SingleElectron' in sample or 'SingleMuon' in sample
        or 'DoubleEG' in sample or 'DoubleMuon' in sample
        or 'MuonEG' in sample or 'EGamma' in sample
        or 'JetHT' in sample or 'MET' in sample or 'HTMHT' in sample): return True
    else: return False

def mergefiles(output_file_name,input_files):
    # determine current working directory
    cwd = os.getcwd()
    # check if executable is present here
    if not os.path.exists('mergeAndRemoveOverlap'):
	print('### ERROR ###: merging executable does not seem to exist.')
	print('Run make -f makeMergeAndRemoveOverlap.')
	return
    if len(input_files)<2:
        print('### ERROR ###: number of input files is smaller than 2, skipping...')
        return
    script_name = 'mergeAndRemoveOverlap.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './mergeAndRemoveOverlap {}'.format(output_file_name)
        for f in input_files: command += ' {}'.format(f)
        script.write(command+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

def mergefilesinfolder(output_file_path,input_folder):
    mergefilesinfolders(output_file_path,[input_folder])

def mergefilesinfolders(output_file_path,input_folders):
    
    if os.path.exists(output_file_path):
        os.system('rm '+output_file_path)
    input_files = []
    for input_folder in input_folders:
	input_files += ([os.path.join(input_folder,f) for f in os.listdir(input_folder)
			if (f[-5:]=='.root')])
    mergefiles(output_file_path,input_files)

if __name__=='__main__':

    if len(sys.argv)!=2:
	print('### ERROR ###: this script requires exactly one command line argument')
	print('               python mergeAndRemoveOverlap.py inputfolder')
	sys.exit()

    inputfolder = os.path.abspath(sys.argv[1])

    for root,dirs,files in os.walk(inputfolder):
	for dirname in dirs:
	    fulldirname = os.path.join(root,dirname)

	    # default case: folders containing '201?data' in path name: simply merge all files within.
	    # files are expected to be of the form SingleElectron_Summer16.root
	    appendix = ''
	    if('2016data' in fulldirname): appendix = '_Summer16'
	    elif('2017data' in fulldirname): appendix = '_Fall17'
	    elif('2018data' in fulldirname): appendix = '_Autumn18'
	    if('_flat' in fulldirname): appendix = ''
	    if appendix!='':
		# check that folder contains at least one .root file
		if len([f for f in os.listdir(fulldirname) if f[-5:]=='.root'])==0: continue
		output_file_name = 'data_combined'+appendix+'.root'
		output_file_name = os.path.join(fulldirname,output_file_name)
		print(fulldirname)
		mergefilesinfolder(output_file_name,fulldirname)

	    # for trigger efficiency measurement: folders containing 201?trigger, 
	    # merge PD's but keep era's apart.
	    # files are expected to be of the form SingleElectron_Summer16_Run2016B.root
	    appendix = ''
	    if('2016trigger' in fulldirname): appendix = '_Summer16'
            elif('2017trigger' in fulldirname): appendix = '_Fall17'
            elif('2018trigger' in fulldirname): appendix = '_Autumn18'
	    if appendix!='':
		allfiles = [f for f in os.listdir(fulldirname) 
				if (f[-5:]=='.root' and samplenameisdata(f))]
		inputdict = {}
		for f in allfiles:
		    shortname = f.split('_',1)[1]
		    if shortname in inputdict:
			inputdict[shortname].append(os.path.join(fulldirname,f))
		    else:
			inputdict[shortname] = [os.path.join(fulldirname,f)]
		for f in inputdict:
		    mergefiles(os.path.join(fulldirname,'data_combined_'+f),inputdict[f])
		# now also submit a merging job for era's together
		allfiles = [os.path.join(fulldirname,f) for f in allfiles]
		mergefiles(os.path.join(fulldirname,'data_combined'+appendix+'.root'),allfiles)
    print('done')

