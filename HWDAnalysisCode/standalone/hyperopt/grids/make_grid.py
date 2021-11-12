############################################
# script for making a hyperopt search grid #
############################################

import sys
import os
import json
import pickle as pkl
from hyperopt import hp
sys.path.append('../../tools')
import optiontools as opt

def read_grid_configuration( jsonfile ):
    ### read grid configuration from a json file
    # input arguments:
    # - jsonfile: path to a json file containing a valid grid definition.
    #             the json object should be a list of dicts,
    #             each dict should have the following items:
    #             - variable: name of the ntuple variable to cut on
    #             - cuttype: either min or max
    #		  - hptype: name of a hyperopt range function, e.g. quniform
    #             - minvalue, maxvalue, stepsize
    # output type:
    # dict containing the grid information
    # keys: strings formed as variable + '_' + cuttype
    # values: lists of the form [hyperopt range function, arguments to hyperopt range function]
    with open(jsonfile, 'r') as f:
	jsonobj = json.load(f)
    config = {}
    for el in jsonobj:
	key = '{}_{}'.format(el['variable'],el['cuttype'])
	value = [getattr(hp,el['hptype']), key, el['minvalue'], el['maxvalue'], el['stepsize']]
	config[key] = value
    return config

def make_grid( config ):
    ### make a hyperopt search grid based on a given configuration
    # see e.g. grid_configuration()
    grid = {}
    for key,val in config.items():
	grid[key] = val[0](*val[1:])
    return grid

def make_str( config ):
    ### make an human readable string based on a given configuration
    res = ''
    for key,val in config.items():
	res += '{}: {}('.format(key,val[0])
	for arg in val[1:-1]: res+='{}, '.format(arg)
	res += '{})\n'.format(val[-1])
    res = res.strip('\n')
    return res

if __name__=='__main__':
    
    # read options
    options = []
    options.append( opt.Option('inputjson', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
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
    if not options.inputjson[-5:]=='.json':
	raise Exception('ERROR: input file is supposed to be in .json format')
    if not os.path.exists(options.inputjson):
	raise Exception('ERROR: input file {} does not seem to exist...'.format(options.inputjson))

    # get the current grid configuration
    config = read_grid_configuration( options.inputjson )

    # make the actual hyperopt grid
    grid = make_grid(config)
    
    # make a human-readable version of the grid
    gridstr = make_str(config)
    print('found follwing grid:')
    print(gridstr)

    # pack both in a dict and writ to a pkl file
    out = {'grid':grid, 'description':gridstr}
    outputpkl = os.path.splitext(options.outputfile)[0]+'.pkl'
    with open(outputpkl,'w') as f:
	pkl.dump(out, f)
    print('grid written to {}.'.format(outputpkl))
