####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
regions.append('signalregion')
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#regions.append(['signalsideband_noossf','signalsideband_noz'])

years = ['2016']
events = ['MC','data']

selection_types = []
selection_types.append('3tight')
selection_types.append('3prompt')
selection_types.append('fakerate')
#selection_types.append('2tight')

variation = 'nominal'
do_mva = 'False' # choose from 'True' or 'False'
path_to_xml_file = '../bdt/outdata/weights/tmvatrain_BDT.weights.xml' # put dummy value if do_mva = False
frdir = '../fakerate/fakeRateMaps'

for year in years:
    for eventtype in events:
	for region in regions:
	    for stype in selection_types:
		inputfolder = '~/Files/oldtzqid/'+year+eventtype+'/'+region+'_'+stype
		#inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'+year+eventtype
		samplelist = '../samplelists/'
		samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
		outputfolder = '~/Files/oldtzqid/'+year+eventtype+'/'+region+'_'+stype+'_flat'
		cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder
		cmd += ' '+region+' '+stype+' '+variation+' '+do_mva+' '+path_to_xml_file+' '+frdir
		print('executing '+cmd)
		os.system(cmd)

