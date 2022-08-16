############################################################################
# tool for splitting skimmed samples into smaller parts for easier merging #
############################################################################

# Typical use case: you have a large skimmed sample
# (e.g. as produced by skimTuplesFromList.py)
# consisting of multiple thousands of files.
# Merging this sample is slow and might get stuck sometimes.
# This script splits the sample into parts (e.g. per 500 files)
# so merging can be done more easily on the parts.
# Afterwards, the parts can be merged if needed 
# (but sometimes for MC studies this is not even needed).


import sys
import os


def is_skimmed_dir(directory):
    return (directory.startswith('ntuples_skimmed')
	    and '_version_' in directory)

def get_file_nb(filename, tag):
    nbstr = filename.split(tag)[-1].replace('.root','')
    return int(nbstr)

def get_max_file_nb(directory, tag):
    fnbs = [get_file_nb(f, tag) for f in os.listdir(directory)]
    return sorted(fnbs)[-1]


if __name__=='__main__':

    # read command line arguments
    sampledir = sys.argv[1]
    # (skimmed sample directory (i.e. ntuples_skimmed_...)
    #  OR directory containing multiple skimmed samples)
    nfilesperpart = sys.argv[2]
    # (number of files per part to split into)

    # hardcoded arguments (for now)
    tag = 'singlelep_'

    # parse arguments
    sampledir = sampledir.rstrip('/')
    nfilesperpart = int(nfilesperpart)

    # make list of samples
    samples = []
    samplebasedir = os.path.basename(sampledir)
    if is_skimmed_dir(samplebasedir):
	samples = [sampledir]
	sampledir = os.path.dirname(sampledir)
    else:
	for el in os.listdir(sampledir):
	    if is_skimmed_dir(el):
		samples.append( os.path.join(sampledir,el) )
    print('found following list of samples:')
    for sample in samples:
	sname = os.path.basename(sample)
	print('  - {}'.format(sname))
    print('in directory {}'.format(sampledir))

    # make list of number of files and parts per sample
    nfiles = []
    maxfilenbs = []
    nparts = []
    for sample in samples:
	nfiles.append( len(os.listdir(sample)) )
	maxfilenbs.append( get_max_file_nb(sample,tag) )
	nparts.append( int(maxfilenbs[-1]/nfilesperpart)+1 )

    # do printouts for checking
    print('will split samples as follows:')
    for sample,nfile,maxfnb,npart in zip(samples,nfiles,maxfilenbs,nparts):
	sname = os.path.basename(sample)
	partname = npart if npart > 1 else 'no split'
	print('  - {} ({} files, {} max nb) -> {} parts'.format(
		sname,nfile,maxfnb,partname))
    print('continue? (y/n)')
    go = raw_input()
    if go!='y': sys.exit()

    # do the splitting
    for sample,npart in zip(samples,nparts):
	if npart < 2: continue
	sname = os.path.basename(sample)
	print('now processing sample {}'.format(sname))
	# rename the original directory
	firstpart = os.path.join(sampledir,sname.replace('_version_','_part1_version_'))
	os.system('mv {} {}'.format(sample,firstpart))
	# make other parts
	movedict = {}
	for partn in range(2,npart+1):
	    partname = firstpart.replace('_part1_version','_part{}_version'.format(partn))
	    if not os.path.exists(partname): os.makedirs(partname)
	    movedict[partn] = partname
	# move files to correct part
	for fname in os.listdir(firstpart):
	    fnb = get_file_nb(fname, tag)
	    npart = int(fnb/nfilesperpart)+1
	    if npart==1: continue
	    os.system('mv {} {}'.format(os.path.join(firstpart,fname),movedict[npart]))

    # do some checks
    print('doing some checks...')
    samplespost = []
    for el in os.listdir(sampledir):
        if is_skimmed_dir(el):
            samplespost.append( os.path.join(sampledir,el) )
    nfilespost = []
    for sample in samplespost:
        nfilespost.append( len(os.listdir(sample)) )
    print('samples after moving:')
    for sample,nfile in zip(samplespost,nfilespost):
	sname = os.path.basename(sample)
        print('  - {}: {} files'.format(sname,nfile))
