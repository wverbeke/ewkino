####################################################################
# script to deal with correlations in the output of runsystematics #
####################################################################

# by default, all systematics are correlated across years and processes
# (as the systematic name does not include the year or the process).
# this script renames some histograms (depending on the configuration defined below)
# introducing the year or process tag in the systematics name, 
# so combine will treat it uncorrelated.

# script does not need to be used on its own, it is integrated in mergeoutput.py

import sys
import os
import ROOT
import numpy as np
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls

def getcorrelations():
    # define correlations to be implemented
    uncorrelated_years = (['_JER','_Uncl','_bTag_heavy','_bTag_light',
			    '_muonIDStat','_electronIDStat'])
    uncorrelated_processes = ['_qcdScalesShapeEnv','_rScale','_fScale','_rfScales']
    #partially_correlated_years = {'prefire':0.5,'pileup':0.3} # just to test, not realistic
    partially_correlated_years = {}
    # partial correlations not yet implemented in combine code!
    return {'uncorrelated_years':uncorrelated_years,
	    'uncorrelated_processes':uncorrelated_processes,
	    'partially_correlated_years':partially_correlated_years}

def getpcorrfactors(correlationvalue):
    # input is a given correlation value (e.g. 0.5 = 50% correlated over the years)
    # output is a tuple of two scaling factors, first for correlated part and second for uncorrelated part
    # note: exact implementation not sure yet, e.g. 0.5 -> (0.5,1-0.5) or (sqrt(0.5),sqrt(1-0.5)) ?    
    return (np.sqrt(correlationvalue),np.sqrt(1-correlationvalue))

def implementcorrelations(histfilename,outputfilename):
    # main function in this script

    # first get year to which this file belongs
    year = ''
    if '2016' in histfilename: year = '2016'
    elif '2017' in histfilename: year = '2017'
    elif '2018' in histfilename: year = '2018'
    else:
	print('### WARNING ###: could not guess year from file name, returning...')
	return

    # load all histograms in this file
    allhists = tls.loadallhistograms(histfilename)

    # get correlation settings
    conf = getcorrelations()
    uncorrelated_years = conf['uncorrelated_years']
    uncorrelated_processes = conf['uncorrelated_processes']
    partially_correlated_years = conf['partially_correlated_years']

    # loop over histograms and copy to new list, some with modified naming
    newhists = []
    for hist in allhists:
	histname = hist.GetName()
	process = histname.split('_')[0] # depends on arbitrary naming convention!
	newnames = [histname] # for partial correlations will need multiple names
	# first modify newname based on full decorrelations in years and/or processes
	for systematic in uncorrelated_years:
	    if systematic in histname:
		newnames[0] = newnames[0].replace(systematic,systematic+'_'+year)
	for systematic in uncorrelated_processes:
	    if systematic in histname:
		newnames[0] = newnames[0].replace(systematic,systematic+'_'+process)
	# next treat partial correlations between years
	for systematic in partially_correlated_years.keys():
            if systematic in histname:
                basename = newnames[0]
		pcorrfactors = getpcorrfactors(partially_correlated_years[systematic])
		
		newnames[0] = basename.replace(systematic,systematic+'_pcorr'
						+'{:.2f}'.format(pcorrfactors[0]).replace('.','p'))
		newnames.append(basename.replace(systematic,systematic+'_'+year+'_pcorr'
						+'{:.2f}'.format(pcorrfactors[1]).replace('.','p')))
	for name in newnames:
	    newhist = hist.Clone()
	    newhist.SetName(name)
	    newhists.append(newhist)

    # open output file and write all histograms
    f = ROOT.TFile.Open(outputfilename,'recreate')
    for hist in newhists:
	hist.Write()
    f.Close()
