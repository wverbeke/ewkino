#########################################################################
# Retrieve the cross-section of a given MC sample using its name on DAS #
#########################################################################

import os
import sys
import argparse
import fnmatch


def export_proxy(proxy):
    pass

def set_cmsenv(cmsswpath, verbose=False):
    ### set CMSSW environment from the path to a CMSSW release
    cmsswpath = os.path.join(cmsswpath,'src')
    if not os.path.exists(cmsswpath):
	print('WARNING in set_cmsenv:'
		+' path to CMSSW src {} does not exist;'.format(cmsswpath)
		+' skipping setting the cmsenv.')
	return
    cwd = os.getcwd()
    if verbose:
	print('setting CMSSW environment to {}'.format(cmsswpath))
    os.chdir(cmsswpath)
    os.system('cmsenv')
    os.chdir(cwd)

def get_cmsenv_from_path(apath, verbose=False):
    ### get the correct CMSSW environment for a generic path
    # (the path can be to anything inside a CMSSW environment)
    if verbose:
        print('getting the CMSSW environment for {}'.format(apath))
    cmssw = None
    pathparts = apath.split('/')
    for i in range(len(pathparts)-1):
        if( fnmatch.fnmatch(pathparts[i], 'CMSSW_*')
            and pathparts[i+1]=='src' ):
                cmssw = '/'.join(pathparts[:i+1])
    if( verbose and cmssw is None ):
        print('WARNING in get_cmsenv_from_path: path {}'.format(apath)
                +' does not seem to contain a CMSSW directory;'
                +' returning None.')
    return cmssw

def set_cmsenv_from_path(apath, verbose=False):
    ### set CMSSW environment from a generic path
    # (the path can be to anything inside a CMSSW environment)
    if verbose:
	print('setting the CMSSW environment for {}'.format(apath))
    cmssw = get_cmsenv_from_path(apath)
    if cmssw is None:
	print('WARNING in set_cmsenv_from_path: path {}'.format(apath)
                +' does not seem to contain a CMSSW directory;'
                +' skipping setting the cmsenv.')
    set_cmsenv(cmssw, verbose=verbose)

def dasgoclient_find_dataset_files(datasetname, verbose=False):
    ### retrieve all files belonging to a dataset using the DAS client
    if verbose: 
	print('running DAS client to find files in dataset {}...'.format(datasetname))
    dascmd = "dasgoclient -query 'file dataset={}' --limit 0".format(datasetname)
    dasstdout = os.popen(dascmd).read()
    dasfiles = [el.strip(' \t') for el in dasstdout.strip('\n').split('\n')]
    if verbose:
	print('DAS client ready; found following files ({}):'.format(len(dasfiles)))
	for f in dasfiles: print('  - {}'.format(f))
    return dasfiles

def parse_genxsecanalyzer_xsecstr(xsecstr):
    ### internal helper function
    xsecparts = xsecstr.strip(' ').split(' ')
    xsec = float(xsecparts[0])
    unc = float(xsecparts[2])
    unit = xsecparts[3]
    return (xsec,unc,unit)

def parse_genxsecanalyzer_output(rawtext):
    ### retrieve the numbers from the raw text output of genXSecAnalyzer
    # note: the output text seems to be not the same for each sample,
    #       so not all elements in the list below will be present for all samples.
    res = {}
    searchdict = ({
	'1_before_matching_and_filter': 'Before matching: total cross section = ',
	'2a_after_matching_before_filter': 'After matching: total cross section = ',
	'2b_before_filter': 'Before Filter: total cross section = ',
	'3_final_after_filter': 'After filter: final cross section = '
    })
    lines = rawtext.split('\n')
    for line in lines:
	for key,value in searchdict.items():
	    if line.startswith(value):
		xsecstr = line.replace(value,'')
		(xsec,unc,unit) = parse_genxsecanalyzer_xsecstr(xsecstr)
		res[key] = {'xsec': xsec, 'unc':unc, 'unit':unit}
    return res
   

if __name__=='__main__':

    # parse arguments
    infostr = 'sampleXSecFromDas.py: retrieve cross-section for a sample from DAS.'
    parser = argparse.ArgumentParser(description=infostr)
    parser.add_argument('--xsecana', required=True,
			help='Path to the GenXSecAnalyzer script.')
    parser.add_argument('--dasname', required=True,
			help='Name of the sample on DAS.')
    parser.add_argument('--nfiles', type=int, default=1,
			help='Number of files of this sample to run on (default: 1)')
    parser.add_argument('--nevents', type=int, default=-1,
			help='Number of events per file to use (default: all)')
    parser.add_argument('--proxy', default=None,
			help='Set the location of a valid proxy created with'
                             +' "--voms-proxy-init --voms cms";'
                             +' needed for DAS client.')
    parser.add_argument('--outfile', default=None,
			help='Output file to write summary results to.'
			     +' Default: no writing to file, just print to stdout.'
			     +' Note that the result will be appended to the bottom'
			     +' of the provided file rather than overwriting it!'
			     +' (useful for running this command in a loop.)')
    args = parser.parse_args()
    xsecana = os.path.abspath(args.xsecana)
    dasname = args.dasname
    nfiles = args.nfiles
    nevents = args.nevents
    proxy = None if args.proxy is None else os.path.abspath(args.proxy)
    outfile = None if args.outfile is None else os.path.abspath(args.outfile)

    # print configuration
    print('running with following configuration:')
    for arg in vars(args):
	print('  - {}: {}'.format(arg,getattr(args,arg)))

    # export proxy
    if proxy is not None: export_proxy( proxy )

    # make a list of all available input files
    dasfiles = dasgoclient_find_dataset_files(dasname, verbose=True)

    # select input files
    if nfiles<len(dasfiles):
	print('selecting the first {} files from the list above.'.format(nfiles))
	dasfiles = dasfiles[:nfiles]

    # set cmsenv
    set_cmsenv_from_path(xsecana, verbose=True)
    
    # make the cmsRun command
    cmd = 'cmsRun {}'.format(xsecana)
    cmd += ' inputFiles="{}"'.format(','.join(dasfiles))
    cmd += ' maxEvents={}'.format(nevents)
    print('running following command:')
    print(cmd)
    rawtext = os.popen(cmd+' 2>&1').read()
    print('raw output:')
    print(rawtext)
    res = parse_genxsecanalyzer_output(rawtext)
    print('extracted cross-section:')
    for key in sorted(res.keys()):
	print('{}: {}'.format(key,res[key]))

    # write to file
    if outfile is not None:
	sample_name = dasname.strip('/').split('/')[0]
	infostr = sample_name+'\n'
	for key in sorted(res.keys()):
	    infostr += '  {} {} {} {}\n'.format(key, res[key]['xsec'], 
			res[key]['unc'], res[key]['unit'])
	with open(outfile,'a') as f:
	    f.write(infostr)
