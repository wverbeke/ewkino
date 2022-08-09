###################################################
# a python looper for plotConeCorrectionFactor.cc #
###################################################

import os
import sys
import glob
import ROOT
sys.path.append('../plotting/python')
import multihistplotter as mhp

# global settings
mvadict = ({'leptonMVATOPUL':
                {
                    'electron':
                        { 
                          'VLoose': 0.20,
                          'Loose': 0.41,
                          'Medium': 0.64,
                          'Tight': 0.81 
                        },
                    'muon':
                        {
                          'VLoose': 0.20, 
                          'Loose': 0.41,
                          'Medium': 0.64,
                          'Tight': 0.81
                        }
                }
            })
# (see fillConeCorrectionFactor.py!)
years = ['2016PreVFP','2016PostVFP','2017','2018']
# (see fillConeCorrectionFactors.py!)
plotmode = 'new'
# (choose from 'old' for deprecated c++ plotting function,
#  or 'new' for newer python equivalent)

def extract_cone_correction_factor_diff(histogram, mvathreshold):
    ### internal helper function
    # extract the cone correction factor from a histogram
    # by equalling bins across the threshold.
    # the histogram is supposed to contain the average lepton pt
    # as a function of lepton mva.
    
    bin_index = histogram.FindBin(mvathreshold)
    above_threshold = histogram.GetBinContent( bin_index )
    below_threshold = histogram.GetBinContent( bin_index-1 )
    cone_factor = above_threshold/below_threshold
    return cone_factor

def extract_cone_correction_factor_fit(histogram, mvathreshold):
    ### internal helper function
    # to do
    return None

def get_average_pt_histogram(	pt_weighted_lepton_mva_histogram,
				lepton_mva_histogram ):
    ### internal helper function
    # make average pt histogram as a function of lepton mva
    # by dividing the pt-weighted histogram by the normal one.
    
    # remove errors on denominator to avoid double counting
    nbins = lepton_mva_histogram.GetNbinsX()
    for i in range(0,nbins+2):
	lepton_mva_histogram.SetBinError(i, 0.)
    # do the division
    average_pt_histogram = pt_weighted_lepton_mva_histogram.Clone()
    average_pt_histogram.Divide( lepton_mva_histogram )
    # return the result
    return average_pt_histogram

def extract_cone_correction_factor(filename, mvathreshold):
    # load the histograms
    rfile = ROOT.TFile.Open(filename)
    lepton_mva_hist = rfile.Get("leptonMVA")
    lepton_mva_hist.SetDirectory(0)
    pt_weighted_lepton_mva_hist = rfile.Get("pTLeptonMVA")
    pt_weighted_lepton_mva_hist.SetDirectory(0)
    rfile.Close()
    # get the average pt histogram
    average_pt_hist = get_average_pt_histogram( pt_weighted_lepton_mva_hist,
			    lepton_mva_hist )
    # extract the cone correction factor
    ccf = extract_cone_correction_factor_diff( average_pt_hist, mvathreshold )
    return ccf, average_pt_hist
    

if __name__=='__main__':

    # make output directory
    outputdir = 'coneCorrectionPlots'
    if not os.path.exists(outputdir): os.makedirs(outputdir)

    # hadd files if needed
    print('hadding files if needed...')
    for year in years:
	for leptonmva in sorted(mvadict.keys()):
	    for flavour in sorted(mvadict[leptonmva].keys()):
		for wpname in sorted(mvadict[leptonmva][flavour].keys()):
		    # define target file
		    wpvalue = mvadict[leptonmva][flavour][wpname]
		    filename = 'coneCorrectionFactor_'+leptonmva+'_'+wpname+'_'
		    filename += flavour+'_'+year+'_histograms.root'
		    # check if target file exists
		    if os.path.exists(filename):
			print('file {} already exists, skipping...'.format(filename))
			continue
		    print('file {} not yet found, doing hadd...'.format(filename))
		    # define source files
		    haddyears = year
		    if year=='all': haddyears = '*'
		    haddsource = 'coneCorrectionFactor_'+leptonmva+'_'+wpname+'_'
		    haddsource += flavour+'_'+haddyears+'_histograms_sample*.root'
		    # check if source files exist
		    if len(glob.glob(haddsource))==0:
			print('required source files for {} do not exist, skipping...'.format(filename))
			continue
		    cmd = 'hadd {} {}'.format(filename, haddsource)
		    print('executing {}'.format(cmd))
		    os.system(cmd)

    # loop over flavours, years and leptonmvas
    cwd = os.getcwd()
    for year in years:
        for leptonmva in sorted(mvadict.keys()):
            for flavour in sorted(mvadict[leptonmva].keys()):
                for wpname in sorted(mvadict[leptonmva][flavour].keys()):
                    wpvalue = mvadict[leptonmva][flavour][wpname]
		    # check if the correct file exists
		    filename = 'coneCorrectionFactor_'+leptonmva+'_'+wpname+'_'
		    filename += flavour+'_'+year+'_histograms.root'
		    if not os.path.exists(filename):
			print('ERROR: file '+filename+' not found, skipping it.')
			continue
		    
		    # case of old plotting mode
		    if plotmode=='old':
			exe = './plotConeCorrectionFactor'
			if not os.path.exists(exe):
			    raise Exception('ERROR: executable {} does not exist.'.format(exe))
			command = exe + ' {} {} {} {} {}'.format(
					flavour, year, leptonmva, wpname, wpvalue)
			print('executing {}...'.format(command))
			os.system(command)
	
		    # case of new plotting mode
		    if plotmode!='new': continue
		    # get the histogram and correction factor
		    ccf, average_pt_hist = extract_cone_correction_factor(filename, wpvalue)
		    print('=== cone correction factor for {} {} {} {}:'.format(leptonmva, wpname, year, flavour))
		    print(ccf)

		    # apply the cone correction factor
		    postcor_hist = average_pt_hist.Clone()
		    nbins = postcor_hist.GetNbinsX()
		    bin_index = postcor_hist.FindBin(wpvalue)
		    for i in range(0, bin_index):
			postcor_hist.SetBinContent( i, average_pt_hist.GetBinContent(i)*ccf )
	
		    # plot settings
		    histlist = [average_pt_hist, postcor_hist]
		    labellist = ['Without correction factor', 'With correction factor']
		    colorlist = [ROOT.kBlue-4, ROOT.kViolet+1]
		    extracmstext = '#splitline{Preliminary,}{Simulation}'
		    extrainfos = (['MVA: {}'.format(leptonmva),
				    'WP: {} ({})'.format(wpname,wpvalue), 
				    'Flavour: {}'.format(flavour), 
				    'Year: {}'.format(year.replace('all','all years')),
				    'Correction factor: {:.3f}'.format(ccf)])
		    figname = os.path.join(outputdir, 'coneCorrectionFactor_{}_{}_{}_{}.png'.format(
				leptonmva, wpname, flavour, year))

		    # make plot
		    mhp.plotmultihistograms( histlist, figname=figname,
					 labellist=labellist, colorlist=colorlist,
					 extracmstext=extracmstext,
					 extrainfos=extrainfos, infosize=15, infoleft=0.16,
					 ymaxlinfactor=1.5,
					 drawoptions='hist e' )	
