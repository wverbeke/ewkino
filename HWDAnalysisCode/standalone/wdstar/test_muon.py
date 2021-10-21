############################################
# small test script for using muon objects #
############################################

import sys
import os
import ROOT
from muon import Muon, MuonCollection
import muonselector
sys.path.append('../tools')
import optiontools as opt

def runtest( tree, nprocess=-1 ):
  
    # set number of events to process
    if( nprocess<0 or nprocess>tree.GetEntries() ): nprocess=tree.GetEntries()
    # loop over events
    for i in range(nprocess):
        if( i%10000==0 ):
            print('number of processed events: '+str(i))
        tree.GetEntry(i)
    
	# make a collection of muons
	muons = MuonCollection( tree, i )
	print(muons)

	# do selection
	mask = muonselector.select( muons, 'default' )
	print(mask)
	print(sum(mask))

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('treename', default='blackJackAndHookers/blackJackAndHookersTree') )
    options.append( opt.Option('nprocess', vtype='int', default=-1) )
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
    runtest( tree, nprocess=options.nprocess )
