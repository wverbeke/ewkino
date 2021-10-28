################################################
# make plots of histograms filled tree_fill.py #
################################################

import sys
import os
import ROOT
import json
sys.path.append('../tools/python')
import optiontools as opt
import histtools as ht
import plotsinglehistogram as psh
import tree_fill as tf

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('variables', vtype='path') )
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

    if not os.path.exists(options.outputdir):
        os.makedirs(options.outputdir)

    # load the the histograms and variables
    histlist = ht.loadallhistograms(options.inputfile)
    variables = tf.load_variables(options.variables)

    # loop over variables
    for el in variables:
	# get the histogram
        varname = el['name']
        xaxtitle = el['title']
        print('now processing variable {}'.format(varname))
        thishistlist = []
        for hist in histlist:
            if hist.GetName()==varname: thishistlist.append(hist)
        if len(thishistlist)!=1:
            print('ERROR: number of histograms for this variable is {}'.format(len(thishistlist)))
            continue
        hist = thishistlist[0]

	# set plot options
        figname = os.path.join(options.outputdir, varname)
	yaxtitle = 'Number of events'
	drawoptions = 'hist e'
        psh.plotsinglehistogram(hist, figname, title=None, 
			    xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='')
