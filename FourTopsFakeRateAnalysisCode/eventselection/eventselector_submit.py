####################################################################################################
# A very simple submitter that runs eventselection.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
for r in ['zgcontrolregion','wzcontrolregion','zzcontrolregion']: regions.append(r)

years = ['2017']
events = ['MC','data']

selection_types = []
selection_types.append('3tight')
#selection_types.append('3prompt')
#selection_types.append('fakerate')

for year in years:
    for eventtype in events:
	for region in regions:
	    for stype in selection_types:
		inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'+year+eventtype
		samplelist = '../samplelists/samplelists_tzq_v_iTuple/'
		samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
		outputfolder = '~/Files/tzqidmedium0p4/'+year+eventtype+'/'+region+'_'+stype
		cmd = 'python eventselector.py '+inputfolder+' '+samplelist+' '+outputfolder
		cmd += ' '+region+' '+stype
		print('executing '+cmd)
		os.system(cmd)
