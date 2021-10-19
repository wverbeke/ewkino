##########################################
# rename the histograms in the test file #
##########################################
# (to test if unwanted correlations are causing issues)

import sys
import os
import ROOT
sys.path.append(os.path.join('../tools'))
import histtools as ht

if __name__=='__main__':

    infile = sys.argv[1]
    outfile = sys.argv[2]

    histlist = ht.loadallhistograms(infile)
    newhistlist = []
    for hist in histlist:
	if 'JECAll_' in hist.GetName():
	    newname = hist.GetName().replace('JECAll_','JECAll')
	    hist.SetName(newname)
	newhistlist.append(hist)

    f = ROOT.TFile.Open(outfile,'recreate')
    for hist in newhistlist:
	hist.Write()
    f.Close()
