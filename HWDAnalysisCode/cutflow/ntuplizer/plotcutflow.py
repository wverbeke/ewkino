############################################################
# make plots of the histograms filled with cutflow_fill.py #
############################################################

import sys
import os
import ROOT
import json
sys.path.append('../../tools/python')
import optiontools as opt
import histtools as ht
sys.path.append('../../plotting')
import plotsinglehistogram as psh


### help functions for creating alternative views of the cutflow histogram
sys.path.append('..')
from cutflow_plot import make_cumulative_absolute, make_cumulative_relative, make_relative_fail, make_relative_pass

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('labelfile', vtype='path') )
    options.append( opt.Option('outputdir', vtype='path') )
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
    if not os.path.exists(options.outputdir):
	os.makedirs(options.outputdir)

    # get the histogram
    histlist = ht.loadallhistograms(options.inputfile)
    if len(histlist)!=1:
	raise Exception('ERROR: unexpected number of histograms found: {}'.format(len(histlist)))
    hist = histlist[0]

    # print the histogram to get numerical values
    #ht.printhistogram(hist)

    # get the labels from a json file
    with open(options.labelfile,'r') as f:
	labels = json.load(f)

    # set the labels on the histogram object
    for binnb in range(1,hist.GetNbinsX()+1):
	hist.GetXaxis().SetBinLabel(binnb, labels[str(binnb)])

    # modify the angle of the labels
    hist.GetXaxis().LabelsOption('v')
    
    # make raw plot
    xaxtitle = ''
    yaxtitle = 'Number of events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'raw.png')
    psh.plotsinglehistogram(hist, outputfile, title=None, 
			    xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
			    bottommargin=0.45)

    # make cumulative absolute plot
    cumabshist = make_cumulative_absolute( hist )
    cumabshist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Number of remaining events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'cumulative_absolute.png')
    psh.plotsinglehistogram(cumabshist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45)

    # make cumulative relative plot
    cumrelhist = make_cumulative_relative( hist )
    cumrelhist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of remaining events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'cumulative_relative.png')
    psh.plotsinglehistogram(cumrelhist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
			    writebincontent=True)

    # make relative fail plot
    relfailhist = make_relative_fail( hist )
    relfailhist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of failing events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'relative_fail.png')
    psh.plotsinglehistogram(relfailhist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
			    writebincontent=True)

    # make relative pass plot
    relpasshist = make_relative_pass( hist )
    relpasshist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of passing events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'relative_pass.png')
    psh.plotsinglehistogram(relpasshist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
			    writebincontent=True)

