##############################################################
# make plots of the histograms filled with fillhistograms.py #
##############################################################

import sys
import os
import ROOT
import json
import fillhistograms as fh
sys.path.append('../tools')
import optiontools as opt
import histtools as ht
sys.path.append('../plotting')
import plotmultihistograms as pmh

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

    histlist = ht.loadallhistograms(options.inputfile)
    variables = fh.load_variables(options.variables)
    
    # loop over variables 
    for el in variables:
	varname = el['name']
	xaxtitle = el['title']
	print('now processing variable {}'.format(varname))
	thishistlist = []
	for hist in histlist:
	    if hist.GetName()==varname+'_match': thishistlist.append(hist)
	    elif hist.GetName()==varname+'_nomatch': thishistlist.append(hist)
	if len(thishistlist)!=2:
	    print('ERROR: number of histograms for this variable is {}'.format(len(thishistlist)))
	    continue

	# make absolute plot
	figname = os.path.join(options.outputdir, varname+'_abs')
        yaxtitle = 'Number of D_{s} candidates'
	pmh.plotmultihistograms(thishistlist, figname, 
			title=None, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                        normalize=False, normalizefirst=False,
                        dolegend=True, labellist=None,
                        colorlist=None,
                        logy=False, drawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=False, ratiorange=None, yrange=None, yminzero=False )

	# make normalized plot
	figname = os.path.join(options.outputdir, varname+'_norm')
	yaxtitle = 'Arbitrary units'
        pmh.plotmultihistograms(thishistlist, figname,
                        title=None, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                        normalize=True, normalizefirst=False,
                        dolegend=True, labellist=None,
                        colorlist=None,
                        logy=False, drawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=False, ratiorange=None, yrange=None, yminzero=False )
