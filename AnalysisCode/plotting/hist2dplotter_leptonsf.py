############################################################################
# small script to call hist2dplotter functionality for lepton scalefactors #
############################################################################
import sys
import os
import ROOT
import hist2dplotter as p2d

if __name__=='__main__':

    outputdir = 'leptonSFplot'
    flavours = ['muon','electron']
    years = ['2016','2017','2018']

    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    for flavour in flavours:
	for year in years:
	    
	    histfile = '/user/llambrec/ewkino/weights/weightFiles/leptonSF/'
	    histfile += 'SFTOPLeptonIDTight_{}_{}.root'.format(year,flavour)

	    histlist = p2d.loadhistograms(histfile)

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
		    else: continue
		    hist = p2d.swapaxes(hist)
		    p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
				histtitle=histtitle,
				logx=True,drawoptions='col z cjust text')

	    elif flavour=='muon':
		for hist in histlist:
		    histtitle = 'muon scale factors'
		    if hist.GetName()=='NUM_LeptonMvaTight_DEN_genTracks_abseta_pt':
			hist.SetName('muonSF_{}'.format(year))
		    elif hist.GetName()=='NUM_LeptonMvaTight_DEN_genTracks_abseta_pt_stat': 
			hist.SetName('muonSF_{}_stat'.format(year))
			histtitle = histtitle+' (stat. uncertainty)'
			hist = p2d.geterrorhist( hist )
		    elif hist.GetName()=='NUM_LeptonMvaTight_DEN_genTracks_abseta_pt_syst': 
			hist.SetName('muonSF_{}_syst'.format(year))
			histtitle = histtitle+' (syst. uncertainty)'
			hist = p2d.geterrorhist( hist )
		    else: continue
		    hist = p2d.swapaxes(hist)
		    p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
				histtitle=histtitle,
				logx=True,drawoptions='col z cjust text')

