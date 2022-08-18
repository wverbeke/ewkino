################################################
# check the error and output log files of jobs #
################################################

import sys
import os


def check_start_done(filename):
    # returns 0 if the number of done tags equals the number of starting tags,
    # and if both are present at least once in the file;
    # returns 1 otherwise.

    # read the file content
    f = open(filename)
    filetext = f.read()
    f.close()
    
    # count number of starting tags
    nstarted = filetext.count('###starting###')
    if(nstarted==0):
        print('### WARNING: file '+filename+' contains no valid starting tag.')
        print('             does the process write the correct tags to the error files?')
        return 1

    # count number of done tags
    ndone = filetext.count('###done###')
    if(nstarted==ndone): return 0

    # print result
    print('found issue in file '+filename+':')
    print('   '+str(nstarted)+' commands were initiated.')
    print('   '+str(ndone)+' seem to have finished normally.')
    return 1


def check_content(filename, contentlist):
    # returns 0 if none of the elements of contentlist is present in the file;
    # returns 1 otherwise.

    # read the file content
    f = open(filename)
    filetext = f.read()
    f.close()

    # check if the file content contains provided error tags
    contains = False
    for content in contentlist:
	if filetext.count(content)>0:
	    contains = True
	    print('found issue in file '+filename+':')
	    print('   file contains the sequence '+content+' which was flagged as problematic.')
    if contains: return 1
    return 0


if __name__=='__main__':

    # print available options
    print('available options from the command line:')
    print('- ignoreQsub: ignore qsub error log files')
    print('- ignoreCondor: ignore condor error log files')
    print('- ignoreStartingTags: do not check starting and done tags')
    print('- ignoreErrorTags: do not check error tags')

    # read command line args
    ignoreQsub = False
    ignoreCondor = False
    ignoreStartingTags = False
    ignoreErrorTags = False
    for rawarg in sys.argv[1:]:
	arg = rawarg.strip(' -')
	if arg=='ignoreQsub': ignoreQsub = True
	if arg=='ignoreCondor': ignoreCondor = True
	if arg=='ignoreStartingTags': ignoreStartingTags = True
	if arg=='ignoreErrorTags': ignoreErrorTags = True

    # hard-coded arguments
    errortags = ([  'SysError',
		    '/var/torque/mom_priv/jobs',
		    'R__unzip: error',
		    'hadd exiting due to error in',
		    'Bus error' ])
    errortags.append('###error###') # custom error tag for flagging

    files = []
    # find qsub error log files
    if not ignoreQsub: files = [fname for fname in os.listdir(os.getcwd()) if '.sh.e' in fname]
    # find condor error log files
    if not ignoreCondor: files += [fname for fname in os.listdir(os.getcwd()) if '_err_' in fname]
    print('found '+str(len(files))+' error log files.')
    print('start scanning...')

    # loop over files
    nfiles = len(files)
    nerror = 0
    for fname in files:
	# initialize
	error_start_done = 0
	error_content = 0
	# error checking
	if not ignoreStartingTags: error_start_done = check_start_done(fname)
	if not ignoreErrorTags: error_content = check_content(fname, errortags)
	if(error_start_done + error_content > 0): nerror += 1

    # print results
    print('number of files scanned: {}'.format(nfiles))
    print('number of files with error: {}'.format(nerror))
    print('number of files without apparent error: {}'.format(nfiles-nerror))
