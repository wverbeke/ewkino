###################################################################################################
# python scripts for comparing histograms: read histograms from different files and plot together #
###################################################################################################
# note: the input files are assumed to already contain the histograms, this script is only for plotting
# except for an additional normalization to compare shapes
import sys
import os
import ROOT
from singlehistplotter import plothistograms,loadhistograms

def getBDTvariables():
    variables = (['_abs_eta_recoil','_Mjj_max','_lW_asymmetry','_deepCSV_max',
		'_lT','_MT','_pTjj_max','_dRlb_min','_dPhill_max','_HT','_nJets',
		'_dRlWrecoil','_dRlWbtagged','_M3l','_abs_eta_max','_eventBDT',
		'_nMuons','_nElectrons'])
    return variables

def getfilebytag(region,year,tag,is2tight):
    path = os.path.abspath('../systematics/output_new/'+region+'/'+year+'MC')
    if is2tight: path = os.path.abspath('../systematics/output_2tight/'+region+'/'+year+'MC_2tight')
    fnames = [f for f in os.listdir(path) if tag in f]
    if len(fnames)>1: 
	print('### ERROR ###: more than one file meets selection requirements')
	sys.exit()
    if len(fnames)==0: 
	print('### ERROR ###: no file matches selection requirements')
	sys.exit()
    return os.path.join(path,fnames[0])

def getDYfile(region,year,is2tight):
    return getfilebytag(region,year,'DYJetsToLL_M-50',is2tight)

def getTTBarfile(region,year,is2tight):
    if year in ['2016']:
	return getfilebytag(region,year,'TTJets_DiLept',is2tight)
    else: return getfilebytag(region,year,'TTTo2L2Nu',is2tight)

if __name__=='__main__':

    plotdict = {}
    # a plot will be made for each element in plotdict;
    # each entry in plotdict matches a figname to a list of 
    # dicts with keys 'file', 'histname' and 'label'

    region = 'signalregion_1'
    year = '2016'

    variables = getBDTvariables()

    for v in variables:
	figname = os.path.abspath('doublehistplotter_output/'+v+'.png')
	plotdict[figname] = []
	histname = 'nonprompt_'+v+'_nominal'
	f1 = getTTBarfile(region,year,False)
	f2 = getTTBarfile(region,year,True)
	plotdict[figname].append({'file':f1,'histname':histname,'label':'3tight'})
	plotdict[figname].append({'file':f2,'histname':histname,'label':'2tight'})

    # at this stage plotdict is assumed to be filled, now make ready for plotting
    for plot in list(plotdict.keys()):
	
	histlist = []
	for el in plotdict[plot]:
	    allhistsinfile = loadhistograms(el['file'])
	    thishistlist = []
	    for hist in allhistsinfile:
		if hist.GetName()==el['histname']: 
		    thishistlist.append(hist)
	    if len(thishistlist)!=1: 
		print('### ERROR ###: unexpected number of histograms in file with given name')
		sys.exit()
	    histlist.append(thishistlist[0])
	    histlist[-1].SetTitle(el['label'])

	# normalize all histograms
	for hist in histlist:
	    hist.Scale(1./hist.Integral("width"))

	# call plotting function
	plothistograms(histlist,'arbitrary units','',plot,errorbars=True)
		
