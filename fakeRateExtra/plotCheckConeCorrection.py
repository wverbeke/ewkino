#########################################################
# merge and plot the results of fillCheckConeCorrection #
#########################################################

import os
import sys
import glob
import ROOT
import array
sys.path.append('../plotting/python')
import hist2dplotter as h2p
import plottools as pt


# global settings
flavours = ['muon','electron','emu']
# (choose lepton flavours; use "emu" for adding electrons and muons together)
years = ['2016PreVFP','2016PostVFP','2018','allyears']
# (choose data taking years; use "allyears" for adding all years together)


def rescale(hist):
    ### little helper function to rescale a 2D histogram to maximum bin content 1
    maxbincontent = hist.GetMaximum()
    if( maxbincontent<1e-12 ):
	print('WARNING: cannot rescale histogram'
		+' as its maximum bin content is {}'.format(maxbincontent))
	return
    hist.Scale(1./maxbincontent)

def multi_contour_plot(histlist, outfilepath, outfmts=['.png'],
	    contours=None,
	    labellist=None, colormaplist=None,
	    title=None, titlesize=None,
	    xaxtitle=None, xaxtitlesize=None, xaxtitleoffset=None,
	    yaxtitle=None, yaxtitlesize=None, yaxtitleoffset=None,
	    extracmstext='', lumitext='',
	    extrainfos=[], infosize=None, infoleft=None, infotop=None ):
    ### draw multiple 2D contours in a figure
    # input arguments:
    # - histlist: list of TH2D objects
    # - outfilepath: path where to store the figure
    # - outfmts: list of output formats (.png, .pdf, .eps and possibly others are allowed)
    # - contours: either a list with countour levels to draw,
    #             or an integer number of (equally-spaced) contour levels (default 5).
    #		  note: contour levels are the same for each histogram
    #                   and determined based on the first histogram in histlist;
    #                   possibly to be made more flexible later!
    # - labellist: list of legend entries
    # - colorlist: list of ROOT color maps
    #              note that the maps must be in string format and not contain 'ROOT.',
    #              so for example 'kBird', 'kViridis', etc.
    # notes:
    # - the axes will be drawn based on the first histogram in histlist
    #   (though in most foreseen use cases they would all have the same axis ranges anyway)
    
    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    
    # set global properties
    cheight = 500 # height of canvas
    cwidth = 700 # width of canvas
    titlefont = 4
    if titlesize is None: titlesize = 22
    xaxtitlefont = 4
    if xaxtitlesize is None: xaxtitlesize = 22
    yaxtitlefont = 4
    if yaxtitlesize is None: yaxtitlesize = 22
    infofont = 4
    if infosize is None: infosize = 15
    # title offset
    if yaxtitleoffset is None: ytitleoffset = 1.5
    if xaxtitleoffset is None: xtitleoffset = 1.5
    # margins
    topmargin = 0.15
    bottommargin = 0.15
    leftmargin = 0.15
    rightmargin = 0.15
    # legend box 
    legendbox = [leftmargin+0.45,1-topmargin-0.2,1-rightmargin-0.03,1-topmargin-0.03]
    # extra info box parameters
    if infoleft is None: infoleft = leftmargin+0.05
    if infotop is None: infotop = 1-topmargin-0.1
    # get a reference histogram for axes
    hist = histlist[0]
    # get axis properties
    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()
    zmin = hist.GetMinimum()
    zmax = hist.GetMaximum()

    # parse color map list argument
    # note: using multiple color maps in one canvas is not easy in ROOT,
    #       and requires the magic below.
    if colormaplist is None: colormaplist = ['kViridis', 'kSolar']
    if len(histlist)>len(colormaplist):
        raise Exception('ERROR: this many histograms do not support automatic colors.'
			+' Please provide a color map list explicitly.')
    colormaplist = colormaplist[:len(histlist)]
    texeclist = []
    for i,cmap in enumerate(colormaplist):
	texeclist.append( ROOT.TExec('ex{}'.format(i), 
			  'gStyle->SetPalette({});'.format(cmap)) )

    # parse labellist argument
    dolegend = True
    if labellist is None: 
	labellist = ['']*len(histlist)
	dolegend = False

    # set the line widths and colors for all histograms
    # note: line colors are not used for drawing (color palettes are used instead),
    #       but they are displayed in the legend.
    # note: again, a bit of ROOT magic seems to be needed to simply extract
    #       a color from a color map...
    for ihist,cmap in zip(histlist,colormaplist):
        ihist.SetLineWidth(2)
	cmapnb = getattr(ROOT,cmap)
	ROOT.gStyle.SetPalette(cmapnb)
	color = ROOT.TColor.GetPalette().At(128)
	ihist.SetLineColor( color )

    # make legend and add all histograms
    legend = ROOT.TLegend(legendbox[0],legendbox[1],legendbox[2],legendbox[3])
    legend.SetFillStyle(1001)
    legend.SetFillColor(ROOT.kWhite)
    for ihist,ilabel in zip(histlist,labellist):
        legend.AddEntry(ihist,ilabel,"l")

    # parse contours argument
    if contours is None: contours = 5
    if isinstance(contours,int):
	contours = list(np.linspace(zmin,zmax,contours,endpoint=False))
    ncontours = len(contours)
    contours = array.array('d', contours)

    # set the contours for all histograms
    for ihist in histlist:
	ihist.SetContour(ncontours, contours)

    # create canvas
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    c1.SetTopMargin(topmargin)
    c1.SetBottomMargin(bottommargin)
    c1.SetLeftMargin(leftmargin)
    c1.SetRightMargin(rightmargin)
    
    # set title and label properties
    if xaxtitle is None: xaxtitle = ''
    hist.GetXaxis().SetTitle(xaxtitle)
    hist.GetXaxis().SetTitleOffset(xtitleoffset)
    hist.GetXaxis().SetTitleFont(xaxtitlefont*10+3)
    hist.GetXaxis().SetTitleSize(xaxtitlesize)
    if yaxtitle is None: yaxtitle = ''
    hist.GetYaxis().SetTitle(yaxtitle)
    hist.GetYaxis().SetTitleOffset(ytitleoffset)
    hist.GetYaxis().SetTitleFont(yaxtitlefont*10+3)
    hist.GetYaxis().SetTitleSize(yaxtitlesize)

    # set title
    ttitle = ROOT.TLatex()
    ttitle.SetTextFont(titlefont*10+3)
    ttitle.SetTextSize(titlesize)

    # draw contours
    hist.Draw()
    for i,ihist in enumerate(histlist):
	texeclist[i].Draw( 'same' )
	ihist.Draw( 'cont1 same' )
	c1.Update()
    # redraw first histogram to get the axes
    texeclist[0].Draw( 'same' )
    hist.Draw( 'cont1 same' )
    c1.Update()

    # draw diagonal line
    bisect = ROOT.TLine(xmin,ymin,xmax,ymax)
    bisect.SetLineWidth(1)
    bisect.SetLineColor(ROOT.kRed)
    bisect.SetLineStyle(9)
    bisect.Draw('same')

    # draw header
    if dolegend: legend.Draw('same')
    ttitle.DrawLatexNDC(leftmargin,0.9,histtitle)
    pt.drawLumi(c1, extratext=extracmstext, cmstext_size_factor=0.3,
		    lumitext=lumitext)

    # draw extra info
    tinfo = ROOT.TLatex()
    tinfo.SetTextFont(10*infofont+3)
    tinfo.SetTextSize(infosize)
    for i,info in enumerate(extrainfos):
        vspace = 0.07*(float(infosize)/20)
        tinfo.DrawLatexNDC(infoleft,infotop-(i+1)*vspace, info)

    # save the plot
    c1.Update()
    outfilepath = os.path.splitext(outfilepath)[0]
    for outfmt in outfmts: c1.SaveAs(outfilepath+outfmt)

    # set gStyle back to default after messing with it
    ROOT.gStyle.SetPalette(ROOT.kBird)


