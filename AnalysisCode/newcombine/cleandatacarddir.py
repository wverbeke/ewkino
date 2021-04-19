####################################################################################
# restore a datacard directory to its original state (i.e. before running combine) #
####################################################################################

import os
import sys
import combinetools as ct

if __name__=='__main__':

    if len(sys.argv)<2:
	print('### ERROR ###: no directory provided as command line argument')
	print('               usage: python cleandatacarddir.py <directory> [rmtxt] [rmroot]')
	sys.exit()

    datacarddir = sys.argv[1]
    if not os.path.exists(datacarddir):
	print('### ERROR ###: directory does not seem to exist...')
	sys.exit()

    rmtxt = True
    rmroot = True

    for el in sys.argv[2:]:
	(key,value) = el.split('=')
	if key=='rmtxt': rmtxt = (val=='True' or val=='true')
	if key=='rmroot': rmroot = (val=='True' or val=='true')

    ct.cleandatacarddir(datacarddir,rmtxt=rmtxt,rmroot=rmroot)
