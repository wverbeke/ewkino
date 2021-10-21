############################################
# script for making a hyperopt search grid #
############################################

import sys
import os
import json
import pickle as pkl
from hyperopt import hp

### define constants ###
phimass = 1.019

def grid_configuration():
    ### define the current grid configuration
    # should return a dict matching names to lists;
    # the first element in each list is a hp object,
    # the next elements in each list are initiazer arguments
    grid = {}
    grid['_DIntResMass_min'] = [hp.quniform, '_DIntResMass_min', phimass-0.07, phimass, 0.01]
    grid['_DIntResMass_max'] = [hp.quniform, '_DIntResMass_max', phimass, phimass+0.07, 0.01]
    grid['_DIntResMassDiff_min'] = [hp.quniform, '_DIntResMassDiff_min', 0.5, 1., 0.05]
    grid['_DIntResMassDiff_max'] = [hp.quniform, '_DIntResMassDiff_max', 0.8, 1.3, 0.05]
    grid['_DFirstTrackPt_min'] = [hp.quniform, '_DFirstTrackPt_min', 0., 10., 1.]
    grid['_DSecondTrackPt_min'] = [hp.quniform, '_DSecondTrackPt_min', 0., 10., 1.]
    grid['_DThirdTrackPt_min'] = [hp.quniform, '_DThirdTrackPt_min', 0., 20., 1.]
    grid['_DPt_min'] = [hp.quniform, '_DPt_min', 0., 40., 2.]
    grid['_DDR_min'] = [hp.quniform, '_DDR_min', 0., 0.05, 0.01]
    grid['_DDR_max'] = [hp.quniform, '_DDR_max', 0.1, 0.2, 0.01]
    grid['_DIsolation_min'] = [hp.quniform, '_DIsolation_min', 0., 1., 0.05]
    return grid

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

    # set options
    outputfile = 'grid'

    # get the current grid configuration
    config = grid_configuration()

    # make the actual hyperopt grid
    grid = make_grid(config)
    
    # make a human-readable version of the grid
    gridstr = make_str(config)
    print('found follwing grid:')
    print(gridstr)

    # pack both in a dict and writ to a pkl file
    out = {'grid':grid, 'description':gridstr}
    outputpkl = os.path.splitext(outputfile)[0]+'.pkl'
    with open(outputpkl,'w') as f:
	pkl.dump(out, f)
    print('grid written to {}.'.format(outputpkl))
