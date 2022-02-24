#########################################################################################
# Simple submitter that runs mergefiles.py for a number of predefined regions and years #
#########################################################################################
# note: corresponds to old convention with one file per sample
#       and per event selection and per selection type!

import sys
import os

topdir = sys.argv[1]

regions = []
for r in ['wzcontrolregion']: regions.append(r)
#for r in ['zzcontrolregion','zgcontrolregion']: regions.append(r)

years = ['2017']

npmodes = ['npfromdata']

for year in years:
        for region in regions:
	    for npmode in npmodes:
                inputdir = os.path.join(topdir, year+'_'+region)
                outputfile = os.path.join(topdir, year+'_'+region, 
				'merged_{}'.format(npmode), 'merged.root')
                cmd = 'python mergefiles.py '+inputdir+' '+outputfile+' '+npmode
                print('executing '+cmd)
                os.system(cmd)
