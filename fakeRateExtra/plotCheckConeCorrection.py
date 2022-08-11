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
flavours = ['muon']
# (choose lepton flavours; use "emu" for adding electrons and muons together)
years = ['2017']
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
	    labellist=None, colorlist=None,
	    title=None, titlesize=None,
	    xaxtitle=None, xaxtitlesize=None, xaxtitleoffset=None,
	    yaxtitle=None, yaxtitlesize=None, yaxtitleoffset=None,
	    extracmstext='', lumitext='' ):
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
    # - colorlist: list of ROOT colors
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
    # title offset
    if yaxtitleoffset is None: ytitleoffset = 1.5
    if xaxtitleoffset is None: xtitleoffset = 1.5
    # margins
    topmargin = 0.15
    bottommargin = 0.15
    leftmargin = 0.15
    rightmargin = 0.15
    # legend box 
    legendbox = [leftmargin+0.35,1-topmargin-0.3,1-rightmargin-0.03,1-topmargin-0.03]
    # get a reference histogram for axes
    hist = histlist[0]
    # get axis properties
    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()
    zmin = hist.GetMinimum()
    zmax = hist.GetMaximum()

    # parse colorlist argument
    if colorlist is None: 
	colorlist = ([ROOT.kBlue-10, ROOT.kCyan-7, ROOT.kYellow+1, ROOT.kRed-7,
		      ROOT.kTeal-5,ROOT.kMagenta-7])
	if len(histlist)>len(colorlist):
	    raise Exception('ERROR: this many histograms do not support automatic colors.')
	colorlist = colorlist[:len(histlist)]

    # set the colors for all histograms
    for ihist,icolor in zip(histlist, colorlist):
	ihist.SetLineColor(icolor)
	ihist.SetLineWidth(2)

    # parse labellist argument
    dolegend = True
    if labellist is None: dolegend = False

    # make legend and add all histograms
    legend = ROOT.TLegend(legendbox[0],legendbox[1],legendbox[2],legendbox[3])
    legend.SetFillStyle(0)
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
    for ihist in histlist:
	ihist.Draw( 'cont3 same' )
    # redraw first histogram to get the axes
    hist.Draw( 'cont3 same' )

    # write additional information
    legend.Draw('same')
    ttitle.DrawLatexNDC(leftmargin,0.9,histtitle)
    pt.drawLumi(c1, extratext=extracmstext, cmstext_size_factor=0.3,
		    lumitext=lumitext)

    # save the plot
    c1.Update()
    outfilepath = os.path.splitext(outfilepath)[0]
    for outfmt in outfmts: c1.SaveAs(outfilepath+outfmt)


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
	    uptcorr = rfile.Get("uncorrectedPtCorr")
	    uptcorr.SetDirectory(0)
	    cptcorr = rfile.Get("correctedPtCorr")
	    cptcorr.SetDirectory(0)
	    rfile.Close()

	    # rescale both
	    rescale(uptcorr)
	    rescale(cptcorr)

	    # make a plot of histogram with uncorrected pt
	    outfile = 'correlation_{}_{}_uncorrectedpt'.format(year,flavour)
	    outfile = os.path.join(outputdir,outfile)
	    histtitle = 'Uncorrected lepton p_{T} vs mother parton p_{T}'
	    h2p.plot2dhistogram( uptcorr, outfile,
		histtitle=histtitle,
		drawoptions='col0z', cmin=0.01,
		docmstext=True, extracmstext='Preliminary Simulation' )

	    # make a plot of histogram with corrected pt
	    outfile = 'correlation_{}_{}_correctedpt'.format(year,flavour)
            outfile = os.path.join(outputdir,outfile)
            histtitle = 'Cone-corrected lepton p_{T} vs mother parton p_{T}'
            h2p.plot2dhistogram( cptcorr, outfile,
                histtitle=histtitle,
		drawoptions='col0z', cmin=0.01,
		docmstext=True, extracmstext='Preliminary Simulation' )

	    # make a plot of both histograms together
	    outfile = 'correlation_{}_{}_comparison'.format(year,flavour)
            outfile = os.path.join(outputdir,outfile)
            histtitle = 'Lepton p_{T} vs mother parton p_{T}'
	    histlist = [uptcorr, cptcorr]
	    labellist = ['Uncorrected p_{T}', 'Cone-corrected p_{T}']
	    colorlist = [ROOT.kBlue-4, ROOT.kViolet+1]
	    contours = [0.9]
            multi_contour_plot( histlist, outfile,
		contours=contours,
		labellist=labellist, colorlist=colorlist,
                title=histtitle, xaxtitle='Parton p_{T}', yaxtitle='Lepton p_{T}',
		extracmstext='#splitline{Preliminary}{Simulation}' )
