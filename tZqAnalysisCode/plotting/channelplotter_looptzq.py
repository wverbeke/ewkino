###########################################################
# caller to channelplotter with settings for tzq analysis #
###########################################################

import sys
import os

from channelplotter import readchanneltxt, plotchannels

if __name__=='__main__':

    # define files to run on (put empty string to skip)
    ratiofiles = ([
		    os.path.abspath('../newcombine/datacards_rescaledlumi_topratio_plots/'+
				    'datacards_rescaledlumi_topratio_exp.txt'),
		    os.path.abspath('../newcombine/datacards_rescaledlumi_topratio_plots/'+
                                    'datacards_rescaledlumi_topratio_obs.txt'),
		    os.path.abspath('../newcombine/datacards_rescaledlumi_topratio_plots/'+
                                    'datacards_rescaledlumi_topratio_obs_ext.txt'),
		    os.path.abspath('../newcombine/datacards_rescaledlumi_topchannels_plots/'+
                                    'datacards_rescaledlumi_topchannels_indirectratio_exp.txt'),
		    os.path.abspath('../newcombine/datacards_rescaledlumi_topchannels_plots/'+
                                    'datacards_rescaledlumi_topchannels_indirectratio_obs.txt'),
		])
    do_paper_style = True
    do_info_style = True

    for ratiofile in ratiofiles:

	if not os.path.exists(ratiofile):
	    raise Exception('### ERROR ###: input file for ratio {}'.format(ratiofile)
				+' does not seem to exist...')
	channels = readchanneltxt(ratiofile)

	if do_paper_style:

	    xaxrange = (0.,1.8)
	    xaxlinecoords = [0.6,0.8,1.2,1.4,1.6]
	    yaxlinecoords = [0.5,1.5,2.5,3.5,4.5]
	    legendbox = [0.65, 0.7, 0.98, 0.91]
	    figfile = ratiofile.replace('.txt','_paperstyle')

	    plotchannels(channels,figfile,showvalues=False,
                xaxcentral=1,xaxlinecoords=xaxlinecoords,
                yaxlinecoords=yaxlinecoords,
                xaxrange=xaxrange,
                legendbox=legendbox,
		extracmstext='Preliminary',
                lumi=138,xaxtitle='Observed / Predicted',yaxtitle='')

	if do_info_style:

	    xaxrange = (-1.,1.75)
            xaxlinecoords = [0.6,0.8,1.2,1.4,1.6]
            yaxlinecoords = [0.5,1.5,2.5,3.5,4.5]
            legendbox = [0.75, 0.78, 0.93, 0.91]
            figfile = ratiofile.replace('.txt','_infostyle')

            plotchannels(channels,figfile,
		showvalues=True,fontsize=0.03,
                xaxcentral=1,xaxlinecoords=xaxlinecoords,
                yaxlinecoords=yaxlinecoords,
                xaxrange=xaxrange,
                legendbox=legendbox,
                lumi=138,xaxtitle='Observed / Predicted',yaxtitle='')
	    
