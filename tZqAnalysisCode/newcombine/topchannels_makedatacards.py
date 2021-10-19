####################################
# generating datacards for combine #
####################################
# tZq split into different top charge channels (top and antitop)
# note: this script belongs to the 'explicit splitting' method,
#       where the signal regions are split explicitly into the different channels.
#       the input histograms are assumed to contain the same signal name,
#       but a change of names will be implemented in the datacard depending on the channel.
#       one can also choose to split implicitly, i.e. keep the signal regions inclusive,
#       but split the signal process in subprocesses.
#       for this approach however, use inclusive_makedatacards with modified list of signals!

import os
import sys
import datacardtools as dct
import uncertaintytools as ut

def get_topchannel_parameters():
    ### return all parameters needed to fully define the splitting
    # define as a function so it can be used in runcombine as well, 
    # since some info is shared and must be synced

    # define what processes to consider as signal (these will be split into top channels)
    signals = ['tZq']
    # define channels
    topchannels = {'_top':'_top','_antitop':'_antitop'}
    return (signals,topchannels)

if __name__=="__main__":

    if len(sys.argv)<3:
	print('### ERROR ###: need different number of command line args:')
	print('               <input directory> and <datacard directory>')
	sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]

    # get top channel info
    (signals,topchannels) = get_topchannel_parameters()
    #to_bkg = [] # needed for syntax (see below)
    #ratio = [] # needed for syntax (see below)
    # for ratio measurement: 
    # - redefine only tZq_top or tZq_antitop as signal (but not both)
    # - introduce a rateParam to link both yields (automatically via 'ratio' argument)
    to_bkg = ['tZq','tZq_antitop']
    ratio = ['tZq_*']

    # define what histograms to read (regions, variables, years, npdatatype)
    npfromdata = True
    cnames = []
    for (tc,tcn) in topchannels.items():
    	cnames.append({'region':'signalregion_cat1'+tc,'var':'_eventBDT','tcn':tcn})
    	cnames.append({'region':'signalregion_cat2'+tc,'var':'_eventBDT','tcn':tcn})
    	cnames.append({'region':'signalregion_cat3'+tc,'var':'_eventBDT','tcn':tcn})
    #	cnames.append({'region':'signalregion_cat123'+tc,'var':'_eventBDT','tcn':tcn})
    #cnames.append({'region':'wzcontrolregion','var':'_MT','tcn':''})
    #cnames.append({'region':'zzcontrolregion','var':'_MT','tcn':''})
    #cnames.append({'region':'zgcontrolregion','var':'_nJets','tcn':''})
    #cnames.append({'region':'ttzcontrolregion','var':'_nBJets','tcn':''})
    #cnames.append({'region':'signalsideband_noossf_cat1','var':'_yield','tcn':''})
    #cnames.append({'region':'signalsideband_noz_cat1','var':'_yield','tcn':''})
    years = ['2016','2017','2018']

    # make a list of channel info dicts for easy looping
    channels = []
    suffix = 'npfromdata' if npfromdata else 'npfromsim'
    for c in cnames:
	region = c['region']
	var = c['var']
	tcn = c['tcn']	
	for year in years:
	    channels.append( {
			'name':region+'_'+year,
			'region':region,
			'year':year,
			'tcn':tcn,
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
	tcn = channel['tcn']

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
	if tcn in topchannels.values():
	    for sp in signals:
		processinfo.changename( sp, sp+tcn )

	# set a part of the signal to background (e.g. for ratio measurement)
	for p in to_bkg:
	    if p in processinfo.plist:
		processinfo.makebkg( p ) 

        # define rate parameters 
        # (don't forget to check normalization uncertainties in addnormsys_default!)
        #rateparamlist = ['WZ','ZZH']
        rateparamlist = []

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
		    ratio=ratio,
		    writeobs=False,
                    autof=True)
