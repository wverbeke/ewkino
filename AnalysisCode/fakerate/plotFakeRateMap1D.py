##############################################################
# make fake rate plots as a function of 1 variable at a time #
##############################################################
# this script presupposes the existence of a root file with a 2D histogram with fake rates!

import ROOT
import sys
import os
from array import array
sys.path.append(os.path.abspath('../plotting'))
from multihistplotter import plothistograms

def extract1Dhists( hist2d, xvarname, yvarname, keepdim='x' ):

    nxbins = hist2d.GetNbinsX()
    xbins = hist2d.GetXaxis().GetXbins().GetArray()
    nybins = hist2d.GetNbinsY()
    ybins = hist2d.GetYaxis().GetXbins().GetArray()

    if keepdim=='x':
	divnbins = nybins
	varnbins = nxbins
	divbins = ybins
	varbins = xbins
	divvarname = yvarname
    elif keepdim=='y':
	divnbins = nxbins
        varnbins = nybins
        divbins = xbins
        varbins = ybins
        divvarname = xvarname
    else:
	raise Exception('argument keepdim not recognized')

    divhists = []
    for i in range(divnbins): 
	divhists.append(ROOT.TH1F('','',varnbins,varbins))
	divlow = divbins[i]
	divhigh = divbins[i+1]
	divbinstr = '{:.2f} < {} < {:.2f}'.format(divlow,divvarname,divhigh)
	divhists[i].SetName('bin_{}_{}'.format(divvarname,i+1))
	divhists[i].SetTitle(divbinstr)
	for j in range(varnbins):
	    if keepdim=='x':
		binvalue = hist2d.GetBinContent(j+1,i+1)
		binerror = hist2d.GetBinError(j+1,i+1)
	    elif keepdim=='y':
		binvalue = hist2d.GetBinContent(i+1,j+1)
                binerror = hist2d.GetBinError(i+1,j+1)
	    divhists[i].SetBinContent(j+1,binvalue)
	    divhists[i].SetBinError(j+1,binerror)
    return divhists

if __name__=='__main__':

    rootfile = sys.argv[1]
    frmapname = sys.argv[2]
    lumitxt = sys.argv[3]

    if not os.path.exists(rootfile):
	raise Exception('ERROR: file '+rootfile+' does not seem to exist...')
    f = ROOT.TFile.Open(rootfile)
    keys = [key.GetName() for key in f.GetListOfKeys()]
    if not frmapname in keys:
	raise Exception('ERROR: histogram '+frmapname+' does not seem to exist in file '
			+rootfile+'...')
    frmap = f.Get(frmapname)
    frmap.SetDirectory(0)
    f.Close()

    xhists = extract1Dhists( frmap, 'p_{T}', '|#eta|', keepdim='x' ) 
    yhists = extract1Dhists( frmap, 'p_{T}', '|#eta|', keepdim='y' )
    yhists = yhists[:-1] # remove largest pt histogram

    # write a file (for testing)
    #f = ROOT.TFile.Open('test.root','recreate')
    #for h in xhists: h.Write()
    #for h in yhists: h.Write()
    #f.Close()

    # make the plots
    outfigname = rootfile.replace('.root','_pt.png')
    plothistograms( xhists, 'p_{T} (GeV)', 'fake rate', outfigname, errorbars=True,
		    yminzero=True, doratio=False, lumistr=lumitxt )
    outfigname = rootfile.replace('.root','_eta.png')
    plothistograms( yhists, '|#eta|', 'fake rate', outfigname, errorbars=True,
                    yminzero=True, doratio=False, lumistr=lumitxt )
