#######################################################################################
# loop over the search regions and produce a txt file as input for channelsystplotter #
#######################################################################################

import sys 
import os
import numpy as np
sys.path.append(os.path.abspath('../tools/'))
import histtools as ht

topdir = sys.argv[1]

variable = '_yield'
years = ['2016','2017','2018']
regions = []
for cat in ['_cat1','_cat2','_cat3']:
    #for ch in ['','_ch0','_ch1','_ch2','_ch3']:
    for ch in ['']:
	regions.append('signalregion'+cat+ch)

totdict = {}
for year in years:
    for region in regions:

	# read the histograms
	filepath = os.path.join(topdir,year+'combined',region,'npfromdata','combined.root')
	histlist = ht.loadallhistograms_select(filepath,variable)
	histdict = {}
	for hist in histlist: histdict[hist.GetName()] = hist
	# get a list of all systematics and processes
	slist = []
	plist = []
	for key in histdict:
	    systematic = key.split(variable)[-1].strip('_')
	    if(systematic[-2:]=='Up'): systematic = systematic[:-2]
	    elif(systematic[-4:]=='Down'): systematic = systematic[:-4]
	    if systematic == 'nominal':
		process = key.split(variable)[0].strip('_')
		if not process in plist: plist.append(process)
	    elif( not systematic in slist ): slist.append(systematic)
	if 'data' in plist: plist.remove('data')
	
	# need to distinguish systematics uncorrelated per process and year
	slist_uncorrp = []
	slist_uncorry = []
	slist_corr = []
	for systematic in slist:
	    isuncorrp = False
	    isuncorry = False
	    for p in plist:
		if p in systematic:
		    isuncorrp = True
		    sysname = systematic.replace('_'+p,'')
		    if sysname not in slist_uncorrp: slist_uncorrp.append(sysname)
	    if year in systematic:
		isuncorry = True
		sysname = systematic.replace('_'+year,'')
		if sysname not in slist_uncorry: slist_uncorry.append(sysname)
	    if not (isuncorrp or isuncorry): slist_corr.append(systematic)

	del slist

	sumdict = {}
	# compute nominal
	nomyield = 0
        for p in plist:
            nomyield += histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
	# compute systematic variations for fully correlated systematics
	for systematic in slist_corr:
	    upyield = 0
	    downyield = 0
	    for p in plist:
		if( p+'_'+variable+'_'+systematic+'Up' in histdict.keys()
		    and p+'_'+variable+'_'+systematic+'Down' in histdict.keys() ):
		    # note: maybe replace BinContent by Integral in order to user other variables
		    upyield += histdict[p+'_'+variable+'_'+systematic+'Up'].GetBinContent(1)
		    downyield += histdict[p+'_'+variable+'_'+systematic+'Down'].GetBinContent(1)
		else:
		    upyield += histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
                    downyield += histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
		    print('### WARNING ###: problem with {}/{}.'.format(p,systematic))
	    sumdict[systematic] = (upyield/nomyield,downyield/nomyield)
	# compute systematic variations for systematics split per year
	# (not different from fully correlated ones except for naming)
	for systematic in slist_uncorry:
            upyield = 0
            downyield = 0
	    fullname = systematic+'_'+year
            for p in plist:
                if( p+'_'+variable+'_'+fullname+'Up' in histdict.keys()
                    and p+'_'+variable+'_'+fullname+'Down' in histdict.keys() ):
                    # note: maybe replace BinContent by Integral in order to user other variables
                    upyield += histdict[p+'_'+variable+'_'+fullname+'Up'].GetBinContent(1)
                    downyield += histdict[p+'_'+variable+'_'+fullname+'Down'].GetBinContent(1)
                else:
		    upyield += histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
                    downyield += histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
                    print('### WARNING ###: problem with {}/{}.'.format(p,systematic))
	    sumdict[systematic] = (upyield/nomyield,downyield/nomyield)
	# compute systematic variations for systematics split per process
	# (need to sum quadratically instead of linearly)
	for systematic in slist_uncorrp:
	    maxsqvar = 0
	    for p in plist:
		fullname = systematic+'_'+p
		thisnomyield = histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
		if( p+'_'+variable+'_'+fullname+'Up' in histdict.keys()
                    and p+'_'+variable+'_'+fullname+'Down' in histdict.keys() ):
                    # note: maybe replace BinContent by Integral in order to user other variables
                    upyield = histdict[p+'_'+variable+'_'+fullname+'Up'].GetBinContent(1)
                    downyield = histdict[p+'_'+variable+'_'+fullname+'Down'].GetBinContent(1)
                else:
		    upyield = histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
                    downyield = histdict[p+'_'+variable+'_nominal'].GetBinContent(1)
                    print('### WARNING ###: problem with {}/{}.'.format(p,systematic))
		maxsqvar += max((upyield-thisnomyield)**2,(downyield-thisnomyield)**2)
	    sumdict[systematic] = ((nomyield+np.sqrt(maxsqvar))/nomyield,
				    (nomyield-np.sqrt(maxsqvar)/nomyield))
	
	# set the label
	label = year+'_'+region.replace('signalregion','')
	totdict[label] = sumdict

with open('test.txt','w') as f:
    
    for label in totdict.keys():
	for sys in totdict[label].keys():
	    vals = totdict[label][sys]
	    line = label+' '+sys+' '+str(vals[0])+' '+str(vals[1])+'\n'
	    f.write(line)
