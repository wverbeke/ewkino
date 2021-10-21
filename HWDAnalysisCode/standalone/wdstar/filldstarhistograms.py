##################################################
# loop over ntuples and fill a set of histograms #
##################################################

import sys
import os
import ROOT
import json
import fillhistograms as fh
from dmeson import DMeson, DMesonCollection
from muon import Muon, MuonCollection
import muonselector
sys.path.append('../tools')
import optiontools as opt

def fillhistograms( tree, variables, outputfile=None, nprocess=-1 ):
    ### loop over the events in an ntuple and fill the histograms
    # variables is a list of dicts with at least the following keys: 'name', and 'hist'
    # (the histograms in it will be implicitly filled by this function).
    # if outputfile is None, no output file will be written.
  
    # filter variables to keep only valid variables
    # (does not work yet, need to fix)
    #newvariables = []
    #for var in variables:
    #    if var['name'] not in dir(DMeson):
    #	    print('WARNING: skipping variable {}'.format(var['name'])
    #		    +' as it is not a valid DMeson attribute')
    #	    continue
    #	else: newvariables.append(var) 
    #variables = newvariables
    if len(variables)==0:
	print('WARNING: no valid variables found, returning...')
	return
 
    # set number of events to process
    if( nprocess<0 or nprocess>tree.GetEntries() ): nprocess=tree.GetEntries()
    # loop over events
    for i in range(nprocess):
        if( i%10000==0 ):
            print('number of processed events: '+str(i))
        tree.GetEntry(i)
        # determine weight for this entry
        weight = 1
        # (maybe extend later)

	# make a collection of muons
	#muons = MuonCollection( tree, i )

	# do selections
	#mask = muonselector.select( muons, 'default' )
	#if sum(mask)!=1: continue
	#muons = [muons[i] for i in range(len(mask)) if mask[i]]
	#muon = muons[0]
	
	# make a collection of Dstar mesons
	dmesons = DMesonCollection( tree, i )
	# loop over dstar mesons
	for dmeson in dmesons:
	    
	    # apply selections
	    #if abs(dmeson.intresmass-1.865)>0.035: continue
	    #if abs(dmeson.thirdtrackz-muon.innertrackz)>0.2: continue
	    #if abs(dmeson.massdiff-0.1454)>0.001: continue
	    
	    # loop over variables
	    for el in variables:
		varname = el['name']
		hist = el['hist']
		varvalue = getattr(dmeson,varname)
		# fill the histogram
		hist.Fill(varvalue,weight)

    # write output file
    if outputfile is not None:
	outf = ROOT.TFile.Open(outputfile,'recreate')
	for el in variables: el['hist'].Write()
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
    variables = fh.load_variables(options.variables)
    fh.initialize_histograms(variables)
    fillhistograms( tree, variables, outputfile=options.outputfile, nprocess=options.nprocess )
