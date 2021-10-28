#############################################
# loop over a flat tree and fill histograms #
#############################################

import sys
import os
import ROOT
import json
sys.path.append('../tools/python')
import optiontools as opt

def load_variables( jsonfile ):
    ### read a collection of histograms settings (i.e. variable name + other settings)
    # input arguments:
    # - jsonfile: path to a json file.
    # return type: same as encoded in json file,
    #              usually a list of dicts of histogram info.
    with open(jsonfile, 'r') as f:
        variables = json.load(f)
    return variables

def initialize_histograms( variables ):
    ### extend a list of variables with a TH1D for each variable
    # input arguments:
    # - variables: a list of dicts with at least the following keys:
    #              'name', 'xlow', 'xhigh', 'nbins', 'title'.
    #              (usually the object returned by load_variables)
    for el in variables:
        varname = el['name']
        xlow = el['xlow']
        xhigh = el['xhigh']
        nbins = el['nbins']
        title = el['title']
        hist = ROOT.TH1D( varname, title, nbins, xlow, xhigh )
        hist.SetDirectory(0)
        el['hist'] = hist

def fillhistograms( tree, variables, nprocess=-1 ):
    ### loop over the events in a flat tree and fill the histograms
    # input arguments:
    # - tree: a flat ROOT tree.
    # - variables: a list of dicts with at least the following keys: 'name' and 'hist'.
    #		   note: 'name' must correspond to the name of a variable in the tree.
    #		   note: an optional key 'weight' is used to get the weight 
    #			 for this event from the tree (default: 1 for all events).
    #		   note: the histograms in it will be implicitly filled by this function.
    # - nprocess: number of events to process.

    # set number of events to process
    if( nprocess<0 or nprocess>tree.GetEntries() ): nprocess=tree.GetEntries()
    # loop over events
    for i in range(nprocess):
        if( i%10000==0 ):
            print('number of processed events: '+str(i))
        tree.GetEntry(i)
        # loop over variables
        for el in variables:
	    # determine value for requested variable in this entry
            varname = el['name']
            varvalue = getattr(tree, varname)
	    # determine weight
	    weight = 1
	    if 'weight' in el.keys(): weight = getattr(tree, el['weight'])
            # fill the histogram
	    hist = el['hist']
            hist.Fill(varvalue, weight)


if __name__=='__main__':

    sys.stderr.write('### starting ###\n')

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('treename', default='HWDAnalysis/HWDTree') )
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

    # open root file and get the tree
    fin = ROOT.TFile.Open(options.inputfile)
    tree = fin.Get(options.treename)
    
    # prepare the histograms
    variables = load_variables(options.variables)
    initialize_histograms(variables)
    
    # fill the histograms
    fillhistograms( tree, variables, nprocess=options.nprocess )
    fin.Close()

    # write the output file
    if options.outputfile is not None:
        outf = ROOT.TFile.Open(options.outputfile,'recreate')
        for el in variables: el['hist'].Write()
        outf.Close()

    sys.stderr.write('### done ###\n')
