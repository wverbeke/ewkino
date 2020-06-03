####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = ['wzcontrolregion','zzcontrolregion','zgcontrolregion']
years = ['2016','2017','2018']
events = ['MC','data']
variation = 'nominal'

for region in regions:
    for year in years:
	for eventtype in events:
	    inputfolder = '~/Files/tthid/'+region+'/'+year+eventtype
	    #inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'+year+eventtype
	    #samplelist = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/'
	    samplelist = '../samplelists/'
	    samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
	    #outputfolder = '~/Files/tzqid/reference5/signalregion/'+year+eventtype+'_flat'
	    outputfolder = '~/Files/tthid/'+region+'/'+year+eventtype+'_flat_reweighting'
	    cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder
	    cmd += ' '+region+' '+variation
	    print('executing '+cmd)
	    os.system(cmd)
