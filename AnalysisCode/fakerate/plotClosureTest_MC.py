######################################
# plot the result of MC closure test #
######################################

import os
import sys
import ROOT
sys.path.append('../plotting/')
import histplotter as hp
sys.path.append('../tools')
import histtools as ht

inputfiles = ([f for f in os.listdir(os.getcwd()) 
		    if ('closurePlots_MC' in f and f[-5:]=='.root') ])
if len(sys.argv)>1:
    inputfiles = sys.argv[1:]

for f in inputfiles:

    print('running on {}'.format(f))

    # set output directory
    outdir = f.replace('.root','')
    if not os.path.exists(outdir):
	os.makedirs(outdir)

    # set figure base name
    figbasename = outdir.replace('closurePlots_MC_','')

    # determine year
    year = 'all'
    if('2016' in f): year = '2016'
    if('2017' in f): year = '2017'
    if('2018' in f): year = '2018'
    lumimap = {'all':137100,'2016':35900,'2017':41500,'2018':59700}
    lumi = lumimap[year]

    histlist = ht.loadallhistograms(f)

    # get a list of variables
    observedhists = ht.selecthistograms(histlist,mustcontainall=['observed'])[1]
    names = [h.GetName() for h in observedhists]
    variables = []
    for name in names:
	var = name.split('_observed')[0]
	if var not in variables: variables.append(var)
    print('found following variables: '+str(variables))

    for var in variables:
	print('running on variable {}'.format(var))
	observedhists = ht.selecthistograms(histlist,mustcontainall=[var+'_','observed'])[1]
	predictedhists = ht.selecthistograms(histlist,mustcontainall=[var+'_','predicted'],
					    maynotcontainone=['heavy','light','other'])[1]
    
	observedhist = observedhists[0].Clone()
	for h in observedhists[1:]: observedhist.Add(h)
	predictedsum = predictedhists[0].Clone()
	for h in predictedhists[1:]: predictedsum.Add(h)

	predictedsyst = predictedsum.Clone()
	for i in range(0,predictedsum.GetNbinsX()+2):
	    predictedsyst.SetBinContent(i,predictedsum.GetBinContent(i)*0.3)

	xaxtitle = observedhist.GetXaxis().GetTitle()
	yaxtitle = observedhist.GetYaxis().GetTitle()

	for h in predictedhists: 
	    process = h.GetName().split('_predicted_')[1].split('_')[0]
	    h.SetTitle(process)
	observedhist.SetTitle("MC Observed")

	colormap = {'TT': ROOT.kMagenta-7,'DY':ROOT.kMagenta+1}
	labelmap = ({'TT':'FR '+r'(t#bar{t})', 
		    'DY':'FR (DY)'})
	
	hp.plotdatavsmc( os.path.join(outdir,var+'_'+figbasename), observedhist, 
			    predictedhists, mcsysthist=predictedsyst,
			    datalabel='MC Obs.', 
			    colormap=colormap, labelmap=labelmap,
			    xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi)
	hp.plotdatavsmc( os.path.join(outdir,var+'_'+figbasename+'_log'), observedhist, 
			    predictedhists, mcsysthist=predictedsyst, 
			    datalabel='MC Obs.',
                            colormap=colormap, labelmap=labelmap,
                            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
			    yaxlog=True)

    
