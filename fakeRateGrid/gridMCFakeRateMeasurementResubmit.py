#################################################################
# dedicated submitter for gridMCFakeRateMeasurement failed jobs #
#################################################################

import os
import sys
import glob
sys.path.append('../jobSubmission')
import jobCheck

if __name__=='__main__':

    # find error log files
    print('finding error log files...')
    condorpattern = './*_err_*'
    elfiles = glob.glob(condorpattern)
    nelfiles = len(elfiles)
    print('found {} error log files.'.format(nelfiles))
    print('start scanning...')

    # loop over all error log files found above
    # and find those corresponding to unfinished/failed jobs
    errorfiles = []
    nerrortag = 0
    nunfinished = 0
    for elfile in elfiles:
        if jobCheck.check_start_done(elfile,
                starting_tag = '###starting###',
                done_tag = '###done###'):
            errorfiles.append(elfile)
            nunfinished += 1
        elif jobCheck.check_error_content(elfile):
            errorfiles.append(elfile)
            nerrortag += 1
    nerrorfiles = len(errorfiles)
    print('found {} error log files'.format(nunfinished)
            +' corresponding to unfinished jobs and {}'.format(nerrortag)
            +' corresponding to known error content.')

    # handle case of no failures
    if( nerrorfiles==0 ):
        print('nothing to resubmit; exiting.')
        sys.exit()

    # sort the list of error log files
    errorfiles = sorted(errorfiles)

    # make corresponding argument strings
    argstrings = []
    argtags = (['- outputdir ','- year ','- flavour ',
		'- ptRatioCut ','- deepFlavorCut ','- extraCut '])
    for errorfile in errorfiles:
	outputfile = errorfile.replace('_err_', '_out_')
        with open(outputfile,'r') as f:
            lines = f.readlines()
	argstring = ''
	for line in lines[:12]:
	    for argtag in argtags:
		if( argtag in line ):
		    argstring += line.split(argtag)[-1].strip(' \t\n')+' '
	argstring = argstring.strip(' ')
	argstring = 'arguments = "runMCFakeRateMeasurement.py {}"'.format(argstring)
	argstrings.append(argstring)

    # make a new job description file
    jdfile = 'cjob_gridMCFakeRateMeasurement.txt'
    jdfilenew = 'cjob_gridMCFakeRateMeasurementResubmit.txt'
    linesnew = []
    with open(jdfile,'r') as f:
	lines = f.readlines()
    for line in lines:
	if( 'arguments = ' in line ): continue
	if( 'queue' in line ): continue
	if( len(line)<2 ): continue
	linesnew.append(line)
    for argstring in argstrings:
	linesnew.append(argstring+'\n')
	linesnew.append('queue\n\n')
    with open(jdfilenew,'w') as f:
	for line in linesnew: f.write(line)

    # printouts
    print('new job description file {} has been created.'.format(jdfilenew))
    print('if all seems good, the resubmission can be started'
	    +' by the command "condor_submit {}"'.format(jdfilenew))
