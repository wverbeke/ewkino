##################################################################
# Some recurring functions for training and testing TMVA methods #
##################################################################
import os
import ROOT

def gettrees(input_directory,treename,sigtag=''):
    # returns a dict with filenames and ROOT tree objects with given name in the file.
    # if sigtag = '', a single dict is returned.
    # else, separate dicts are returned for signal (i.e. tag in filename) and background 
    # WARNING: this function seems to read only a part of the trees, do not use until fixed.
    filelist = [f for f in os.listdir(input_directory) if f[-5:]=='.root']
    sigtreedict = {}
    bcktreedict = {}
    for f in filelist:
	tfile = ROOT.TFile.Open(os.path.join(input_directory,f))
	ttree = tfile.Get(treename)
	if ttree.GetEntries()==0: continue # skip empty trees
	ttree.GetEntry(0) # this seems to be necessary to read the tree properly...
	ttree.SetDirectory(0)
	if sigtag in f:
	    sigtreedict[f] = ttree
	else:
	    bcktreedict[f] = ttree
	tfile.Close()
    if sigtag=='':
	return sigtreedict.update(bcktreedict)
    return (sigtreedict,bcktreedict)
