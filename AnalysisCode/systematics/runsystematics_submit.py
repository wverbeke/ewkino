####################################################################################################
# A very simple submitter that runs runsystematics.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
regions.append('signalregion')
regions.append('wzcontrolregion')
regions.append('zzcontrolregion')
regions.append('zgcontrolregion')

years = ['2018']
events = ['MC','data']

selection_types = []
selection_types.append('3tight')
selection_types.append('3prompt')
selection_types.append('fakerate')

outputfolder = 'output_tzqid' # only top-level directory needed here

for year in years:
    for eventtype in events:
	for region in regions:
	    for stype in selection_types:
		inputfolder = '~/Files/tzqid/'+year+eventtype+'/'+region+'_'+stype
		samplelist = '../samplelists/'
		samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
		# for control regions: run once
		signal_categories = [0]
		# for signal region and sidebands: run for each category
		if('signalregion' in region or 'signalsideband' in region): signal_categories = [1,2,3]

		# make and run command(s)
		for i in signal_categories:
		    suffix = ''
		    if i>0: suffix = '_'+str(i)
		    thisoutputfolder = os.path.join(outputfolder,year+eventtype,region+suffix+'_'+stype)
		    cmd = 'python runsystematics.py '+inputfolder+' '+samplelist+' '
		    cmd += thisoutputfolder+' '+region+' '+stype+' '+str(i)
		    print('executing '+cmd)
		    os.system(cmd)
