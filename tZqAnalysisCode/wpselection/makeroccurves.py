#############################################################################################
# python script to read ROOT histograms filled with classifier response and make roc curves #
#############################################################################################

import os
import sys
import ROOT
import numpy as np
import matplotlib.pyplot as plt
sys.path.append('../tools')
import histtools as ht

def getrocfromhists( signalhist, backgroundhist ):
    
    nbins = signalhist.GetNbinsX()
    xlow = signalhist.GetBinLowEdge(1)
    xhigh = signalhist.GetBinLowEdge(nbins)+signalhist.GetBinWidth(nbins)
    nsig = np.zeros(nbins+1)
    nbck = np.zeros(nbins+1)
    # loop over bins
    for i in range(0,nbins+1):
	# threshold is lower edge of next bin
	nsig[i] = signalhist.Integral(i+1,nbins+1)
	nbck[i] = backgroundhist.Integral(i+1,nbins+1)
    # normalize 
    nsig = nsig[::-1]/nsig[0]
    nbck = nbck[::-1]/nbck[0]
    return {'signalefficiency':nsig,'backgroundefficiency':nbck}

def plotrocs( inputlist, outfilename, title='' ):
    # inputlist contains dicts (one dict per curve)
    # with entries 'sigeff','bckeff','color','label'
    
    fig,ax = plt.subplots()
    for inputdict in inputlist:
	ax.plot(inputdict['bckeff'],inputdict['sigeff'],
		label=inputdict['label'],color=inputdict['color'],
		linewidth=2.)
    ax.legend(loc='lower right')
    if len(title)>0: ax.set_title(title)
    # general axis titles:
    ax.set_xlabel('background effiency')
    ax.set_ylabel('signal efficiency')
    ax.set_xscale('log')
    # set x axis limits: hard-coded
    ax.set_xlim(1e-3,1)
    # set x axis limits
    #ax.set_xlim((np.amin(np.where(inputlist[0]['bckeff']>0.,inputlist[0]['bckeff'],1.))/2.,1.))
    # set y axis limits: hard-coded
    ax.set_ylim(0.7,1.001)
    # set y axis limits: adaptive limits based on measured signal efficiency array.
    #ylowlim = np.amin(np.where((sigeff>0.) & (bckeff>0.),sigeff,1.))
    #ylowlim = 2*ylowlim-1.
    #ax.set_ylim((ylowlim,1+(1-ylowlim)/5))
    ax.grid()
    plt.savefig(outfilename.split('.')[0]+'.png')

if __name__=='__main__':

    inputfile = 'test.root'
    outputfilebasename = 'test'

    if len(sys.argv)==2:
	inputfile = sys.argv[1]
	outputfilebasename = inputfile.replace('.root','')
    elif len(sys.argv)==3:
	inputfile = sys.argv[1]
	outputfilebasename = sys.argv[2]
    else:
	print('### WARNING ###: command line args <inputfile> and/or <outputfile> are missing.')
	print('continue with default values? (y/n)')
	go = raw_input()
	if not go=='y': sys.exit()

    # in case a directory is given as input, apply to all .root files in that directory
    inputfiles = []
    if inputfile[-5:]=='.root': inputfiles.append(inputfile)
    else: inputfiles = ([os.path.join(inputfile,f) for f in os.listdir(inputfile) 
			if f[-5:]=='.root'])
    # in that case outputfilebasename represents the output directory
    # (= input directory if only one argument was given)
    outputfiles = []
    if len(inputfiles)==1: outputfiles = [outputfilebasename]
    else: 
	outputfiles = [f.replace('.root','') for f in inputfiles]
	# temp: additional naming conventions
	for i,f in enumerate(outputfiles):
	    (dirname,f) = os.path.split(f)
	    ellist = f.split('_')
	    pname = ellist[0]
	    era = ellist[-1]
	    outputfiles[i] = os.path.join(dirname,pname+'_'+era)
    
    mvanames = ['TTH','TZQ','TOP']
    clist = ['b','g','r']
    pttags = ['pt10to25','pt25'] 
    flavours = ['muon','electron']

    for inputfile,outputfilebasename in zip(inputfiles,outputfiles):
	print(inputfile)
	print(outputfilebasename)
	if not os.path.exists(outputfilebasename.rsplit('/',1)[0]):
	    os.makedirs(outputfilebasename.rsplit('/',1)[0])

	for flavour in flavours:
	    for pttag in pttags:
		print('making roc curves for '+flavour+'s with pt in '+pttag)
		mvarocs = []
		for i,mvaname in enumerate(mvanames):
		    print('mva: '+mvaname)
		    histlist = ht.loadhistograms(inputfile,mustcontainall=[mvaname,flavour,pttag])
		    print('found following histograms:')
		    for hist in histlist: print('  '+hist.GetName())
		    if len(histlist)!=2:
			print('### ERROR ###: expecting two histograms (prompt and nonprompt)')
			continue
		    prompthist = histlist[0]
		    nonprompthist = histlist[1]
		    if 'nonprompt' in histlist[0].GetName():
			prompthist = histlist[1]
			nonprompthist = histlist[0]
		    roc = getrocfromhists(prompthist,nonprompthist)
		    label = mvaname + ' lepton MVA'
		    if label=='TOP lepton MVA': # replace with 'better' name
			label='new lepton MVA'
		    mvarocs.append({'sigeff':roc['signalefficiency'],
				'bckeff':roc['backgroundefficiency'],
				'label':label,'color':clist[i]})
		pttitle = ''
		if pttag=='pt25': pttitle = '(pt > 25)'
		elif pttag=='pt10to25': pttitle = '(10 < pt < 25)'
		figtitle = 'ROC curves for '+flavour+'s '+pttitle
		plotrocs( mvarocs,outputfilebasename+'_'+flavour+'_'+pttag,title=figtitle )
