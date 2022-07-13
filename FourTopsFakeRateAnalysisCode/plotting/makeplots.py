##############
# make plots #
##############

# input histograms are supposed to be contained in a single root file.
# the naming of the histograms should be <process name>_<variable name>_<systematic>.
# note that only "nominal" is supported as systematic for now.

import sys
import os
sys.path.append('../../Tools/python')
import histtools as ht
sys.path.append('../../plotting/python')
import histplotter as hp
import colors

variables = (
    {'name':'_Mjj_max','title':r'm_{jet+jet}^{max}','unit':'GeV'},
    {'name':'_lW_asymmetry','title':r'Asymmetry (lepton from W)','unit':''},
    {'name':'_deepCSV_max','title':r'Highest b tagging discriminant','unit':''},
    {'name':'_deepFlavor_max','title':r'Highest b tagging discriminant','unit':''},
    {'name':'_lT','title':'L_{T}','unit':'GeV'},
    {'name':'_MT','title':'Transverse W boson mass','unit':'GeV'},
    {'name':'_smallRangeMT','title':'Transverse W boson mass','unit':'GeV'},
    {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
    {'name':'_dPhill_max','title':r'\Delta \Phi (l,l)_{max}','unit':''},
    {'name':'_HT','title':r'H_{T}','unit':'GeV'},
    {'name':'_nJets','title':r'Number of jets','unit':''},
    {'name':'_nBJets','title':r'Number of b-tagged jets','unit':''},
    {'name':'_M3l','title':r'm_{3l}','unit':'GeV'},
    {'name':'_altBinnedM3l','title':r'm_{3l}','unit':'GeV'},
    {'name':'_fineBinnedM3l','title':r'm_{3l}','unit':'GeV'},
    {'name':'_nMuons','title':r'Number of muons in event','unit':''},
    {'name':'_nElectrons','title':r'Number of electrons in event','unit':''},
    {'name':'_yield','title':r'Total yield','unit':''},
    {'name':'_leptonPtLeading','title':r'Leading lepton p_{T}','unit':'GeV'},
    {'name':'_leptonPtSubLeading','title':r'Subleading lepton p_{T}','unit':'GeV'},
    {'name':'_leptonPtTrailing','title':r'Trailing lepton p_{T}','unit':'GeV'},
    {'name':'_fineBinnedleptonPtTrailing','title':r'Trailing lepton p_{T}','unit':'GeV'},
    {'name':'_leptonEtaLeading','title':r'Leading lepton \eta','unit':''},
    {'name':'_leptonEtaSubLeading','title':r'Subleading lepton \eta','unit':''},
    {'name':'_leptonEtaTrailing','title':r'Trailing lepton \eta','unit':''},
    {'name':'_jetPtLeading','title':r'Leading jet p_{T}','unit':'GeV'},
    {'name':'_jetPtSubLeading','title':r'Subleading jet p_{T}','unit':'GeV'},
    {'name':'_numberOfVertices','title':r'Number of vertices','unit':''},
    {'name':'_bestZMass','title':r'Mass of OSSF pair','unit':'GeV'},
    )


def getmcsysthist(mchistlist, npunc=0.3):
    syshist = mchistlist[0].Clone()
    syshist.Reset()
    totmchist = mchistlist[0].Clone()
    totmchist.Reset()
    nphist = None
    for hist in mchistlist:
	totmchist.Add(hist)
	syshist.Add(hist)
	if hist.GetTitle()=="nonprompt": nphist = hist.Clone()
    if nphist is None: return None
    if nphist.GetBinContent(1)<1e-4: nphist.SetBinContent(1,0.)
    syshist.Add(nphist,npunc)
    syshist.Add(totmchist,-1)
    return syshist


if __name__=="__main__":

    inputfile = sys.argv[1]
    year = sys.argv[2]
    region = sys.argv[3]
    outputdir = sys.argv[4]
    doblind = sys.argv[5]

    # read all histograms
    histlist = ht.loadallhistograms(inputfile)
    histlist = ht.selecthistograms(histlist,mustcontainall=[region])[1]

    # make output directory
    if not os.path.exists(outputdir): os.makedirs(outputdir)

    # loop over variables
    for el in variables:
	varname = el['name']
	title = el['title']
	# select histograms
	thishists = ht.selecthistograms(histlist, mustcontainall=[varname])[1]
	# find data and sim histograms
	datahists = []
	simhists = []
	for hist in thishists:
	    if 'data' in hist.GetName(): datahists.append(hist)
	    else: simhists.append(hist)
	if len(datahists)!=1:
	    raise Exception('ERROR: expecting one data histogram'
			    +' but found {}'.format(len(datahists)))
	datahist = datahists[0]

	# blind data histogram
	if doblind:
	    for i in range(0,datahist.GetNbinsX()+2):
		datahist.SetBinContent(i, 0)
		datahist.SetBinError(i, 0)

	# set plot properties
	xaxtitle = title
	yaxtitle = 'Events'
	outfile = os.path.join(outputdir, varname)
	lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
		    '2016PreVFP':19520, '2016PostVFP':16810 }
	lumi = lumimap[year]
	extracmstext = 'Preliminary'
	#extracmstext = 'StandsWithUkraine'
	colormap = colors.getcolormap(style='default')
	#colormap = colors.getcolormap(style='ukraine')
	npunc = 0.3
	simsysthist = getmcsysthist(simhists, npunc=npunc)

	# make the plot
	hp.plotdatavsmc(outfile, datahist, simhists,
	    mcsysthist=simsysthist, 
	    xaxtitle=xaxtitle,
	    yaxtitle='Number of events',
	    colormap=colormap,
	    lumi=lumi, extracmstext=extracmstext )
