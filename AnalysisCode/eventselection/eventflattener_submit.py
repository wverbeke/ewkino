####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = ['signalregion']
years = ['2016','2017','2018']
events = ['MC']

for region in regions:
    for year in years:
	for eventtype in events:
	    inputfolder = '~/Files/'+region+'/'+year+eventtype
	    samplelist = '~/ewkino/AnalysisCode/samplelists/samplelist_tzq_'+year+'_'+eventtype+'.txt'
	    outputfolder = '~/Files/'+region+'/'+year+eventtype+'_flat'
	    cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder+' '+region
	    print('executing '+cmd)
	    os.system(cmd)
