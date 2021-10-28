############################################################
# make plots of the histograms filled with cutflow_fill.py #
############################################################

import sys
import os
import ROOT
import json
sys.path.append('../tools/python')
import optiontools as opt
import histtools as ht
sys.path.append('../plotting')
import plotsinglehistogram as psh

### help functions for creating alternative views of the cutflow histogram
# note: the input histogram for each of these functions is the output of cutflow_fill.py.
#       (in each bin, absolute number of events that fail a given cut,
#	last bin: number of passing events)

def make_cumulative_absolute( hist ):
    ### output histograms: in each bin, total amount of remaining events after each cut

    # copy original histogram skeleton but with an extra first bin (before any cuts)
    # and one bin less (removing final bin)
    nbins = hist.GetNbinsX()
    name = hist.GetName()
    title = hist.GetTitle()
    chist = ROOT.TH1D( "", title, nbins, -0.5, nbins+0.5 )
    # set first bin
    nevents = hist.Integral()
    chist.SetBinContent(1, nevents)
    chist.SetBinError(1,0)
    chist.GetXaxis().SetBinLabel(1, "Before selections")
    # fill further bins
    for i in range(2,chist.GetNbinsX()+1):
	nevents = nevents - hist.GetBinContent(i-1)
	chist.SetBinContent(i, nevents)
	chist.SetBinError(i,0)
	chist.GetXaxis().SetBinLabel(i, 
	    hist.GetXaxis().GetBinLabel(i-1).replace('Fail', 'Pass'))
    return chist

def make_cumulative_relative( hist ):
    ### output histogram: same as make_cumulative_absolute_pass but with y-axis in per cent
    chist = make_cumulative_absolute( hist )
    scale = 1./hist.Integral()
    chist.Scale(scale)
    chist.SetMaximum(1.2)
    return chist

def make_relative_fail( hist ):
    ### output histogram: in each bin, relative fraction of events that fail given cut
    #			  (relative w.r.t. number of events that pass all previous cuts!)
    helphist = make_cumulative_absolute( hist )
    nevents = hist.Integral()
    # copy original histogram skeleton but with last bin removed
    nbins = hist.GetNbinsX()
    name = hist.GetName()
    title = hist.GetTitle()
    rhist = ROOT.TH1D( "", title, nbins-1, -0.5, nbins-0.5 )
    for i in range(1, rhist.GetNbinsX()+1):
	nfail = float(hist.GetBinContent(i))
	ntot = helphist.GetBinContent(i)
	rhist.SetBinContent(i, nfail/ntot)
	rhist.SetBinError(i,0)
	rhist.GetXaxis().SetBinLabel(i,
            hist.GetXaxis().GetBinLabel(i))
    rhist.SetMaximum(1.2)
    return rhist

def make_relative_pass( hist ):
    ### output histogram: same as make_relative_fail but with passing fraction instead of failing
    rhist = make_relative_fail( hist )
    for i in range(1, rhist.GetNbinsX()+1 ):
	rhist.SetBinContent(i, 1-rhist.GetBinContent(i))
	rhist.GetXaxis().SetBinLabel(i, 
	    rhist.GetXaxis().GetBinLabel(i).replace('Fail','Pass'))
    rhist.SetMaximum(1.2)
    return rhist
  

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
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

    # parse arguments
    if not os.path.exists(options.outputdir):
	os.makedirs(options.outputdir)

    # get the histogram
    histlist = ht.loadallhistograms(options.inputfile)
    if len(histlist)!=1:
	raise Exception('ERROR: unexpected number of histograms found: {}'.format(len(histlist)))
    hist = histlist[0]

    # print the histogram to get numerical values
    ht.printhistogram(hist)

    # modify the angle of the labels
    #for i in range(1,hist.GetNbinsX()+1):
    #	label = hist.GetXaxis().GetBinLabel(i)
    #	hist.GetXaxis().ChangeLabel(i,45,-1,-1,-1,-1,label)
    # does not seem to work in combination with SetBinLabel (only defined for numeric axis)...
    # workaround:
    hist.GetXaxis().LabelsOption('v')
    
    # make raw plot
    xaxtitle = ''
    yaxtitle = 'Number of events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'raw.png')
    psh.plotsinglehistogram(hist, outputfile, title=None, 
			    xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
			    bottommargin=0.45)

    # make cumulative absolute plot
    cumabshist = make_cumulative_absolute( hist )
    cumabshist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Number of remaining events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'cumulative_absolute.png')
    psh.plotsinglehistogram(cumabshist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45)

    # make cumulative relative plot
    cumrelhist = make_cumulative_relative( hist )
    cumrelhist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of remaining events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'cumulative_relative.png')
    psh.plotsinglehistogram(cumrelhist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
			    writebincontent=True)

    # make relative fail plot
    relfailhist = make_relative_fail( hist )
    relfailhist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of failing events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'relative_fail.png')
    psh.plotsinglehistogram(relfailhist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
			    writebincontent=True)

    # make relative pass plot
    relpasshist = make_relative_pass( hist )
    relpasshist.GetXaxis().LabelsOption('v')
    xaxtitle = ''
    yaxtitle = 'Fraction of passing events'
    drawoptions = 'hist e'
    outputfile = os.path.join(options.outputdir,'relative_pass.png')
    psh.plotsinglehistogram(relpasshist, outputfile, title=None,
                            xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                            label=None, color=None, logy=False, drawoptions=drawoptions,
                            lumitext='', extralumitext='',
                            bottommargin=0.45,
			    writebincontent=True)

