################################################################
# read pulls and constraints from a fitDiagnostics output file #
################################################################

import os
import sys
import ROOT

def readpulls( filename, whichfit ):
    # note: whichfit is either 'nuisances_prefit_res', 'fit_b' or 'fit_s'
    # code adapted from Gianny

    pullsAndConstraints = {}
    resultsFile = ROOT.TFile( filename )
    fitResults  = resultsFile.Get( whichfit ).floatParsFinal()
    for r in [fitResults.at(i) for i in range(fitResults.getSize())]:
	pullsAndConstraints[r.GetName()] = (r.getVal(), r.getAsymErrorHi(), r.getAsymErrorLo())

    return pullsAndConstraints

if __name__=='__main__':

    infile = sys.argv[1]
    whichfit = sys.argv[2]
    pulls = readpulls( infile, whichfit )
    pullsnames = sorted(pulls.keys())
    print('pulls for {}'.format(infile))
    for p in pullsnames:
	print(' - '+p+': '+str(pulls[p]))
