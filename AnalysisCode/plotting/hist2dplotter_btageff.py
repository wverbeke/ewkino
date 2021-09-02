###############################################################################
# small script to call hist2dplotter functionality for b-tagging efficiencies #
###############################################################################

import sys
import os
import ROOT
sys.path.append(os.path.abspath('../tools'))
import histtools as ht
import hist2dplotter as p2d

if __name__=='__main__':

    outputdir = 'plots_test'
    years = ['2016','2017','2018']

    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    for year in years:
	    
	    histfile = '/user/llambrec/ewkino/weights/weightFiles/bTagEff/'
	    histfile += 'bTagEff_deepFlavor_looseLeptonCleaned_{}.root'.format(year)

	    histlist = ht.loadallhistograms(histfile)

	    for hist in histlist:
		histtitle = ''
		if 'beauty' in hist.GetName(): histtitle = 'b-jet identification efficiency'
                elif 'charm' in hist.GetName(): histtitle = 'c-jet misidentification rate'
                elif 'uds' in hist.GetName(): histtitle = 'light jet misidentification rate'
		else: 
		    print('WARNING: histogram {} not recognized;'.format(hist.GetName())
			    +' skipping it.')
		figname = hist.GetName()+'_'+str(year)
		p2d.plot2dhistogram(hist,os.path.join(outputdir,figname),
				histtitle=histtitle,
				logx=True,drawoptions='col z cjust text269 e')
