#################################################################################################
# A very simple submitter that runs eventbinner.py for a number of predefined regions and years #
#################################################################################################

import os
import sys

regions = []
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

selection_types = []
#selection_types.append('3tight')
selection_types.append('3prompt')
selection_types.append('fakerate')

variation = 'nominal'
frdir = '../../fakeRate/fakeRateMaps' # put dummy path here if not using nonprompt from data

for year in years:
	for region in regions:
	    for stype in selection_types:
		# case 1: Liam's trilepton skims
		inputdir = '/pnfs/iihe/cms/store/user/lwezenbe/skimmedTuples/HNL/default/'
		inputyear = year.replace('PreVFP','pre').replace('PostVFP','post')
		inputdir += 'UL{}/Reco'.format(inputyear)
		samplelist = '../samplelists/'
		samplelist += 'samples_controlregions_{}.txt'.format(year)
		outputdir = 'output_20220215_npfromdata'
		outputdir = os.path.join(outputdir, year+'_'+region, stype)
		cmd = 'python eventbinner.py '+inputdir+' '+samplelist+' '+outputdir
		cmd += ' '+region+' '+stype+' '+variation+' '+frdir
		print('executing '+cmd)
		os.system(cmd)
