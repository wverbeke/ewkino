#####################################################################################
# extract a small test file from a regular histogram file used as input for combine #
#####################################################################################

import os
import sys
import ROOT
sys.path.append(os.path.abspath('../tools'))
import histtools as ht

if __name__=='__main__':

    sourcefile = sys.argv[1]
    outfile = sys.argv[2]
    processes = ['all'] # put ['all'] to keep all processes
    variables = ['_eventBDT'] # put ['all'] to keep all variables
    systematics = ['all'] # put ['all'] to keep all systematics

    mustcontainone = []
    if systematics!=['all']: mustcontainone = systematics+['nominal']
    elif variables!=['all']: mustcontainone = variables
    elif processes!=['all']: mustcontainone = processes

    histlist = ht.loadhistograms(sourcefile,mustcontainone=mustcontainone)
    if variables!=['all']: histlist = ht.selecthistograms(histlist,mustcontainone=variables)[1]
    if processes!=['all']: histlist = ht.selecthistograms(histlist,mustcontainone=processes)[1]

    f = ROOT.TFile.Open(outfile,'recreate')
    for h in histlist: h.Write()
    f.Close()
