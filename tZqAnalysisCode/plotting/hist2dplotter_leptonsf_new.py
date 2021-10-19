############################################################################
# small script to call hist2dplotter functionality for lepton scalefactors #
############################################################################
# note: use this script for the new (official) version of the scalefactors,
#       i.e. after 26/02/2021

import sys
import os
import ROOT
sys.path.append(os.path.abspath('../tools'))
import histtools as ht
import hist2dplotter as p2d

if __name__=='__main__':

    outputdir = 'lepton_SF_plots/official_mediumid'
    flavours = ['muon','electron']
    years = ['2016','2017','2018']
    wp = 'Medium040' 

    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    for flavour in flavours:
	for year in years:
	    
	    histfile = '/user/llambrec/ewkino/weights/weightFiles/leptonSF/'
	    histfile += 'SFTOPLeptonID{}_{}_{}_new.root'.format(wp,year,flavour)

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
		    if hist.GetName()=='SF':
			hist.SetName('muonSF_{}'.format(year))
		    elif hist.GetName()=='SFTotStat': 
			hist.SetName('muonSF_{}_stat'.format(year))
			histtitle = histtitle+' (stat. uncertainty)'
		    elif hist.GetName()=='SFTotSys': 
			hist.SetName('muonSF_{}_syst'.format(year))
			histtitle = histtitle+' (syst. uncertainty)'
		    # (first histogram contains scale factors as bin contents)
		    # (SFTotStat contains relative (!) uncertainties as bin contents)
		    # (SFTotSys contains relative (!) uncertainties as bin contents)
		    else: continue
		    hist = p2d.swapaxes(hist)
		    p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
				histtitle=histtitle,
				logx=True,drawoptions='col z cjust text')
