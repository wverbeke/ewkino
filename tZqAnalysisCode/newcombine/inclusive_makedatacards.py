####################################
# generating datacards for combine #
####################################
# - typical use case: most simple case for fully inclusive fit with 1 signal process.
#   note: fill the process to be considered as signal in the 'signals' list.
# - extension: this script can also be used for datacards with multiple signals!
#   simply fill the list called 'signals' with whatever processes should be considered as signal.
#   note: it is assumed of course that these proces names are present in the histograms.
#   note: the result is a datacard where the processes listed in 'signals' have a process ID<=0.
#   note: for the actual measurement, the defaultPhysicsModel does not suffice,
#         need to use multiSignalModel with a PO map; see e.g. topchannels_runcombine.py.
# - extension: this script can also be used for datacards suitable for ratio measurements.
#   simply edit the list called 'ratio' with two processes A and B, if you want to fit A/B.
#   (leave empty list for no ratio measurement)
#   note: make sure the 'signals' list contains only A!
#   note: the result is a datacard with A considered as signal but with an additional rateParam
#         that scales both A and B; hence B will be modified by the rateParam to its best fit,
#         A will be modified by the same value and additionally by the signal strength,
#         which then corresponds to a 'signal strength' for the ratio.

import os
import sys
import datacardtools as dct
import uncertaintytools as ut

if __name__=="__main__":

    if len(sys.argv)<3:
	print('### ERROR ###: need different number of command line args:')
	print('               <input directory> and <datacard directory>')
	sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]

    # define what histograms to read (regions, variables, years, npdatatype)
    npfromdata = True
    cnames = []
    cnames.append({'region':'signalregion_cat1','var':'_eventBDT'})
    cnames.append({'region':'signalregion_cat2','var':'_eventBDT'})
    cnames.append({'region':'signalregion_cat3','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat123','var':'_eventBDT'})
    cnames.append({'region':'wzcontrolregion','var':'_MT'})
    cnames.append({'region':'zzcontrolregion','var':'_nJets'})
    cnames.append({'region':'zgcontrolregion','var':'_nJets'})
    cnames.append({'region':'ttzcontrolregion','var':'_nBJets'})
    cnames.append({'region':'signalsideband_noossf_cat1','var':'_yield'})
    cnames.append({'region':'signalsideband_noz_cat1','var':'_yield'})
    #cnames.append({'region':'signalregion_cat1_top','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2_top','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3_top','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat1_antitop','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2_antitop','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3_antitop','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat1_ch0','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2_ch0','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3_ch0','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat1_ch1','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2_ch1','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3_ch1','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat1_ch2','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2_ch2','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3_ch2','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat1_ch3','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2_ch3','var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3_ch3','var':'_eventBDT'})
    years = ['2016','2017','2018']
    # define what processes to consider as signal and whether to allow a ratio measurement
    signals = ['tZq']
    ratio = []

    # make a list of channel info dicts for easy looping
    channels = []
    suffix = 'npfromdata' if npfromdata else 'npfromsim'
    for c in cnames:
	region = c['region']
	for year in years:
	    channels.append( {'name':region+'_'+year,
		    'path':os.path.join(topdir,year+'combined',region,suffix,'combined.root'),
		    'var':c['var'] })

    if os.path.exists(datacarddir):
	os.system('rm -r '+datacarddir)
    os.makedirs(datacarddir)

    for channel in channels:
	# get region, year and variable to use
	name = channel['name']
	region = name[:-5]
	year = name[-4:]
	variable = channel['var']
	path = channel['path']

	# check if histogram file exists
        if not os.path.exists(path):
            print('### WARNING ###: '+path+' not found, skipping it.')
            continue
        print('making datacard for '+path+'...')
	
	# get necessary info
        processinfo = dct.readhistfile(path,variable,signals)
	#print(processinfo)

	ut.disable_default( processinfo, year, region, npfromdata )
	normsyslist = ut.addnormsys_default( processinfo, year, region, npfromdata )

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
		    rateparamlist=rateparamlist,ratio=ratio,
		    writeobs=False,autof=True)
