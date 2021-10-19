####################################
# generating datacards for combine #
####################################
# tZq split into different lepton flavour channels (eee, eem, emm and mmm)
# note: this script belongs to the 'explicit splitting' method,
#       where the signal regions are split explicitly into the different channels.
#	the input histograms are assumed to contain the same signal name,
#       but a change of names will be implemented in the datacard depending on the channel.
#       one can also choose to split implicitly, i.e. keep the signal regions inclusive,
#	but split the signal process in subprocesses.
#       for this approach however, use inclusive_makedatacards with modified list of signals!

import os
import sys
import datacardtools as dct
import uncertaintytools as ut

def get_lepchannel_parameters():
    ### return all parameters needed to fully define the splitting
    # define as a function so it can be used in runcombine as well, 
    # since some info is shared and must be synced

    # define what processes to consider as signal (these will be split into lepton channels)
    signals = ['tZq']
    # define channels
    #lepchannels = {'_ch0':'_eee','_ch1':'_eem','_ch2':'_emm','_ch3':'_mmm'}
    lepchannels = {'_ch0':'_eee','_ch1':'_eem','_ch2':'_emm','_ch3':'_mmm'}
    #lepchannels = {'_ch0':'','_ch1':'','_ch2':'','_ch3':''}
    return (signals,lepchannels)

if __name__=="__main__":

    if len(sys.argv)<3:
	print('### ERROR ###: need different number of command line args:')
	print('               <input directory> and <datacard directory>')
	sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]

    # get lepton channel info
    (signals,lepchannels) = get_lepchannel_parameters()

    # define what histograms to read (regions, variables, years, npdatatype)
    npfromdata = True
    cnames = []
    for (lc,lcn) in lepchannels.items():
	cnames.append({'region':'signalregion_cat1'+lc,'var':'_eventBDT','lcn':lcn})
	cnames.append({'region':'signalregion_cat2'+lc,'var':'_eventBDT','lcn':lcn})
	cnames.append({'region':'signalregion_cat3'+lc,'var':'_eventBDT','lcn':lcn})
    #cnames.append({'region':'wzcontrolregion','var':'_MT','lcn':''})
    #cnames.append({'region':'zzcontrolregion','var':'_MT','lcn':''})
    #cnames.append({'region':'zgcontrolregion','var':'_nJets','lcn':''})
    #cnames.append({'region':'ttzcontrolregion','var':'_nBJets','lcn':''})
    #cnames.append({'region':'signalsideband_noossf_cat1','var':'_yield','lcn':''})
    #cnames.append({'region':'signalsideband_noz_cat1','var':'_yield','lcn':''})
    years = ['2016','2017','2018']

    # make a list of channel info dicts for easy looping
    channels = []
    suffix = 'npfromdata' if npfromdata else 'npfromsim'
    for c in cnames:
	region = c['region']
	var = c['var']
	lcn = c['lcn']	
	for year in years:
	    channels.append( {
			'name':region+'_'+year,
			'region':region,
			'year':year,
			'lcn':lcn,
			'path':os.path.join(topdir,year+'combined',region,suffix,'combined.root'),
			'var':c['var'] })

    if os.path.exists(datacarddir):
	os.system('rm -r '+datacarddir)
    os.makedirs(datacarddir)

    for channel in channels:
	# get region, year and variable to use
	name = channel['name']
	region = channel['region']
	year = channel['year']
	variable = channel['var']
	path = channel['path']
	lcn = channel['lcn']

	# check if histogram file exists
        if not os.path.exists(path):
            print('### WARNING ###: '+path+' not found, skipping it.')
            continue
        print('making datacard for '+path+'...')
	# get necessary info
        processinfo = dct.readhistfile(path,variable,signals)
	#print(processinfo)

	# treatment of uncertainties
        ut.disable_default( processinfo, year, region, npfromdata )
        normsyslist = ut.addnormsys_default( processinfo, year, region, npfromdata )

	# change name of signal processes
	if lcn in lepchannels.values():
	    for sp in signals:
		processinfo.changename( sp, sp+lcn )

	# remove overlap between shape and normalization uncertainties
	# (how to do this automatically in datacardtools?)
	shapesyslist = []
	for sys in processinfo.slist: 
	    if sys not in normsyslist: shapesyslist.append(sys)

	# remove superfluous systematics
        shapesyslist = ut.subselect_systematics(shapesyslist)
        print('will take into account the following shape systematics:')
        for sys in shapesyslist: print('  - '+sys)
        print('will take into account the following norm systematics:')
        for sys in normsyslist: print('  - '+sys)

	# write the datacard
	dct.writedatacard(datacarddir,name,processinfo,path,variable,
                    shapesyslist=shapesyslist,lnNsyslist=normsyslist,
                    writeobs=False,autof=True)
