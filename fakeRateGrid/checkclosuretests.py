###############################################
# check if all closure tests maps are present #
###############################################

import sys
import os

if __name__=='__main__':

    topdir = sys.argv[1]
    
    # find all folders with conventional name one level below top directory
    print('finding working point folders in {}...'.format(topdir))
    ctdirs = []
    for el in os.listdir(topdir):
	if 'ptRatio' in el and 'deepFlavor' in el: ctdirs.append( os.path.join(topdir,el) )
    print('found {} working point folders'.format(len(ctdirs)))
    
    # check if they contain all required closure test files
    flavors = ['electron','muon']
    years = ['2016','2017','2018']
    processes = ['DY','TT']
    print('checking content...')
    for ctdir in ctdirs:
	ctfiles = os.listdir(ctdir)
	for flavor in flavors:
	    for year in years:
		for process in processes:
		    name = 'closurePlots_MC_{}_{}_{}.root'.format(process, year, flavor)
		    if name not in ctfiles:
			msg = 'WARNING: closure test file {} not in {}'.format(name,ctdir)
			print(msg)
    print('done')
