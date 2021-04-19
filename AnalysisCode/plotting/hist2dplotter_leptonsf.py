############################################################################
# small script to call hist2dplotter functionality for lepton scalefactors #
############################################################################
# note: use this script for the old (preliminary) version of the scalefactors,
#       i.e. before 26/02/2021
# for new version: see hist2dplotter_leptonsf_new.py
# - for electrons, only filenames changed, file structure is exactly the same,
#   with histograms having same meaning
# - for muons, file structure changed, with new histograms with new interpretation

import sys
import os
import ROOT
sys.path.append(os.path.abspath('../tools'))
import histtools as ht
import hist2dplotter as p2d

if __name__=='__main__':

    outputdir = 'lepton_SF_plots/preliminary_mediumid'
    flavours = ['muon','electron']
    years = ['2016','2017','2018']
    wp = 'Medium040'

    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    for flavour in flavours:
	for year in years:
	    
	    histfile = '/user/llambrec/ewkino/weights/weightFiles/leptonSF/'
	    histfile += 'SFTOPLeptonID{}_{}_{}.root'.format(wp,year,flavour)

	    histlist = ht.loadallhistograms(histfile)

	    if flavour=='electron':
		for hist in histlist:
		    histtitle = 'electron scale factors'
		    if hist.GetName()=='EGamma_SF2D':
			hist.SetName('electronSF_{}'.format(year))
		    elif hist.GetName()=='stat': 
			hist.SetName('electronSF_{}_stat'.format(year))
			histtitle = histtitle+' (stat. uncertainty)'
		    elif hist.GetName()=='sys': 
			hist.SetName('electronSF_{}_syst'.format(year))
			histtitle = histtitle+' (syst. uncertainty)'
		    # (EGamma_SF2D contains scale factors as bin contents)
		    # (sys contains relative (?) uncertainties as bin contents)
		    # (stat contains relative (?) uncertainties as bin contents)
		    else: continue
		    hist = p2d.swapaxes(hist)
		    p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
				histtitle=histtitle,
				logx=True,drawoptions='col z cjust text')

	    elif flavour=='muon':
		for hist in histlist:
		    histtitle = 'muon scale factors'
		    if hist.GetName()=='NUM_LeptonMva'+wp+'_DEN_genTracks_abseta_pt':
			hist.SetName('muonSF_{}'.format(year))
		    elif hist.GetName()=='NUM_LeptonMva'+wp+'_DEN_genTracks_abseta_pt_stat': 
			hist.SetName('muonSF_{}_stat'.format(year))
			histtitle = histtitle+' (stat. uncertainty)'
			hist = p2d.geterrorhist( hist )
		    elif hist.GetName()=='NUM_LeptonMva'+wp+'_DEN_genTracks_abseta_pt_syst': 
			hist.SetName('muonSF_{}_syst'.format(year))
			histtitle = histtitle+' (syst. uncertainty)'
			hist = p2d.geterrorhist( hist )
		    # (first histogram contains scale factors as bin contents)
		    # ("_syst contains absolute (?) uncertainties as bin errors)
		    # ("_stat contains absolute (?) uncertainties as bin errors)
		    else: continue
		    hist = p2d.swapaxes(hist)
		    p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
				histtitle=histtitle,
				logx=True,drawoptions='col z cjust text')
