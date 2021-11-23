################################################################################
# make plots of the results of different Higgs boson reconstruction techniques #
################################################################################

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
    options.append( opt.Option('overflow', vtype='bool', default=True) )
    options.append( opt.Option('underflow', vtype='bool', default=True) )
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
	for hist in histlist:
	    if hist.GetName().split('_',-1)[0]==varname: 
		thishistlist.append(hist)
	print('found {} histograms'.format(len(thishistlist)))

	# remove under- and/or overflow bins if requested
	# preliminary implementation, simply put first and last bin to zero
	if not options.underflow:
	    for hist in thishistlist: 
		hist.SetBinContent(1,0)
		hist.SetBinError(1,0)
	if not options.overflow:
	    for hist in thishistlist:
		hist.SetBinContent(hist.GetNbinsX(),0)
		hist.SetBinError(hist.GetNbinsX(),0)

	# other settiings
	labels = [hist.GetName().split('_',-1)[1] for hist in thishistlist]
	xaxtitle = el['title']
	ymaxlinfactor = 1.8
	if( varname=='hmass' or varname=='hpt' ): ymaxlinfactor = 1.3

	# make plot
	figname = os.path.join(options.outputdir, varname+'_abs')
        yaxtitle = 'Number of H candidates'
	plotobject = pmh.plotmultihistograms(thishistlist, 
			labellist = labels,
			title=None, xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                        normalize=False,
                        dolegend=True,
                        colorlist=None,
                        logy=False, 
			ymaxlinfactor=ymaxlinfactor,
			drawoptions='hist e',
                        lumitext='', extralumitext = '')

	if plotobject is not None: 
	    
	    canvas = plotobject['canvas']
	    xlims = plotobject['xlims']
	    ylims = plotobject['ylims']

	    # draw nominal H mass line
	    if varname=='hmass':
		infofont = 4
		tline = ROOT.TLine(125., ylims[0], 125., ylims[1])
		tline.SetLineWidth(2)
		tline.SetLineColor(ROOT.kBlack)
		tline.SetLineStyle(9)
		tline.Draw()
		tinfo = ROOT.TLatex()
		tinfo.SetTextFont(10*infofont+3)
		tinfo.SetTextSize(22)
		tinfo.SetTextColor(ROOT.kBlack)
		xcoord = 120.
		ycoord = ylims[0]-0.04*(ylims[1]-ylims[0])
		tinfo.DrawLatex(xcoord, ycoord, 'm(H)')

	    # save the figure
	    canvas.SaveAs(figname.replace('.png','')+'.png')	
