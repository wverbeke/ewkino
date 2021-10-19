##########################################################
# simple looper to run combine for top, antitop channels #
##########################################################

import sys
import os

if __name__=='__main__':

    # parse command line args
    if len(sys.argv)<2:
	print('### ERROR ###: need at least one command line arg (input folder)')
	sys.exit()
    datacarddir = os.path.abspath(sys.argv[1])

    channels = ['_top','_antitop']

    for channel in channels:
        thisdatacarddir = datacarddir+channel
        os.system('python runcombine.py {}'.format(thisdatacarddir))
