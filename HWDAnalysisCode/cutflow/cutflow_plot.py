############################################################
# make plots of the histograms filled with cutflow_fill.py #
############################################################

import sys
import os
import ROOT
import json
sys.path.append('../tools/python')
import optiontools as opt
import histtools as ht
sys.path.append('../plotting')
import plotsinglehistogram as psh

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # parse arguments
    outputdir = os.path.dirname(options.outputfile)
    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    # get the histogram
    histlist = ht.loadallhistograms(options.inputfile)
    if len(histlist)!=1:
	raise Exception('ERROR: unexpected number of histograms found: {}'.format(len(histlist)))
    hist = histlist[0]

    # print the histogram to get numerical values
    ht.printhistogram(hist)

    # modify the angle of the labels
    #for i in range(1,hist.GetNbinsX()+1):
    #	label = hist.GetXaxis().GetBinLabel(i)
    #	hist.GetXaxis().ChangeLabel(i,45,-1,-1,-1,-1,label)
    # does not seem to work in combination with SetBinLabel (only defined for numeric axis)...
    # workaround:
    hist.GetXaxis().LabelsOption('v')
    
    # set plotting options
    xaxtitle = ''
    yaxtitle = 'Number of events'
    drawoptions = 'hist e'
    psh.plotsinglehistogram(hist, options.outputfile, title=None, 
			    xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
			    bottommargin=0.45)
