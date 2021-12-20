######################################################################
# calculate chi2 test between data and simulation for tZq histograms #
######################################################################
# note: this script runs on the output of histplotter_postfit_looptzq.py,
#       with the option noclean (in order to not remove the root files after making the plots)!
# note: see here for more information on the chi2 implementation:
#	https://root.cern.ch/doc/master/classTH1.html#a6c281eebc0c0a848e7a0d620425090a5

import sys
import os
import ROOT
sys.path.append('../tools')
import histtools as ht

if __name__=='__main__':

    # read command line arguments
    topdir = sys.argv[1]

    # hard-coded arguments
    regions = []
    regions.append('signalregion_cat1_yearscombined')
    regions.append('signalregion_cat2_yearscombined')
    regions.append('signalregion_cat3_yearscombined')
    variables = []
    variables.append('_eventBDT')

    # loop over regions and variables
    for region in regions:
	for variable in variables:
	    # make name of input file
	    inputdir = os.path.join(topdir, region)
	    inputfile = 'dc_combined_{}_var_{}_temp.root'.format(region, variable)
	    inputfile = os.path.join(inputdir, inputfile)
	    if not os.path.exists(inputfile):
		print('ERROR: input file {} does not exist, skippng.'.format(inputfile))
		continue
	    # read the histograms from the input file
	    f = ROOT.TFile.Open(inputfile, 'read')
	    sim = f.Get('prefit/TotalProcs')
	    sim.SetDirectory(0)
	    data = f.Get('prefit/data_obs')
	    data.SetDirectory(0)
	    f.Close()
	    # do the chi2 test
	    normchi2 = data.Chi2Test(sim, "UW P CHI2/NDF")
