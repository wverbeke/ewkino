#################################################################################
# a function to read a samplelist and compare it to the contenst of a directory #
#################################################################################
from readsamplelist import readsamplelist
import os

def extendsamplelist(samplelist_name,directory):
    # returns a dictionary of files analogous to the result of readsamplelist,
    # but with an extra key 'file' pointing to the location of the file in directory
    inputfiles = readsamplelist(samplelist_name,unique=True)
    inputfilesnew = []
    missinglist = []
    for f in inputfiles:
	fname = f['sample_name']
	indir = False
	for findir in os.listdir(directory):
	    if fname in findir:
		indir = True
		f['file'] = os.path.join(directory,findir)
		inputfilesnew.append(f)
		break
	if not indir: missinglist.append(fname)
    if len(missinglist)>0:
	print('### WARNING ###: the following samples are in sample list but not in input directory:')
	print(missinglist)
    return inputfilesnew

### test section ###
if __name__=="__main__":
    # command line args: samplelist_name directory
    import sys
    sl = extendsamplelist(sys.argv[1],sys.argv[2])
    for s in sl:
	print(s)
