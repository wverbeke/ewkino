####################################################################################################
# A very simple submitter that runs eventflattener.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = ['signalregion']
#,'zgcontrolregion','signalsideband_noz','signalsideband_noossf']
years = ['2016','2017','2018']
events = ['MC']
variation = 'nominal'
do_mva = 'False' # choose from 'True' or 'False'
path_to_xml_file = '../bdt/outdata/weights/tmvatrain_BDT.weights.xml' # put dummy value if do_mva = False

for region in regions:
    for year in years:
	for eventtype in events:
	    #inputfolder = '~/Files/'+interpendix+'/'+region+'/'+year+eventtype
	    #inputfolder = '~/Files/tzqid/signalregion_wp0p6/'+year+eventtype
	    inputfolder = '~/Files/trileptonskim/'+year+eventtype
	    #samplelist = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/'
	    samplelist = '../samplelists/'
	    samplelist += 'samplelist_tzq_'+year+'_'+eventtype+'.txt'
	    #outputfolder = '~/Files/'+interpendix+'/'+region+'/'+year+eventtype+'_flat'
	    outputfolder = '~/Files/tzqid/reference5/signalregion/'+year+eventtype+'_flat'
	    cmd = 'python eventflattener.py '+inputfolder+' '+samplelist+' '+outputfolder
	    cmd += ' '+region+' '+variation+' '+do_mva+' '+path_to_xml_file
	    print('executing '+cmd)
	    os.system(cmd)
