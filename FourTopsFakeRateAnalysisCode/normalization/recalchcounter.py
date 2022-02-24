##########################################
# recalculate hCounter for given samples #
##########################################

import sys
import os
import ROOT
import json
sys.path.append('../../skimmer')
from fileListing import listSampleDirectories, listFiles
sys.path.append(os.path.abspath('../../Tools/python'))
from samplelisttools import readsamplelist
sys.path.append('../../jobSubmission')
import condorTools as ct

if __name__=="__main__":

    inputdir = sys.argv[1]
    samplelist = sys.argv[2]
    outputfile = sys.argv[3]
    runmode = 'condor'
    for arg in sys.argv[4:]:
	key,val = arg.split('=',1)
	if key=='runmode': runmode = val
	else:
	    raise Exception('ERROR: unrecognized argument "{}"'.format(key))

    # check arguments
    if not os.path.exists(inputdir):
	raise Exception('ERROR: input directory {} does not exist'.format(inputdir))
    if not os.path.exists(samplelist):
	raise Exception('ERROR: sample list {} does not exist'.format(samplelist))
    if not runmode in ['condor','local']:
	raise Exception('ERROR: rumode not recognized "{}"'.format(runmode))

    # if runmmode is condor: submit a job
    if runmode=='condor':
	cmd = 'python recalchcounter.py'
	for arg in sys.argv[1:]: cmd += ' {}'.format(arg)
	# add runmode local at the end
	cmd += ' runmode=local'
	ct.submitCommandAsCondorJob('cjob_recalchcounter', cmd)
	sys.exit()

    # read the sample list
    samples_dict = readsamplelist(samplelist)
    
    # make a list of samples (the main directories) 
    # and an equally long list of subdirectories with the latest version of the ntuples 
    # (one for each main directory)
    for sample in samples_dict:
	totaldir = os.path.join(inputdir, sample['sample_name'], sample['version_name'])
        if os.path.exists(totaldir):
	    sample['total_dir'] = totaldir
        else:
            print('WARNING: sample {} not found in input directory {}'.format(
	    sample['sample_name']+'/'+sample['version_name'], inputdir))
            print('Continue without? (y/n)')
            go = raw_input()
            if not go=='y': sys.exit()

    # printouts for testing
    print('found {} valid sample directories.'.format(len(samples_dict)))
    for sample in samples_dict:
	print('  - {}'.format(sample['total_dir']))

    # loop over samples
    cwd = os.getcwd()
    for sample in samples_dict:
	root_files = list(listFiles( sample['total_dir'], '.root' ))
	print('processing the following sample:')
	print(sample['total_dir'])
	print('number of root files: {}'.format(len(root_files)))
	hcounter = 0
	# loop over files
	for f in root_files:
	    fileptr = ROOT.TFile.Open(f,'read')
	    thishcounter = fileptr.Get('blackJackAndHookers/hCounter')
	    thishcounter = thishcounter.GetBinContent(1)
	    hcounter += thishcounter
	print('hCounter: {}'.format(hcounter))
	sample['hcounter'] = hcounter

    # write result to file
    outputfile = os.path.splitext(outputfile)[0]+'.json'
    res = {}
    for sample in samples_dict:
	res[sample['sample_name']] = sample['hcounter']
    with open(outputfile, 'w') as f:
	json.dump(res, f)

    # write results to file (other format)
    outputfile = os.path.splitext(outputfile)[0]+'.txt'
    with open(outputfile, 'w') as f:
	for sample in samples_dict:
	    line = '{} {}'.format(sample['process_name'], sample['sample_name'])
	    line += ' {}'.format(sample['hcounter'])
	    line += '\n'
	    f.write(line)
