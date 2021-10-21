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
import plotsinglehistogram as psh

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
    
    for el in variables:
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
	figname = os.path.join(options.outputdir, varname)
	psh.plotsinglehistogram(hist, figname, title=None, xaxtitle=xaxtitle, yaxtitle=None,
                            label=None, color=None, logy=False, drawoptions='',
                            lumitext='', extralumitext='')
