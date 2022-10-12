##########################################################
# make plots of gen-matched vs non-gen-matched Ds mesons #
##########################################################

import sys
import os
import ROOT
sys.path.append('../tools/python')
import optiontools as opt
import histtools as ht
import variabletools as vt
sys.path.append('../plotting')
import plotmultihistograms as pmh

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path', required=True) )
    options.append( opt.Option('outputdir', vtype='path', required=True) )
    options.append( opt.Option('variables', vtype='path', required=True) )
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
    variables = vt.readvariables(options.variables)

    # loop over variables 
    for el in variables:
	varname = el['name']
	
	# find histograms
	print('now processing variable {}'.format(varname))
	thishistlist = []
	thismatchhist = None
	thispartmatchhist = None
	thisnonmatchhist = None
	for hist in histlist:
	    if hist.GetName()==varname+'_match': 
		thismatchhist = hist
		thishistlist.append(hist)
	    elif hist.GetName()==varname+'_partmatch':
		thispartmatchhist = hist
		thishistlist.append(hist)
	    elif hist.GetName()==varname+'_nomatch': 
		thisnonmatchhist = hist
		thishistlist.append(hist)
	if( thismatchhist is None 
	    or thisnonmatchhist is None
	    or thispartmatchhist is None ):
	    print('ERROR: could not find all expected histograms for this variable.')
	    continue
	if len(thishistlist)!=3:
	    print('ERROR: number of histograms for this variable is {}'.format(len(thishistlist)))
	    continue

	# other settiings
	thishistlist = [thismatchhist, thispartmatchhist, thisnonmatchhist]
	labels = ['gen match', 'partial gen match', 'no gen match']
	xaxtitle = el['title']

	# make absolute plot
	figname = os.path.join(options.outputdir, varname+'_abs')
        yaxtitle = 'Number of D_{s} candidates'
	pmh.plotmultihistograms(thishistlist, 
			figname,
			labellist = labels,
			title=None, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                        normalize=False, normalizefirst=False,
                        dolegend=True,
                        colorlist=None,
                        logy=False, drawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=False, ratiorange=None, yminzero=False )

	# make normalized plot
	figname = os.path.join(options.outputdir, varname+'_norm')
	yaxtitle = 'Arbitrary units'
        pmh.plotmultihistograms(thishistlist, 
			figname,
			labellist = labels,
                        title=None, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                        normalize=True, normalizefirst=False,
                        dolegend=True,
                        colorlist=None,
                        logy=False, drawoptions='hist e',
                        lumitext='', extralumitext = '',
                        doratio=False, ratiorange=None, yminzero=False )
