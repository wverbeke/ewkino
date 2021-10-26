#################################################
# read a histogram from a root file and plot it #
#################################################

import ROOT
import sys
import os
sys.path.append('../tools/python')
import plottools as pt
import optiontools as opt

def plotsinglehistogram(hist, figname, title=None, xaxtitle=None, yaxtitle=None, 
			    label=None, color=None, logy=False, drawoptions='',
			    lumitext='', extralumitext='',
			    topmargin=None, bottommargin=None,
			    leftmargin=None, rightmargin=None,
			    xaxlabelfont=None, xaxlabelsize=None ):
    ### drawing a single histogram
    # - label: string for the legend entry for this histogram.
    #	note: if label is 'auto', the implicit title of the TH1 will be used.
    # - drawoptions: string passed to TH1.Draw.
    #   use "HIST" for histogram style (no error bars)
    #       "E" for error bars
    #       "X0" to suppress horizontal error bars
    #       see https://root.cern/doc/master/classTHistPainter.html for a full list of options

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### parse arguments
    if color is None: color = ROOT.kAzure-4
    
    ### initializations
    # make canvas
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(600,600)
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetTicks(1,1)
    pad1.SetFrameLineWidth(2)
    pad1.SetGrid()
    pad1.Draw()
    pad1.cd()
    # set fonts and text sizes
    titlefont = 6; titlesize = 26
    if xaxlabelfont is None: xaxlabelfont = 4 
    if xaxlabelsize is None: xaxlabelsize = 22
    yaxlabelfont = 4; yaxlabelsize = 22
    axtitlefont = 4; axtitlesize = 26
    legendfont = 4
    # margins
    if leftmargin is None: leftmargin = 0.15
    if rightmargin is None: rightmargin = 0.05
    if topmargin is None: topmargin = 0.05
    if bottommargin is None: bottommargin = 0.15
    pad1.SetBottomMargin(bottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.SetTopMargin(topmargin)
    # legendbox
    pentryheight = 0.05
    plegendbox = ([leftmargin+0.3,1-topmargin-pentryheight,
                    1-rightmargin-0.03,1-topmargin-0.03])

    ### set histogram properties
    hist.SetLineColor(color)
    hist.SetLineWidth(3)
    hist.Sumw2()

    ### make the legend
    if label is not None:
	leg = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
	leg.SetTextFont(10*legendfont+3)
	leg.SetFillColor(ROOT.kWhite)
	leg.SetBorderSize(1)
	if label=='auto': label = hist.GetTitle()
	leg.AddEntry(hist,label,"l")
    hist.Draw(drawoptions)

    ### X-axis layout
    xax = hist.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelFont(10*xaxlabelfont+3)
    xax.SetLabelSize(xaxlabelsize)
    if xaxtitle is not None: 
	xax.SetTitle(xaxtitle)
	xax.SetTitleFont(10*axtitlefont+3)
	xax.SetTitleSize(axtitlesize)
	xax.SetTitleOffset(1.2)
    ### Y-axis layout
    if not logy:
	hist.SetMaximum(hist.GetMaximum()*1.2)
	hist.SetMinimum(0.)
    else:
	c1.SetLogy()
	hist.SetMaximum(hist.GetMaximum()*10)
	hist.SetMinimum(hist.GetMaximum()/1e7)
    yax = hist.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*yaxlabelfont+3)
    yax.SetLabelSize(yaxlabelsize)
    if yaxtitle is not None: 
	yax.SetTitle(yaxtitle)
	yax.SetTitleFont(10*axtitlefont+3)
	yax.SetTitleSize(axtitlesize)
	yax.SetTitleOffset(1.5)
    hist.Draw(drawoptions)

    # title
    # note: use of title is not recommended
    #if title is not None:
    #	ttitle = ROOT.TLatex()	
    #	ttitle.SetTextFont(10*titlefont+3)
    #	ttitle.SetTextSize(titlesize)
    #	titlebox = (0.15,0.92)

    # draw all objects
    hist.Draw(drawoptions)
    ROOT.gPad.RedrawAxis()
    pt.drawLumi(pad1, extratext=extralumitext, lumitext=lumitext)
    if label is not None: leg.Draw("same")
    #if title is not None: ttitle.DrawLatexNDC(titlebox[0],titlebox[1],title)

    c1.SaveAs(figname.split('.')[0]+'.png')

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('histname', default=None) )
    options.append( opt.Option('xaxtitle', default='') )
    options.append( opt.Option('yaxtitle', default='') )
    options.append( opt.Option('title', default='') )
    options.append( opt.Option('logy', vtype='bool', default=False) )
    options.append( opt.Option('label', default='auto') )
    options.append( opt.Option('drawoptions', default='') )
    options.append( opt.Option('lumitext', default='') )
    options.append( opt.Option('extralumitext', default='') )
    options.append( opt.Option('outputfile', default='figure.png') )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
	print(options)
	sys.exit()
    else:
	options.parse_options( sys.argv[1:] )
	print('Found following configuration:')
	print(options)

    fin = ROOT.TFile.Open(options.inputfile)
    hist = fin.Get(options.histname)

    plotsinglehistogram(hist, options.outputfile, 
			xaxtitle=options.xaxtitle, yaxtitle=options.yaxtitle, 
			title=options.title, label=options.label,
			logy=options.logy, drawoptions=options.drawoptions,
			lumitext=options.lumitext, extralumitext=options.extralumitext)
