####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)

years = ['2017']
events = ['MC','data']

selection_types = []
selection_types.append('3tight')
#selection_types.append('3prompt')
#selection_types.append('fakerate')

variation = 'nominal'
frdir = '../fakerate/fakeRateMaps' # put dummy path here if not using nonprompt from data

for year in years:
    for eventtype in events:
	for region in regions:
	    for stype in selection_types:
		# case 1: Liam's trilepton skims
		inputdir = '/pnfs/iihe/cms/store/user/lwezenbe/skimmedTuples/HNL/default/'
		inputdir += '{}/Reco'.format(year)
		samplelist = '../samplelists/'
		outputdir = '~/Files/test'
		outputdir = os.path.join(outputdir, year+eventtype, region+'_'+stype+'_flat')
		cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder
		cmd += ' '+region+' '+stype+' '+variation+' '+frdir
		print('executing '+cmd)
		os.system(cmd)
