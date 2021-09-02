##################################################################
# script to read combine output file and plot the postfit shapes #
##################################################################
import sys 
import os
import ROOT
import histplotter as hp
import plottools as pt

def readchannels(histfile):
    ### read fitDiagnosics output file and get list of available channels
    f = ROOT.TFile.Open(histfile)
    dirname = 'shapes_fit_s'
    f.cd(dirname)
    gdir = ROOT.gDirectory
    keylist = gdir.GetListOfKeys()
    return [c.GetName() for c in keylist]

def loadhistograms(histfile,channel):
    ### read fitDiagnostics output file and get histograms
    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    dirname = 'shapes_fit_s/'+channel
    f.cd(dirname)
    gdir = ROOT.gDirectory
    histdict = {}
    keylist = gdir.GetListOfKeys()
    for key in keylist:
        hist = f.Get(dirname+'/'+key.GetName())
        try:
	    hist.SetDirectory(0)
	except:
	    # (SetDirectory(0) does not seem to work for data, which is a TGraphAsymmErrors)
	    if hist.GetName()=='data': pass
	    else:
		print('WARNING in loadhistograms: could not call SetDirectory(0)'
			+' on object {}, skipping it...'.format(hist.GetName()))
		continue
        # add hist to dict
        histdict[hist.GetName()] = hist
    return histdict

def tgraphtohist(tgraph,emptyhist):
    ### convert TGraphAsymmetricErrors to TH1F (only for data!)
    for i in range(tgraph.GetN()):
	for c in range(int(tgraph.GetY()[i])):
	    emptyhist.Fill(tgraph.GetX()[i])
    emptyhist.SetName(tgraph.GetName())
    emptyhist.SetTitle(tgraph.GetTitle())
    return emptyhist
    

if __name__=='__main__':

    ### parse input arguments
    if len(sys.argv)==2:
	histfilepath = os.path.abspath(sys.argv[1])
    else:
	raise Exception('ERROR: wrong number of input arguments')
    if not os.path.exists(histfilepath):
	raise Exception('ERROR: FitDiagnostics output file {} does not seem to exist...'.format(
			histfilepath))
    (datacarddir,histfilename) = histfilepath.rsplit('/',1)
    channels = readchannels(histfilepath)
    #outputdir = histfilepath.replace('.root','_plots')
    outputdir = 'test'
    if os.path.exists(outputdir):
	os.system('rm -r '+outputdir)
    os.makedirs(outputdir)

    ### do additional settings
    # color and label maps
    colormap = pt.getcolormap('tzqanalysis')
    labelmap = pt.getlabelmap('tzqanalysis')

    for channel in channels:
	thisoutputdir = os.path.join(outputdir,channel)
	os.makedirs(thisoutputdir)

	# load histograms
	# - predictions for processes separately
	histdict = loadhistograms(histfilepath,channel)
	mchistlist = [histdict[h] for h in histdict.keys() if not ('data' in h or 'total' in h)]
	# - data histogram
	emptyhist = mchistlist[0].Clone()
	emptyhist.Reset()
	datahistlist = [histdict[h] for h in histdict.keys() if 'data' in h]
	if not len(datahistlist)==1:
	    raise Exception('ERROR: found {} data histograms'.format(len(datahistlist)))
	datahist = tgraphtohist(datahistlist[0],emptyhist)
	# - uncertainty on simulation
	mcerrorhistlist = [histdict[h] for h in histdict.keys() if h=='total']
	if not len(mcerrorhistlist)==1:
	    raise Exception('ERROR: found {} sim. unc. histograms'.format(len(mcerrorhistlist)))
	mcerrorhist = mcerrorhistlist[0]

	# prints for testing
	for i in range(0,mcerrorhist.GetNbinsX()+2):
	    print('bin: {}, value: {}, error: {}'.format(i,mcerrorhist.GetBinContent(i),
						    mcerrorhist.GetBinError(i)))

	# make a histogram for total variation
	for i in range(0,mcerrorhist.GetNbinsX()+2):
	    bc = mcerrorhist.GetBinContent(i)
	    be = mcerrorhist.GetBinError(i)
	    mcerrorhist.SetBinContent(i,be)
	    
	# set histogram titles (for plot legend)
	for hist in mchistlist:
	    hist.SetTitle(hist.GetName())
	    print(hist.GetTitle())
	datahist.SetTitle('data')

	lumi = 0.
	if '2016' in histfilename: lumi = 35900
	elif '2017' in histfilename: lumi = 41500
	elif '2018' in histfilename: lumi = 59700

	# set plot properties
	binwidth = datahist.GetBinWidth(1)
	#if binwidth.is_integer():
	#    yaxtitle = 'events / '+str(int(binwidth)) #+' '+vardict['unit']
	#else:
	#    yaxtitle = 'events / {0:.2f}'.format(binwidth) #+' '+vardict['unit']
	yaxtitle = 'Number of events' # use simple title instead of mentioning bin width
	xaxtitle = 'bin number'
	#if not vardict['unit']=='':
	#    xaxtitle += '('+vardict['unit']+')'
	figname = os.path.join(thisoutputdir,'fit_variable_postfit')

	hp.plotdatavsmc(figname+'_lin',datahist,mchistlist,mcerrorhist,dostat=False,
			colormap=colormap,labelmap=labelmap,
			xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,yaxlog=False)
	hp.plotdatavsmc(figname+'_log',datahist,mchistlist,mcerrorhist,dostat=False,
                        colormap=colormap,labelmap=labelmap,
                        xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,yaxlog=True)
