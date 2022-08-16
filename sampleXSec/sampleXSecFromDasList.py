###############################################################################
# Retrieve the cross-sections of a list of MC sample using their names on DAS #
###############################################################################
# extension of sampleXSecFromDas.py:
# - use a list of sample names instead of a single sample name
# - run in a job instead of locally (optional)
# - parse the output in an overview table

import os
import sys
import argparse
sys.path.append('../jobSubmission')
import condorTools as ct
import sampleXSecFromDas as xsec


def read_daslist(daslist, verbose=False):
    ### read a sample list containing DAS dataset names
    dasnames = []
    if not os.path.exists(daslist):
	raise Exception('ERROR in read_daslist: file {} does not exist.'.format(daslist))
    with open(daslist, 'r') as f:
	lines = f.readlines()
    for line in lines:
	valid = True
	line = line.strip(' \t\n')
	if( line.startswith('#') ): continue
	if( line.count('/')!=3 ): valid = False
	if not valid:
	    print('WARNING in read_daslist:'
		    +' line {} was found to be invalid;'.format(line)
		    +' will skip this line.')
	    continue
	dasnames.append(line)
    return dasnames


if __name__=='__main__':

    # parse arguments
    infostr = 'sampleXSecFromDasList.py: retrieve cross-sections for a sample list from DAS.'
    parser = argparse.ArgumentParser(description=infostr)
    parser.add_argument('--xsecana', required=True,
			help='Path to the GenXSecAnalyzer script.')
    parser.add_argument('--daslist', required=True,
			help='Path to a sample list containing sample names on DAS.')
    parser.add_argument('--runmode', default='condor',
			help='Choose from "condor" (for job submission)'
			     +' or "local" (for running locally).')
    parser.add_argument('--nfiles', type=int, default=1,
			help='Number of files per sample to run on (default: 1)')
    parser.add_argument('--nevents', type=int, default=-1,
			help='Number of events per file to use (default: all)')
    parser.add_argument('--proxy', default=None,
			help='Set the location of a valid proxy created with'
			     +' "--voms-proxy-init --voms cms";'
                             +' needed for DAS client.')
    parser.add_argument('--outfile', default=None,
			help='Output txt file where to write the summary to.')
    args = parser.parse_args()
    xsecana = os.path.abspath(args.xsecana)
    daslist = args.daslist
    runmode = args.runmode
    nfiles = args.nfiles
    nevents = args.nevents
    proxy = None if args.proxy is None else os.path.abspath(args.proxy)
    outfile = None if args.outfile is None else os.path.abspath(args.outfile)

    # print configuration
    print('running with following configuration:')
    for arg in vars(args):
	print('  - {}: {}'.format(arg,getattr(args,arg)))

    # create output file
    if outfile is not None:
	if os.path.exists(outfile):
	    raise Exception('ERROR: output file {} already exists.'.format(outfile)
			    +' Choose another filename or delete the existing file.')
	dirname = os.path.dirname(outfile)
	if not os.path.exists(dirname): os.makedirs(outfile)
	print('creating output file...')
	cmd = 'echo "=== GenXSecAnalyzer cross-section overview ===" >> {}'.format(outfile)
	os.system(cmd)

    # read the sample names
    dasnames = read_daslist(daslist)
    
    # loop over samples
    cmds = []
    for dasname in dasnames:
	# make the command
	cmd = 'python sampleXSecFromDas.py'
	cmd += ' --xsecana {}'.format(xsecana)
	cmd += ' --dasname {}'.format(dasname)
	cmd += ' --nfiles {}'.format(nfiles)
	cmd += ' --nevents {}'.format(nevents)
	if proxy is not None: cmd += ' --proxy {}'.format(proxy)
	if outfile is not None: cmd += ' --outfile {}'.format(outfile)
	cmds.append(cmd)

    # run the commands
    if( runmode=='local' ):
	for cmd in cmds: os.system(cmd)
    if( runmode=='condor' ):
	ct.submitCommandsAsCondorJob('cjob_sampleXSecFromDasList',
				     cmds, proxy=proxy)
