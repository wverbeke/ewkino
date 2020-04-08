##############################################################################
# A Python wrapper for the fakeRateMeasurement.cc program in the ewkino code #
##############################################################################

import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

# command line arguments (in sequence:)
# - type of leptons to measure fake rate for ('electron' or 'muon')
# - year ('2016', '2017' or '2018')
# default (no command line arguments): all types and years

# if too few command line args, check with the user if default arguments can be used
if(len(sys.argv)!=1 and len(sys.argv)!=3):
    print('### ERROR ###: fakeratemeasurement.py found wrong number of command line arguments.')
    print('Normal usage from the command line:')
    print('python fakeratemeasurement.py < type > < year >')
    print('OR python fakeratemeasurement.py')
    sys.exit()
doall = True
if len(sys.argv)==3: 
    doall = False
    if(not sys.argv[1] in ['muon','electron'] 
	or not sys.argv[2] in ['2016','2017','2018']):
	print('### ERROR ###: wrong type of command line args.')
	print('Usage: python fakeratemeasurement.py < type > < year >')

# set executable directory
fakerate_directory = os.path.abspath('../../fakeRate')

# check if executable exists
if not os.path.exists('../../fakeRate/fakeRateMeasurement'):
    print('### ERROR ###: fakeRateMeasurement executable does not seem to exist.')
    print('Go to ewkino/fakeRate and run make -f makeFakeRateMeasurement.')
    sys.exit()

#make a job script 
script_name = 'fakeratemeasurement.sh'
with open( script_name, 'w') as script:
    initializeJobScript( script )
    script.write('cd '+fakerate_directory+'\n')
    if doall: command = './fakeRateMeasurement'
    else: command = './fakeRateMeasurement {} {}'.format(sys.argv[1],sys.argv[2])
    script.write( command )

# submit job and catch errors 
submitQsubJob( script_name )
# alternative: run locally (for testing and debugging)
#os.system('bash '+script_name)
