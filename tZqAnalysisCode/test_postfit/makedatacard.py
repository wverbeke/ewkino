####################################################################
# make a datacard for the test file generated with maketestfile.py #
####################################################################

import os
import sys
sys.path.append('../newcombine')
import datacardtools as dct
import uncertaintytools as ut

if __name__=="__main__":

    if len(sys.argv)<3:
	print('### ERROR ###: need different number of command line args:')
	print('               <input directory> and <datacard directory>')
	sys.exit()
    infile = sys.argv[1] # input root file containing histograms
    name = sys.argv[2] # name tag of the datacard

    # define what processes to consider as signal and whether to allow a ratio measurement
    signals = ['tZq']

    # get region, year and variable to use
    datacarddir = 'datacards'
    if not os.path.exists(datacarddir): os.makedirs(datacarddir)
    region = 'signalregion_cat2'
    year = '2017'
    npfromdata = True
    variable = '_eventBDT'

    print('making datacard for '+infile+'...')
	
    # get necessary info
    processinfo = dct.readhistfile(infile,variable,signals)

    ut.disable_default( processinfo, year, region, npfromdata )

    normsyslist = ut.addnormsys_default( processinfo, year, region, npfromdata )
    #normsyslist = []

    #normsyslist = []
    #impacts = {}
    #for p in processinfo.plist: impacts[p]='1.2'
    #processinfo.addnormsys( 'norm', impacts )
    #normsyslist.append('norm')

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
    dct.writedatacard(datacarddir,name,processinfo,infile,variable,
                    shapesyslist=shapesyslist,lnNsyslist=normsyslist,
		    rateparamlist=[],ratio=[],
		    writeobs=False,autof=True)

