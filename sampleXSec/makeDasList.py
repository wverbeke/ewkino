#####################################################
# Small tool for making a list of DAS dataset names #
#####################################################
# Note: this script might depend on the ewkino framework naming convention
#       (depending on the input source type)!

import sys
import os
import argparse
import fnmatch


def read_ntuplizer_sample_list(slist):
    dasnames = []
    with open(slist,'r') as f:
	lines = f.readlines()
    for line in lines:
	line = line.strip(' \t\n')
	if not ':' in line: continue
	dasnames.append(line.split(':')[1])
    return dasnames

def read_ewkino_sample_list(slist):
    pass

def read_sample_dir(sdir):
    pass


if __name__=='__main__':

    # parse arguments
    infostr = 'makeDasList.py: make list of DAS sample names.'
    parser = argparse.ArgumentParser(description=infostr)
    parser.add_argument('--sourcetype', required=True,
			help='Type of the input source.'
			     +' Choose from "ntuple" (for ntuplizer-style sample list),'
			     +' "samplelist" (for ewkino-style sample list),'
			     +' or "dir" (for all samples in a directory).'
			     +' The recommended is "ntuple" as it is the least ambiguous.')
    parser.add_argument('--source', required=True,
                        help='Source to get the sample names from.'
			     +' Can either be a directory or a sample list file'
			     +' (see above).')
    parser.add_argument('--filter', default=None,
                        help='Filter to select sample names;'
			     +' may contain unix-style wildcards.')
    parser.add_argument('--outfile', default=None,
			help='Output file to write the DAS sample names to.'
			     +' Default: no output file, just print to screen.')
    args = parser.parse_args()
    sourcetype = args.sourcetype
    source = args.source
    sfilter = getattr(args,'filter') # apparently a reserved word in python
    outfile = None if args.outfile is None else os.path.abspath(args.outfile)

    # print configuration
    print('running with following configuration:')
    for arg in vars(args):
        print('  - {}: {}'.format(arg,getattr(args,arg)))

    # check output file
    if outfile is not None:
	if os.path.exists(outfile):
	    raise Exception('ERROR: output file {} already exists.'.format(outfile)
			    +' Choose another filename or delete the existing file.')
	dirname = os.path.dirname(outfile)
	if not os.path.exists(dirname): os.makedirs(dirname)

    # read the samples
    dasnames = []
    if sourcetype=='ntuple':
	dasnames = read_ntuplizer_sample_list(source)
    elif sourcetype=='samplelist':
	raise Exception('ERROR: not yet implemented')
    elif sourcetype=='dir':
	raise Exception('ERROR: not yet implemented')
    else:
	raise Exception('ERROR: source type {} not recognized.'.format(sourcetype))

    # apply filter
    if sfilter is not None:
	newdasnames = []
	for dasname in dasnames:
	    if not fnmatch.fnmatch(dasname, sfilter): continue
	    newdasnames.append(dasname)
	dasnames = newdasnames

    # print the result
    print('found following DAS samples:')
    for dasname in dasnames:
	print('  - {}'.format(dasname))

    # write to file
    if outfile is None:
	print('WARNING: no output file provided, results printed only to stdout')
    else:
	with open(outfile,'w') as f:
	    for dasname in dasnames:
		f.write(dasname+'\n')
	print('results written to {}'.format(outfile))
