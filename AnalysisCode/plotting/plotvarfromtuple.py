#####################################################################################
# Python script to read a noskim.root tuple (or similar), and plot a variable in it #
#####################################################################################
import ROOT
import os
import sys
import plottools as pt

def fillvarfromtuple(tree,varname,xlow,xhigh,nbins,varsize='',weightvar='',nprocess=-1,label=''):
    # filling histogram from a given tree
    
    # make output histogram
    hist = ROOT.TH1F('hist','',nbins,xlow,xhigh)
    # also store min and max value if they are outside the range (to rebin if needed)
    minvalue = xlow
    maxvalue = xhigh

    # set number of events to process
    if( nprocess<0 or nprocess>tree.GetEntries() ): nprocess=tree.GetEntries() 

    # loop over events
    for i in range(nprocess):
	if( i%10000==0 ):
	    print('number of processed events: '+str(i)) 
	tree.GetEntry(i)
	# determine weight for this entry
	weight = 1
	if weightvar!='': weight = getattr(tree,weightvar)
	# determine values for requested variable in this entry
	varvalues = []
	# case 1: branch is scalar
	if varsize=='':
	    varvalues = [getattr(tree,varname)]
	# case 2: branch is vector
	else:
	    nvalues = getattr(tree,varsize)
	    varvalues = [getattr(tree,varname)[j] for j in list(range(nvalues))]
	# fill the histogram
	for var in varvalues: 
	    hist.Fill(var,weight)
	    if var > maxvalue: maxvalue = var
	    if var < minvalue: minvalue = var

    if( maxvalue > xhigh or minvalue < xlow ):
	print('detected extrema were outside the histogram range:')
	print('    min: '+str(minvalue))
	print('    max: '+str(maxvalue))
    
    # set histogram title
    if label=='': hist.SetTitle(varname)
    else: hist.SetTitle(label)
    return hist

def plotsinglehistogram(hist,figname,title='',xaxtitle='',yaxtitle='',logy=False):
    # drawing a single histogram

    pt.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(1200,1200)
    # set fonts and text sizes
    titlefont = 6; titlesize = 60
    labelfont = 5; labelsize = 50
    axtitlefont = 5; axtitlesize = 50
    infofont = 6; infosize = 30
    legendfont = 4; legendsize = 30

    c1.SetBottomMargin(0.15)
    c1.SetLeftMargin(0.15)
    c1.SetTopMargin(0.15)

    # set histogram properties
    hist.SetLineColor(ROOT.kBlue)
    hist.SetLineWidth(3)
    hist.Sumw2()

    # legend
    leg = ROOT.TLegend(0.6,0.7,0.9,0.8)
    leg.SetTextFont(10*legendfont+3)
    leg.SetTextSize(legendsize)
    leg.SetBorderSize(1)
    leg.AddEntry(hist,hist.GetTitle(),"l")
    hist.Draw("HIST")

    # X-axis layout
    xax = hist.GetXaxis()
    xax.SetNdivisions(10,4,0,ROOT.kTRUE)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetLabelSize(labelsize)
    xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(1.2)
    # Y-axis layout
    if not logy:
	hist.SetMaximum(hist.GetMaximum()*1.2)
	hist.SetMinimum(0.)
    else:
	c1.SetLogy()
	hist.SetMaximum(hist.GetMaximum()*10)
	#hist.SetMinimum(hist.GetMaximum()/1e7)
	hist.SetMinimum(0.5) # to show all non-empty bins if working with weight = 1
    yax = hist.GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(1.5)
    hist.Draw("HIST")

    # title
    ttitle = ROOT.TLatex()	
    ttitle.SetTextFont(10*titlefont+3)
    ttitle.SetTextSize(titlesize)
    titlebox = (0.15,0.92)

    # draw all objects
    hist.Draw("HIST")
    leg.Draw("same")
    ttitle.DrawLatexNDC(titlebox[0],titlebox[1],title)

    c1.SaveAs(figname.split('.')[0]+'.png')

if __name__=='__main__':

    # initialization
    finloc = '/storage_mnt/storage/user/llambrec/Kshort/files/test/noskim.root'

    if( len(sys.argv)==2 ):
	finloc = os.path.abspath(sys.argv[1])

    fin = ROOT.TFile.Open(finloc)
    tree = fin.Get("blackJackAndHookers/blackJackAndHookersTree")
    nprocess = 10000
    xaxtitle = r'lhe reweighting factor'
    yaxtitle = r'number of entries'
    varname = '_lheWeight'
    varsize = '_nLheWeights'
    title = r'lhe weights'

    hist = fillvarfromtuple(tree,varname,-100,100,200,varsize=varsize,weightvar='',nprocess=nprocess)
    plotsinglehistogram(hist,'figure.png',xaxtitle=xaxtitle,yaxtitle=yaxtitle,title=title,logy=True)
