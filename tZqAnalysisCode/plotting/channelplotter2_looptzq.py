############################################################
# caller to channelplotter2 with settings for tzq analysis #
############################################################

import sys
import os
from channelplotter2 import readchanneltxt, plotchannels

if __name__=='__main__':

    thfile = '../newcombine/datacards_rescaledlumi_topratio_plots/datacards_rescaledlumi_topratio_mc_ext.txt'
    exfile = '../newcombine/datacards_rescaledlumi_topratio_plots/datacards_rescaledlumi_topratio_obs_ext.txt'
    figfile = '../newcombine/datacards_rescaledlumi_topratio_plots/summary_figure'
    
    channels = readchanneltxt([thfile,exfile])
    thchannels = channels[0]
    exchannels = channels[1]

    thbandlabels = ['Theory']
    exbandlabels = ['Stat. unc.', 'Total unc.']

    xaxrange = (0.,1.8)
    xaxlinecoords = [0.6,0.8,1.2,1.4,1.6]
    yaxlinecoords = [0.5]
    for i in range(len(thchannels)):
	yaxlinecoords.append(1.5+i)
    legendbox = [0.65, 0.72, 0.96, 0.90]
    lumi = 138

    plotchannels( thchannels, exchannels, figfile, 
		    thbandlabels=thbandlabels,exbandlabels=exbandlabels,
		    xaxrange=xaxrange,xaxlinecoords=xaxlinecoords,xaxcentral=1.,
		    yaxlinecoords=yaxlinecoords,
		    legendbox=legendbox,showvalues=False,
		    lumi=lumi, xaxtitle='Observed / Predicted', yaxtitle='',
		    extracmstext='',
		    style='whiskers' )