if __name__=='__main__':

    # make output directory
    outputdir = 'coneCorrectionPlots'
    if not os.path.exists(outputdir): os.makedirs(outputdir)

    # hadd files if needed
    print('hadding files if needed...')
    for year in years:
	for flavour in flavours:
	    # define target file
	    filename = 'coneCorrectionCheck_'+flavour+'_'+year+'_histograms.root'
	    # check if target file exists
	    if os.path.exists(filename):
		print('file {} already exists, skipping...'.format(filename))
		continue
	    print('file {} not yet found, doing hadd...'.format(filename))
	    # define source files
	    haddyears = year
	    if year=='allyears': haddyears = '*'
	    haddflavours = flavour
	    if flavour=='emu': haddflavours = '*'
	    haddsource = 'coneCorrectionCheck_'+haddflavours+'_'+haddyears+'_histograms_sample*.root'
	    # check if source files exist
	    if len(glob.glob(haddsource))==0:
		print('required source files for {} do not exist, skipping...'.format(filename))
		continue
	    cmd = 'hadd {} {}'.format(filename, haddsource)
	    print('executing {}'.format(cmd))
	    os.system(cmd)

    # loop over years and flavours
    cwd = os.getcwd()
    for year in years:
        for flavour in flavours:
	    # check if the correct file exists
	    filename = 'coneCorrectionCheck_'+flavour+'_'+year+'_histograms.root'
	    if not os.path.exists(filename):
		print('ERROR: file '+filename+' not found, skipping it.')
		continue
	    
	    # read the histograms
	    rfile = ROOT.TFile.Open(filename)
	    uptcprompt = rfile.Get("uncorrectedPtCorrPrompt")
	    uptcprompt.SetDirectory(0)
	    cptcprompt = rfile.Get("correctedPtCorrPrompt")
	    cptcprompt.SetDirectory(0)
	    uptcnp = rfile.Get("uncorrectedPtCorrNonPrompt")
            uptcnp.SetDirectory(0)
            cptcnp = rfile.Get("correctedPtCorrNonPrompt")
            cptcnp.SetDirectory(0)
	    rfile.Close()

	    # rescale all
	    rescale(uptcprompt)
	    rescale(cptcprompt)
	    rescale(uptcnp)
            rescale(cptcnp)

	    # group them in a dict
	    histdict = {}
	    histdict['uncorrected'] = {'prompt': uptcprompt, 'nonprompt': uptcnp}
	    histdict['corrected'] = {'prompt': cptcprompt, 'nonprompt': cptcnp}

	    # common plot settings
	    labeldict = ({
		'electron': 'Electrons',
		'muon': 'Muons',
		'emu': 'Electrons + Muons',
		'2016PreVFP': '2016PreVFP simulation',
		'2016PostVFP': '2016PostVFP simulation',
		'2017': '2017 simulation',
		'2018': '2018 simulation',
		'allyears': 'Run II simulation',
		'prompt': 'Prompt',
		'nonprompt': 'Nonprompt',
		'uncorrected': 'Uncorrected p_{T}',
		'corrected': 'Cone-corrected p_{T}'
	    })
	    extrainfos = ([
		'Normalized to unit max.',
		labeldict[year],
		labeldict[flavour],
	    ])
	   
	    for leptontype in ['prompt','nonprompt']:
		for pttype in ['uncorrected','corrected']:    
		    outfile = 'correlation_{}_{}_{}_{}'.format(year,flavour,leptontype,pttype)
		    outfile = os.path.join(outputdir,outfile)
		    thisextrainfos = extrainfos + [labeldict[pttype]]
		    h2p.plot2dhistogram( histdict[pttype][leptontype], outfile,
			drawoptions='colz', cmin=0.01,
			docmstext=True, extracmstext='Preliminary Simulation',
			cms_in_grid=False,
			extrainfos=thisextrainfos, infoleft=0.75, infotop=0.85, rightmargin=0.35 )

		# make a plot of both histograms together
		outfile = 'correlation_{}_{}_{}_comparison'.format(year,flavour,leptontype)
		outfile = os.path.join(outputdir,outfile)
		histtitle = 'Lepton p_{T} vs mother parton p_{T}'
		histlist = ([histdict['uncorrected'][leptontype], 
			     histdict['corrected'][leptontype]])
		labellist = ['Uncorrected p_{T}', 'Cone-corrected p_{T}']
		contours = [0.7, 0.8, 0.9, 0.95]
		multi_contour_plot( histlist, outfile,
			contours=contours,
			labellist=labellist,
			title=histtitle, xaxtitle='Parton p_{T} (GeV)', 
			yaxtitle='Lepton p_{T} (GeV)',
			extracmstext='#splitline{Preliminary}{Simulation}',
			extrainfos=extrainfos )
