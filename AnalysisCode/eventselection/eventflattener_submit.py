####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = ['signalregion']
#,'zgcontrolregion','signalsideband_noz','signalsideband_noossf']
years = ['2016']
events = ['MC']
leptonID = 'tth' # choose from 'tzq' or 'tth'
uncertainty = 'JECDown'
do_mva = 'False' # choose from 'True' or 'False'
path_to_xml_file = '../bdt/outdata_wp0p8_bdt2/weights/tmvatrain_BDT.weights.xml' # put dummy value if do_mva = False

for region in regions:
    for year in years:
	for eventtype in events:
	    if leptonID=='tth': interpendix = 'tthid'
	    elif leptonID=='tzq': interpendix = 'tzqid'
	    else:
		print('### ERROR ###: ID "'+leptonID+'" not recognized.')
		sys.exit()
	    #inputfolder = '~/Files/'+interpendix+'/'+region+'/'+year+eventtype
	    inputfolder = '~/Files/tthid/signalregion_test/'+year+eventtype
	    samplelist = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/'
	    #samplelist = '../samplelists/'
	    samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
	    #outputfolder = '~/Files/'+interpendix+'/'+region+'/'+year+eventtype+'_flat'
	    outputfolder = '~/Files/tthid/signalregion_test/'+year+eventtype+'_flat_JECDown'
	    cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder
	    cmd += ' '+region+' '+leptonID+' '+uncertainty+' '+do_mva+' '+path_to_xml_file
	    print('executing '+cmd)
	    os.system(cmd)
