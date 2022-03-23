#########################################################################################
# Simple submitter that runs mergehists.py for a number of predefined regions and years #
#########################################################################################
# note: corresponds to new convention with one file per sample
#       (inclusive in event selection and selection type)

import sys
import os

topdir = sys.argv[1]

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = ['npfromsim','npfromdata']

for year in years:
    for npmode in npmodes:
        inputdir = os.path.join(topdir, year)
        outputfile = os.path.join(topdir, year, 'merged_{}'.format(npmode), 'merged.root')
        cmd = 'python mergehists.py '+inputdir+' '+outputfile+' '+npmode
        print('executing '+cmd)
        os.system(cmd)
