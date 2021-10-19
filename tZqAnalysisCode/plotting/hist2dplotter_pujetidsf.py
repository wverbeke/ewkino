###############################################################################
# small script to call hist2dplotter functionality for PU jet ID scalefactors #
###############################################################################
import sys
import os
import ROOT
import hist2dplotter as p2d
sys.path.append(os.path.abspath('../tools'))
import histtools as ht

if __name__=='__main__':

    outputdir = 'PUJetIDSFplot'
    wps = ['L'] # choose from L, M, T
    flavours = ['eff','mistag'] # choose from eff, mistag
    years = ['2016','2017','2018']

    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    efffile = '/user/llambrec/ewkino/weights/weightFiles/pileupJetIdEff/'
    efffile += 'effcyPUID_81Xtraining.root'
    sffile = '/user/llambrec/ewkino/weights/weightFiles/pileupJetIdSF/'
    sffile += 'scalefactorsPUID_81Xtraining.root'

    effhistlist = ht.loadallhistograms(efffile)
    effnamelist = [hist.GetName() for hist in effhistlist]
    sfhistlist = ht.loadallhistograms(sffile)
    sfnamelist = [hist.GetName() for hist in sfhistlist]
    
    for wp in wps:
	for flavour in flavours:
	    for year in years:
	   
		print('processing WP {}, flavor{}, year {}'.format(wp,flavour,year))

		# find efficiency histogram
		name = 'h2_{}_mc{}_{}'.format(flavour,year,wp)
		index = effnamelist.index(name)
		print(index)
		hist = effhistlist[index]
		p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
				    histtitle=hist.GetTitle())

		# find scalefactor histogram
		name = 'h2_{}_sf{}_{}'.format(flavour,year,wp)
                index = sfnamelist.index(name)
                print(index)
		hist = sfhistlist[index]
                p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
                                    histtitle=hist.GetTitle())

		# find scalefactor uncertainty histogram
		name = 'h2_{}_sf{}_{}_Systuncty'.format(flavour,year,wp)
                index = sfnamelist.index(name)
                print(index)
		hist = sfhistlist[index]
                p2d.plot2dhistogram(hist,os.path.join(outputdir,hist.GetName()),
                                    histtitle=hist.GetTitle())

sys.exit()
	    
'''if flavour=='electron':
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
				logx=True,drawoptions='col z cjust text')'''

