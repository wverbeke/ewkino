####################################################################################################
# A very simple submitter that runs eventselection.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = (['signalregion','zgcontrolregion','wzcontrolregion','zzcontrolregion',
	    'signalsideband_noossf','signalsideband_noz'])
years = ['2016','2017','2018']
events = ['MC','data']

for region in regions:
    for year in years:
        for eventtype in events:
            #inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/'+year+eventtype
            inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'+year+eventtype
	    outputfolder = '~/Files/tthid/'
	    outputfolder += region+'/'+year+eventtype
            cmd = 'python eventselector.py '+inputfolder+' '+outputfolder+' '+region
            print('executing '+cmd)
            os.system(cmd)

