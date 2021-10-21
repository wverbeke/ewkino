#########################################################################################
# compare kinematic distributions between gen-matched and non-gen-matched Ds candidates #
#########################################################################################

import sys
import os
import ROOT
import json
import fillhistograms as fh
from dmeson import DMeson, DMesonCollection
sys.path.append('../tools')
import histtools as ht
import optiontools as opt

def fillhistograms( tree, variables_match, variables_nomatch, 
		    outputfile=None, nprocess=-1 ):
    ### loop over the events in an ntuple and fill the histograms
    # variables is a list of dicts with at least the following keys: 'name', and 'hist'
    # (the histograms in it will be implicitly filled by this function).
    # if outputfile is None, no output file will be written.
  
    if( len(variables_match)==0 or len(variables_nomatch)==0 ):
	print('WARNING: no valid variables found, returning...')
	return

    # initialize counters (for debugging and checking)
    matchcounter = 0
    nomatchcounter = 0
 
    # set number of events to process
    if( nprocess<0 or nprocess>tree.GetEntries() ): nprocess=tree.GetEntries()
    # loop over events
    print('starting event loop...')
    for i in range(nprocess):
        if( i%1000==0 ):
            print('number of processed events: '+str(i))
        tree.GetEntry(i)
        # determine weight for this entry
        weight = 1
        # (maybe extend later)

	# make a collection of Dstar mesons
	dmesons = DMesonCollection( tree, i )
	# loop over dstar mesons
	for dmeson in dmesons:
	    
	    # apply selections, version 20/10/2021 after hyperopt optimalization
	    if( dmeson.firsttrackpt < 5. ): continue
	    if( dmeson.secondtrackpt < 2. ): continue
	    if( dmeson.thirdtrackpt < 1. ): continue
	    if( dmeson.isolation < 0.65 ): continue
	    if( dmeson.intresmass < 1.0 or dmeson.intresmass > 1.04 ): continue
	    if( dmeson.intresmassdiff < 0.9 or dmeson.intresmassdiff > 1.05 ): continue
	    if( dmeson.dr < 0.02 or dmeson.dr > 0.17 ): continue
	    if( dmeson.pt < 12. ): continue

	    # determine if this candidate has a gen match
	    isgenmatched = dmeson.hasfastgenmatch
	    thisvariables = variables_match if isgenmatched else variables_nomatch
	    
	    # increment counters
	    if isgenmatched: matchcounter += 1
	    else: nomatchcounter += 1
	    
	    # loop over variables
	    for el in thisvariables:
		varname = el['name']
		hist = el['hist']
		varvalue = getattr(dmeson,varname)
		if callable(varvalue): varvalue = varvalue()
		# fill the histogram
		ht.fillhistogram( hist, varvalue, weight=weight, clipouterflow=True )

    # printouts
    print('finished event loop')
    print('matchcounter: {}'.format(matchcounter))
    print('nomatchcounter: {}'.format(nomatchcounter))

    # write output file
    if outputfile is not None:
	outf = ROOT.TFile.Open(outputfile,'recreate')
	for el in variables_match: 
	    el['hist'].SetName( el['hist'].GetName()+'_match' )
	    el['hist'].SetTitle('gen matched')
	    el['hist'].Write()
	for el in variables_nomatch: 
            el['hist'].SetName( el['hist'].GetName()+'_nomatch' )
	    el['hist'].SetTitle('not gen matched')
            el['hist'].Write()
	outf.Close()

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('treename', default='blackJackAndHookers/blackJackAndHookersTree') )
    options.append( opt.Option('variables', vtype='path') )
    options.append( opt.Option('nprocess', vtype='int', default=-1) )
    options.append( opt.Option('outputfile', default=None) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    fin = ROOT.TFile.Open(options.inputfile)
    tree = fin.Get(options.treename)
    variables_match = fh.load_variables(options.variables)
    variables_nomatch = fh.load_variables(options.variables)
    fh.initialize_histograms(variables_match)
    fh.initialize_histograms(variables_nomatch)
    fillhistograms( tree, variables_match, variables_nomatch, 
		    outputfile=options.outputfile, nprocess=options.nprocess )
