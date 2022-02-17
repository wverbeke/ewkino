####################################################################################
# Simple submitter that runs merge.py for a number of predefined regions and years #
####################################################################################

import sys
import os

topdir = sys.argv[1]

regions = []
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = ['npfromdata']

for year in years:
        for region in regions:
	    for npmode in npmodes:
                inputdir = os.path.join(topdir, year+'_'+region)
                outputfile = os.path.join(topdir, year+'_'+region, 
				'merged_{}'.format(npmode), 'merged.root')
                cmd = 'python merge.py '+inputdir+' '+outputfile+' '+npmode
                print('executing '+cmd)
                os.system(cmd)
