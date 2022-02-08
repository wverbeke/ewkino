##############################################
# merge files for 2016PreVFP and 2016PostVFP #
##############################################
# merge two files supposed to contain the same type of histograms, 
# but for 2016PreVFP and 2016PostVFP respectively.
# the histograms are simply added, but after replacing any occurence of '2016PreVFP'
# or '2016PostVFP' in the histogram name by '2016Merged'

# use e.g. in fitFakeRateMeasurement.py, after hadding the files for different samples

import sys
import os
import ROOT
sys.path.append('../Tools/python')
import histtools as ht

def merge2016( prevfpfile, postvfpfile, outputfile=None ):
    # returns a list of merged histograms from both files.
    # additionally, if outputfile is not None, the histograms are written to it.
    prehistlist = ht.loadallhistograms( prevfpfile )
    posthistlist = ht.loadallhistograms( postvfpfile )
    for hist in prehistlist:
	prename = hist.GetName()
	postname = prename.replace('2016PreVFP','2016PostVFP')
	mergedname = prename.replace('2016PreVFP','2016Merged')
	thisposthistlist = []
	for posthist in posthistlist:
	    if posthist.GetName()==postname: thisposthistlist.append(posthist)
	if len(thisposthistlist)!=1:
	    msg = 'ERROR in merge2016:'
	    msg += ' found {} 2016PostVFP histograms'.format(len(thisposthistlist))
	    msg += ' corresponding to 2016PreFit histogram {}'.format(prename)
	    msg += ' while expecting exactly 1. Candidates are:\n'
	    for hist in thisposthistlist: msg += '  - {}\n'.format(hist.GetName())
	    raise Exception(msg)
	hist.Add( thisposthistlist[0] )
	hist.SetName( mergedname )
	hist.SetTitle( mergedname )
    if outputfile is None: return prehistlist
    f = ROOT.TFile.Open(outputfile,'recreate')
    for hist in prehistlist: hist.Write()
    f.Close()
    return prehistlist


if __name__=='__main__':

    prevfpfile = sys.argv[1]
    postvfpfile = sys.argv[2]
    outputfile = sys.argv[3]
    
    merge2016( prevfpfile, postvfpfile, outputfile=outputfile )
