#################################################################################
# a function to read a samplelist and compare it to the contenst of a directory #
#################################################################################
from readsamplelist import readsamplelist
import os

def extendsamplelist( samplelistpath, directory ):
    # returns a dictionary of files analogous to the result of readsamplelist,
    # but with an extra key 'file' pointing to the location of the file in directory
    # input arguments:
    # - samplelistpath: either string or list of strings representing path(s) to sample list file(s)
    # - directory: path to a directory where the samples should be
    inputfiles = readsamplelist( samplelistpath, unique=True )
    inputfilesnew = []
    missinglist = []
    available = [os.path.join(directory,f) for f in os.listdir(directory) if f[-5:]=='.root']
    # loop over all files in sample list
    for f in inputfiles:
	fname = f['sample_name']
	# check if sample is in the directory
	indir = False
	for findir in available:
	    if fname in findir:
		indir = True
		f['file'] = findir
		inputfilesnew.append(f)
		available.remove(findir)
		break
	if not indir: missinglist.append(fname)
    # print missing samples
    if len(missinglist)>0:
	print('### WARNING ###: the following samples are in sample list but not in input directory:')
	for el in missinglist: print(el)
    # print samples that are in directory but not in samplelist
    if len(available)>0:
	print('### WARNING ###: the following samples are in input directory but not in sample list:')
	for el in available: print(el)
    return inputfilesnew

def findsample(sample_name,sample_list):
    # find the dictionary entry corresponding to a given sample name in a sample list
    for fdict in sample_list:
	if fdict['sample_name']==sample_name:
	    return fdict
    print('### WARNING ###: sample '+sample_name+' not found in given sample list.')
    return None

### test section ###
if __name__=="__main__":
    # command line args: samplelistpath directory
    import sys
    sl = extendsamplelist(sys.argv[1],sys.argv[2])
    for s in sl:
	print(s)
