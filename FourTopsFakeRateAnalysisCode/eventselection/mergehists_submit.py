#########################################################################################
# Simple submitter that runs mergefiles.py for a number of predefined regions and years #
#########################################################################################
# note: corresponds to old convention with one file per sample
#       and per event selection and per selection type!

import sys
import os

topdir = sys.argv[1]

years = ['2017']

npmodes = ['npfromsim','npfromdata']

for year in years:
    for npmode in npmodes:
        inputdir = os.path.join(topdir, year)
        outputfile = os.path.join(topdir, year, 'merged_{}'.format(npmode), 'merged.root')
        cmd = 'python mergefiles.py '+inputdir+' '+outputfile+' '+npmode
        print('executing '+cmd)
        os.system(cmd)
