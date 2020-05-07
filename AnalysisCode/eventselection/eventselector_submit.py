####################################################################################################
# A very simple submitter that runs eventselection.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = (['signalregion'])#,'zgcontrolregion',
	    #'signalsideband_noossf','signalsideband_noz'])
years = ['2016']
events = ['MC']
leptonID = 'tth' # choose from 'tzq' or 'tth'

for region in regions:
    for year in years:
        for eventtype in events:
            inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/'+year+eventtype
            #inputfolder = '/user/llambrec/Files/trileptonskim/'+year+eventtype
	    outputfolder = '~/Files/'
	    if leptonID=='tth': outputfolder += 'tthid/'
	    elif leptonID=='tzq': outputfolder += 'tzqid/'
	    else:
		print('### ERROR ###: id specifier "'+leptonID+'"not recognized')
		sys.exit()
	    #outputfolder += region+'/'+year+eventtype
	    outputfolder += 'signalregion_test/'+year+eventtype
            cmd = 'python eventselector.py '+inputfolder+' '+outputfolder+' '+region+' '+leptonID
            print('executing '+cmd)
            os.system(cmd)

