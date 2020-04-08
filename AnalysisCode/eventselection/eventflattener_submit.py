####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = ['signalregion','wzcontrolregion','zzcontrolregion','zgcontrolregion']
years = ['2016']
events = ['MC','data']
appendix = '_tZqID'

for region in regions:
    for year in years:
	for eventtype in events:
	    inputfolder = '~/Files/'+region+'/'+year+eventtype+appendix
	    samplelist = '~/ewkino/AnalysisCode/samplelists/samplelist_tzq_'+year+'_'+eventtype+'.txt'
	    outputfolder = '~/Files/'+region+'/'+year+eventtype+appendix+'_flat'
	    cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder+' '+region
	    print('executing '+cmd)
	    os.system(cmd)
