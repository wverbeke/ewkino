####################################################################################################
# A very simple submitter that runs runsystematics.py for a number of predefined regions and years #
####################################################################################################

import os
import sys

regions = []
regions.append('signalregion')
#regions.append('wzcontrolregion')
#regions.append('zzcontrolregion')
#regions.append('zgcontrolregion')
#regions.append('ttzcontrolregion')
#regions.append('signalsideband_noossf')
#regions.append('signalsideband_noz')
#regions.append('signalsideband')
# (list of region strings)

years = ['2016','2017','2018']
# (list of year strings)
datatypes = ['MC','data']
# (list of data type strings: 'MC' and/or 'data')
splitsamples = False
# (whether to use sample splitting as defined in splitSampleTools.cc)
channels = [['all']]
#channels = [['eee'],['eem'],['emm'],['mmm']]
# (list of lists of channel strings, 'eee','eem','emm','mmm' or 'all')
topcharges = ['all']
#topcharges = ['top','antitop']
# (list of top charge strings, 'top', 'antitop' or 'all')
bdtcut = 0.5

#selection_types = []
#selection_types.append('3tight')
#selection_types.append('3prompt')
#selection_types.append('fakerate')
# temp: set selection types below (depending on MC or data)
# (list of selection type strings, '3tight','3prompt' and/or 'fakerate')

bdt_combine_mode = 'all' # choose from 'all', 'years', 'regions' or 'none'

outputfolder = 'output_tzqidmedium0p4_newfiles_bdtcut0p5' # only top-level directory needed here

for year in years:
    for datatype in datatypes:
	if datatype=='MC': 
	    selection_types = []
	    selection_types.append('3prompt')
	    selection_types.append('fakerate')
	elif datatype=='data':
	    selection_types = []
	    selection_types.append('3tight')
	    selection_types.append('fakerate')
	for region in regions:
	    for channel in channels:
		for topcharge in topcharges:
		    for stype in selection_types:

			# set input folder and samplelist
			#inputfolder = ('~/Files/tzqidmedium0p4/'+year+datatype
			#		+'/'+region+'_'+stype)
			#inputfolder = ('/pnfs/iihe/cms/store/user/llambrec/trileptonskim/'
			#		+year+datatype)
			inputfolder = ('/pnfs/iihe/cms/store/user/llambrec/trileptonskim_new/'
					    +year+datatype)
			#samplelist = '../samplelists/samplelists_tzq_v_iTuple/'
			samplelist = '../samplelists/'
			samplelist += 'samplelist_tzq_'+year+'_'+datatype+'.txt'
		    
			# set event categories
			# for control regions: run once
			signal_categories = ['0']
			# for signal region and sidebands: run for categories
			# remark: can put e.g. '123' which implies events from 1, 2 and 3 taken together!
			if('signalregion' in region): 
			    signal_categories = ['1','2','3','123']
			elif('signalsideband' in region): 
			    signal_categories = ['1','123']

			# set channels
			signal_channel = ''
			for c in channel:
			    if c=='all': 
				signal_channel = '4'
				break
			    else: signal_channel += str(c.count('m'))
			
			# make and run command(s)
			for signal_category in signal_categories:
			    category_suffix = ''
			    if signal_category!='0': category_suffix = '_cat'+str(signal_category)
			    channel_suffix = ''
			    if signal_channel!='4': channel_suffix = '_ch'+str(signal_channel)
			    topcharge_suffix = ''
			    if topcharge!='all': topcharge_suffix = '_'+str(topcharge)
			    selection_suffix = '_'+stype
			    thisoutputfolder = os.path.join(outputfolder,year+datatype,
							region
							+category_suffix
							+channel_suffix
							+topcharge_suffix
							+selection_suffix)
			    cmd = 'python runsystematics.py '+inputfolder+' '+samplelist+' '
			    cmd += thisoutputfolder+' '+region+' '+stype
			    cmd += ' '+str(signal_category)+' '+str(splitsamples)
			    cmd += ' '+str(signal_channel)+' '+str(topcharge)
			    cmd += ' '+bdt_combine_mode+' '+str(bdtcut)
			    print('executing '+cmd)
			    os.system(cmd)
