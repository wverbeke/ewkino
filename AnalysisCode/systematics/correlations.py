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
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls

def getcorrelations():
    # define correlations to be implemented
    uncorrelated_years = ['JER','Uncl','bTag_heavy','bTag_light']
    uncorrelated_processes = ['scales','rScale','fScale']
    partially_correlated_years = []
    # partial correlations not yet implemented.
    # idea is to make e.g. [{'JER':0.5},...] and to include degree of correlation in histogram name
    return {'uncorrelated_years':uncorrelated_years,
	    'uncorrelated_processes':uncorrelated_processes,
	    'partially_correlated_years':partially_correlated_years}

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

    # loop over histograms and copy to new list, some with modified naming
    newhists = []
    for hist in allhists:
	histname = hist.GetName()
	process = histname.split('_')[0] # depends on arbitrary naming convention!
	newname = histname
	for systematic in uncorrelated_years:
	    if systematic in histname:
		newname = newname.replace(systematic,systematic+'_'+year)
	for systematic in uncorrelated_processes:
	    if systematic in histname:
		newname = newname.replace(systematic,systematic+'_'+process)
	hist.SetName(newname)
	newhists.append(hist)	

    # open output file and write all histograms
    f = ROOT.TFile.Open(outputfilename,'recreate')
    for hist in newhists:
	hist.Write()
    f.Close()
