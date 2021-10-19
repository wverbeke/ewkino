################################################################
# simple looper to make datacards for top and antitop channels #
################################################################

import sys 
import os
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

if __name__=="__main__":

    if len(sys.argv)!=3:
        print('### ERROR ###: need different number of command line args:')
        print('               <input directory> and <datacard directory>')
        sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]
    
    channels = ['_top','_antitop'] 

    commands = []
    for channel in channels:
	thisdatacarddir = datacarddir+channel
	commands.append('python makedatacards.py {} {} topcharge={}'.format(
			topdir,thisdatacarddir,channel))
	# (note: channel='' is equivalent to omitting topcharge argument)
    
    # run local:
    #for command in commands: os.system(command)
    # run in jobs:
    ct.submitCommandsAsCondorCluster('cjob_makedatacards',commands)
