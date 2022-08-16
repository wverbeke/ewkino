###########################################
# check if all fake rate maps are present #
###########################################

import sys
import os

if __name__=='__main__':

    topdir = sys.argv[1]
    
    # find all folders named 'fakeRateMaps' in given top directory
    print('finding fake rate map folders in {}...'.format(topdir))
    frdirs = []
    for root, dirs, files in os.walk(topdir):
	for d in dirs:
	    if d=='fakeRateMaps': frdirs.append( os.path.join(root,d) )
    print('found {} fake rate map folders'.format(len(frdirs)))
    
    # check if they contain all required fake rate maps
    flavors = ['electron','muon']
    years = ['2016PreVFP','2016PostVFP','2017','2018']
    dtypes = ['MC']
    print('checking content...')
    for frdir in frdirs:
	frfiles = os.listdir(frdir)
	for flavor in flavors:
	    for year in years:
		for dtype in dtypes:
		    name = 'fakeRateMap_{}_{}_{}.root'.format(dtype, flavor, year)
		    if name not in frfiles:
			msg = 'WARNING: fake rate map {} not in {}'.format(name,frdir)
			print(msg)
    print('done')
