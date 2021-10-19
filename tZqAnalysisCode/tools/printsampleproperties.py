######################################################################################
# loop over all samples in a directory and print values for a small number of events #
######################################################################################

import os
import sys
import ROOT

def printbranches(tree,branches,nevents):
    
    for i in range(min(tree.GetEntries(),nevents)):
	tree.GetEntry(i)
	print('----- EVENT -----')
	for branch in branches:
	    print('branch: {}, value: {}'.format(branch,getattr(tree,branch)))

if __name__=='__main__':
    
    indir = os.path.abspath(sys.argv[1])
    files = [os.path.join(indir,f) for f in os.listdir(indir) if f[-5:]=='.root']
    branches = ['_nL','_nPsWeights','_psWeight']
    nevents = 5

    for f in files:
	print('===== FILE: {} ====='.format(f))
	rf = ROOT.TFile.Open(f)
	tree = rf.Get('blackJackAndHookers/blackJackAndHookersTree')
	printbranches(tree,branches,nevents)
