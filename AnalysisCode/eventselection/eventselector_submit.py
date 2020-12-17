####################################################################################################
# A very simple submitter that runs eventselection.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
regions.append('signalregion')
for r in ['zgcontrolregion','wzcontrolregion','zzcontrolregion']: regions.append(r)
for r in ['signalsideband_noossf','signalsideband_noz']: regions.append(r)

years = ['2016','2017','2018']
events = ['MC','data']

selection_types = []
selection_types.append('3tight')
selection_types.append('3prompt')
selection_types.append('fakerate')
#selection_types.append('2tight') # usually only for signalregion

for year in years:
    for eventtype in events:
	for region in regions:
	    for stype in selection_types:
		inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'+year+eventtype
		samplelist = '../samplelists/'
		samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
		outputfolder = '~/Files/tzqidmedium0p4_jetcut/'+year+eventtype+'/'+region+'_'+stype
		cmd = 'python eventselector.py '+inputfolder+' '+samplelist+' '+outputfolder
		cmd += ' '+region+' '+stype
		print('executing '+cmd)
		os.system(cmd)
