####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
regions.append('signalregion')
#for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
#for r in ['signalsideband_noossf','signalsideband_noz']: regions.append(r)
#regions = ['wzcontrolregion']

years = ['2016','2017','2018']
events = ['MC']

selection_types = []
selection_types.append('3tight')
#selection_types.append('3prompt')
#selection_types.append('fakerate')
selection_types.append('2tight')

variation = 'nominal'
do_mva = 'False' # choose from 'True' or 'False'
path_to_xml_file = '../bdt/outdata/weights/tmvatrain_BDT.weights.xml' # put dummy value if do_mva = False
frdir = '../fakerate/fakeRateMaps' # put dummy path here if not using nonprompt from data

topdir = '~/Files/tzqidmedium0p4_forbdt'

for year in years:
    for eventtype in events:
	for region in regions:
	    for stype in selection_types:
		#inputfolder = '~/Files/tzqidmedium0p4/'+year+eventtype+'/'+region+'_'+stype
		inputfolder = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'+year+eventtype
		samplelist = '../samplelists/samplelists_tzq_v_iTuple/'
		samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
		outputfolder = os.path.join(topdir,year+eventtype+'/'+region+'_'+stype+'_flat')
		cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder
		cmd += ' '+region+' '+stype+' '+variation+' '+do_mva+' '+path_to_xml_file+' '+frdir
		print('executing '+cmd)
		os.system(cmd)
