##############################################################
# plotting script for the output files of testReweighterBTag #
##############################################################

import sys
import os
import ROOT
sys.path.append('../../AnalysisCode/tools')
import histtools as ht
sys.path.append('../../AnalysisCode/plotting')
import multihistplotter as mhp

# set input parameters

if( len(sys.argv)!=2 ):
    print('### ERROR ###: wrong number of arguments')
    sys.exit()

inputfile = sys.argv[1]
variables = (['_deepFlavor_max','_yield','_nJets',
		'_eventBDT',
		'_leptonPtLeading','_leptonPtSubLeading','_leptonPtTrailing',
		'_nBJets',
		'_HT','_lT'])
systematics = []
#if( '_wp' in inputfile):
#    systematics = ['heavy','light']
#if( '_shape' in inputfile ):
#    systematics = ['jes','cferr2','cferr1','hf','hfstats2','hfstats1','lf','lfstats2','lfstats1']

for variable in variables:
    # load and select the histograms
    histlist = ht.loadallhistograms(inputfile)
    histlist = ht.selecthistograms(histlist,mustcontainall=[variable],
		  mustcontainone=['noweight','nonorm','nominal']+systematics)[1]
    print('found following histograms:')
    for hist in histlist: print(' '+hist.GetName())

    # put histogram without weights in front (needed for correct ratio)
    # and histogram with nominal weights as second (for nicer plotting)
    idx = ht.selecthistograms(histlist,mustcontainall=['noweight'])[0]
    idx2 = ht.selecthistograms(histlist,mustcontainall=['nonorm'])[0]
    idx3 = ht.selecthistograms(histlist,mustcontainall=['nominal'])[0] 
    if( len(idx)!=1 or len(idx2)!=1 or len(idx3)!=1 ):
	print('### ERROR ###: cannot find histogram without reweighting or nominal')
	sys.exit()
    idx = idx[0]
    idx2 = idx2[0]
    idx3 = idx3[0]
    newhistlist = [histlist[idx],histlist[idx2],histlist[idx3]]
    for i,hist in enumerate(histlist):
	if( i!=idx and i!=idx2 and i!=idx3 ):
	    newhistlist.append(hist)
    histlist = newhistlist

    # make a colorlist and a labellist
    cmap = {'noweight':ROOT.kBlack,
	'nonorm': ROOT.kRed,
    	'nominal':ROOT.kBlue,
	'heavy':ROOT.kRed,
	'light':ROOT.kMagenta,
	'jes_up':ROOT.kGreen,'jes_down':ROOT.kGreen,
	'jesup':ROOT.kGreen,'jesdown':ROOT.kGreen,
	'hf_up':ROOT.kRed+4,'hf_down':ROOT.kRed+4,
	'hfup':ROOT.kRed+4,'hfdown':ROOT.kRed+4,
	'hfstats2':ROOT.kOrange-3,
	'hfstats1':ROOT.kRed+2,
	'lf_up':ROOT.kMagenta,'lf_down':ROOT.kMagenta,
	'lfup':ROOT.kMagenta,'lfdown':ROOT.kMagenta,
	'lfstats2':ROOT.kMagenta+2,
	'lfstats1':ROOT.kMagenta+4,
	'cferr2':ROOT.kViolet+1,
	'cferr1':ROOT.kViolet+3 }

    clist = []
    labellist = []
    for hist in histlist:
	for key,val in cmap.items():
	    if key in hist.GetName():
		clist.append(val)
		break
	#print(hist.GetName()+' -> '+str(val))
	label = hist.GetName()
	label = label.replace(variable,'')
	label = label.replace('bweight','')
	labellist.append(label)

    if( len(histlist)!=len(clist) ):
	print('### ERROR ###: something went wrong in color assignment')
	print(histlist)
	print(len(histlist))
	print(clist)
	print(len(clist))
    if( len(histlist)!=len(labellist) ):
	print('### ERROR ###: something went wrong in label assignment')
	print(histlist)
	print(len(histlist))
	print(labellist)
	print(len(labellist))

    # make the plot
    #outputfile = inputfile.replace('.root','_plot_'+variable)
    #mhp.plothistograms(histlist,clist,variable,'number of events',outputfile,
    #			    labellist=labellist,
    #			    errorbars=True,cliprange=True,ratiorange=(0.8,1.199))
    outputfile = inputfile.replace('.root','_plot_'+variable+'_norm')
    mhp.plothistograms(histlist,clist,variable,'number of events (normalized)',outputfile,
			    labellist=labellist,
		            errorbars=True,cliprange=True,ratiorange=(0.8,1.199),
			    normalizefirst=True)

