##################################################
# loop over ntuples and fill a set of histograms #
##################################################

import sys
import os
import ROOT
import json
sys.path.append('../tools')
import optiontools as opt

def load_variables( jsonfile ):
    ### read a collection of histograms settings (i.e. variable name + other settings)
    with open(jsonfile, 'r') as f:
	variables = json.load(f)
    return variables

def initialize_histograms( variables ):
    ### extend the object returned by read_variables with a TH1D for each variable
    for el in variables:
	varname = el['name']	
	xlow = el['xlow']
	xhigh = el['xhigh']
	nbins = el['nbins']
	title = el['title']
	hist = ROOT.TH1D( varname, title, nbins, xlow, xhigh )
	hist.SetDirectory(0)
	el['hist'] = hist

def fillhistograms( tree, variables, outputfile=None, nprocess=-1 ):
    ### loop over the events in an ntuple and fill the histograms
    # variables is a list of dicts with at least the following keys: 'name', 'size' and 'hist'
    # (the histograms in it will be implicitly filled by this function).
    # if outputfile is None, no output file will be written.
   
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
	# loop over variables
	for el in variables:
	    varname = el['name']
	    varsize = el['size']
	    hist = el['hist']
	    # determine values for requested variable in this entry
	    varvalues = []
	    # case 1: branch is scalar
	    if varsize=='': varvalues = [getattr(tree,varname)]
	    # case 2: branch is vector
	    else:
		nvalues = getattr(tree,varsize)
		varvalues = [getattr(tree,varname)[j] for j in list(range(nvalues))]
	    # fill the histogram
	    for var in varvalues: hist.Fill(var,weight)
    # write output file
    if outputfile is not None:
	outf = ROOT.TFile.Open(outputfile,'recreate')
	for el in variables: el['hist'].Write()
	outf.Close()

if __name__=='__main__':

    sys.stderr.write('### starting ###\n')

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
    variables = load_variables(options.variables)
    initialize_histograms(variables)
    fillhistograms( tree, variables, outputfile=options.outputfile, nprocess=options.nprocess )

    sys.stderr.write('### done ###\n')
