######################################
# Convert an ntuple to a numpy array #
######################################
# note: for use in train_kerasmodel.py, the last variable in the input list
#       should correspond to the instance label!
#       this will make sure the last column in the array is the label.

import sys
import os
import json
import numpy as np
import ROOT
sys.path.append('../tools')
import optiontools as opt

def tuple_to_ndarray( tree, variables, nprocess=-1 ):
    ### convert a root tree (ntuple format) to a numpy array
    # input arguments:
    # - tree: root tree in ntuple format
    # - variables: list of dicts of form 
    #              {'variable':'<variable name>','size':'<length of variable array>'}
    #              note: use 'size':'1' for event-level variables.
    #              note: size specification '1' for some variables can be used simultaneously 
    #                    with another one for other variables; in that case event-level variables
    #                    will be copied for each object instance;
    #                    however, there can be only one other size specification apart from '1',
    #                    else there will be ambiguities!

    # check size specifications for variables
    haseventvariables = False
    objsize = None
    for var in variables:
	varsize = str(var['size'])
	if( varsize=='1' ): haseventvariables = True
	elif( objsize is None ): objsize = varsize
	elif( varsize!=objsize ):
	    raise Exception('ERROR in tuple_to_ndarray:'
			    +' conflicting size specifications:'
			    +' {} and {}'.format(varsize, objsize))

    # loop over entries
    res = []
    nentries = tree.GetEntries()
    if( nprocess<0 or nprocess>nentries ): nprocess = nentries
    print('looping over {} entries...'.format(nprocess))
    for i in range(nprocess):
	if( i%1000==0 ):
            print('number of processed events: {} of {}'.format(i,nprocess))
	tree.GetEntry(i)
	# determine amount of objects in this entry
	thisobjsize = 1
	if objsize is not None: thisobjsize = getattr(tree, objsize)
	# initialize output array for this entry
	arr = np.zeros((thisobjsize,len(variables)))
	# loop over variables
	for k,var in enumerate(variables):
	    varname = var['variable']
	    varsize = str(var['size'])
	    if varsize=='1': 
		for j in range(thisobjsize): arr[j,k] = getattr(tree,varname)
	    else:
		for j in range(thisobjsize): arr[j,k] = getattr(tree,varname)[j]
	# do nan checks
	# this is a consequence of isolation being nan in case there are no tracks in the cone. 
	# it has been fixed in the nuplizer (15/11/2021) for future iterations.
	if(np.sum(np.isnan(arr))>0): np.nan_to_num(arr, copy=False)
	# do magnitude checks
	# some very large values were observed on 16/11/2021 for a small number of entries.
	# still to fix in the ntuplizer, but probably not trivial 
	# (it seems the track pt for some tracks is quasi inf)
	nlarge = len(np.nonzero(np.abs(arr)>1e300)[0])
	if nlarge>0: continue
	# add the array for this entry to the list
	res.append(arr)
    # merge all arrays
    arr = np.vstack(tuple(res))
    print('created ndarray of shape {}.'.format(arr.shape))
    return arr
	
if __name__=='__main__':

    # read options
    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('treename', default='blackJackAndHookers/blackJackAndHookersTree') )
    options.append( opt.Option('variables', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
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
        print('')

    # parse options
    if not options.inputfile[-5:]=='.root':
        raise Exception('ERROR: input file is supposed to be an ntuple in .root format')
    if not os.path.exists(options.inputfile):
        raise Exception('ERROR: input file {} does not seem to exist...'.format(options.inputfile))
    if not options.variables[-5:]=='.json':
        raise Exception('ERROR: variable file is supposed to be in .json format')
    if not os.path.exists(options.variables):
        raise Exception('ERROR: variable file {} does not seem to exist...'.format(options.variables))

    # read the ntuple
    f = ROOT.TFile.Open( options.inputfile )
    tree = f.Get( options.treename )

    # read the variables
    with open(options.variables, 'r') as v:
	variables = json.load(v)

    # convert to numpy array
    arr = tuple_to_ndarray( tree, variables, nprocess=options.nprocess )
 
    # close the input file
    f.Close()

    # write the output file
    np.save(options.outputfile, arr)
