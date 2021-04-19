################################################################
# simple looper to run combine for eee, eem, emm, mmm channels #
################################################################

import sys
import os

if __name__=='__main__':

    # parse command line args
    if len(sys.argv)<2:
	print('### ERROR ###: need at least one command line arg (input folder)')
	sys.exit()
    datacarddir = os.path.abspath(sys.argv[1])

    #channels = ['','_ch0','_ch1','_ch2','_ch3']
    channels = ['_ch0','_ch1','_ch2','_ch3']
    # following naming convention in runsystematics, i.e. count number of muons

    for channel in channels:
        thisdatacarddir = datacarddir+channel
        os.system('python runcombine.py {}'.format(thisdatacarddir))
