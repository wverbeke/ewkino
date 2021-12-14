#########################################
# plot the results of a MC closure test #
#########################################

import os
import sys
import ROOT
sys.path.append('../plotting/python')
import histplotter as hp
sys.path.append('../Tools/python')
import histtools as ht

# global properties
systunc = 0.3 
# (relative systematic uncertainty to add)

# read input files from the command line or else read all input files in the current directory
inputfiles = []
if len(sys.argv)>1:
    inputfiles = sys.argv[1:]
else: inputfiles = ([f for f in os.listdir(os.getcwd()) 
	    if ('closurePlots_MC' in f and f[-5:]=='.root') ])

# loop over input files
for f in inputfiles:
    print('running on {}'.format(f))

    # set output directory
    outdir = f.replace('.root','')
    if not os.path.exists(outdir): os.makedirs(outdir)

    # set figure base name
    figbasename = outdir.replace('closurePlots_MC_','')

    # determine year
    year = 'all'
    if('2016' in f): year = '2016'
    if('2017' in f): year = '2017'
    if('2018' in f): year = '2018'
    lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700}
    lumi = lumimap[year]

    # determine type
    simtype = None
    if '_DY_' in outdir: simtype = 'DY'
    elif '_TT_' in outdir: simtype = 'TT'

    histlist = ht.loadallhistograms(f)

    # get a list of variables
    observedhists = ht.selecthistograms(histlist,mustcontainall=['observed'])[1]
    names = [h.GetName() for h in observedhists]
    variables = []
    for name in names:
	var = name.split('_observed')[0]
	if var not in variables: variables.append(var)
	print('found following variables: '+str(variables))

    # loop over variables
    for var in variables:
	print('running on variable {}'.format(var))

	# get the histograms
	observedhists = ht.selecthistograms(histlist,mustcontainall=[var+'_','observed'])[1]
	predictedhists = ht.selecthistograms(histlist,mustcontainall=[var+'_','predicted'],
		        maynotcontainone=['heavy','light','other'])[1]
	observedhist = observedhists[0].Clone()
	for h in observedhists[1:]: observedhist.Add(h)
	predictedsum = predictedhists[0].Clone()
	for h in predictedhists[1:]: predictedsum.Add(h)

	# add the systematic uncertainty
	predictedsyst = predictedsum.Clone()
	for i in range(0,predictedsum.GetNbinsX()+2):
	    predictedsyst.SetBinContent(i,predictedsum.GetBinContent(i)*0.3)

	# axis titles: get from histograms
	xaxtitle = observedhist.GetXaxis().GetTitle()
	yaxtitle = observedhist.GetYaxis().GetTitle()

        for h in predictedhists: 
	    process = h.GetName().split('_predicted_')[1].split('_')[0]
	    h.SetTitle(process)
	observedhist.SetTitle("MC Observed")

	# set other plot options
	colormap = {'TT': ROOT.kMagenta-7,'DY':ROOT.kMagenta+1}
	labelmap = ({'TT':'Misid. rate prediction',
                     'DY':'Misid. rate prediction'})
	legendbox = [0.5,0.5,0.92,0.9]
	extracmstext = 'Preliminary'
	extrainfos = []
	if simtype=='DY': extrainfos=['Drell-Yan simulation']
	if simtype=='TT': extrainfos=['t#bar{t} simulation']
    
	hp.plotdatavsmc( os.path.join(outdir,var+'_'+figbasename), observedhist, 
	        predictedhists, mcsysthist=predictedsyst,
	        datalabel='MC Obs.', p2yaxtitle='#frac{MC Obs.}{Pred.}',
	        colormap=colormap, labelmap=labelmap,
	        xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
	        p1legendncols=1,p1legendbox=legendbox,
	        extracmstext=extracmstext,
	        extrainfos=extrainfos)
	hp.plotdatavsmc( os.path.join(outdir,var+'_'+figbasename+'_log'), observedhist, 
	        predictedhists, mcsysthist=predictedsyst, 
	        datalabel='MC Obs.', p2yaxtitle='#frac{MC Obs.}{Pred.}',
                colormap=colormap, labelmap=labelmap,
                xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
	        p1legendncols=1,p1legendbox=legendbox,
	        extracmstext=extracmstext,
	        extrainfos=extrainfos,
	        yaxlog=True)
