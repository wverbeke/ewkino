####################################################
# a python looper for plotMCFakeRateMeasurement.cc #
####################################################

import os
import sys

# read fixed command line arguments
workingdir = sys.argv[1]
year = sys.argv[2]
flavour = sys.argv[3]

# do some printouts for logging
print('starting plotMCFakeRateMeasurement.py...')
print('command line arguments:')
print(' - workingdir {}'.format(workingdir))
print(' - year {}'.format(year))
print(' - flavour {}'.format(flavour))

# check if executable exists
if not os.path.exists('./plotMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCPlotFakeRateMeasurement first.')
    sys.exit()

# hadd files if needed
cwd = os.getcwd()
os.chdir(workingdir)
filename = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
if not os.path.exists(filename):
    cmd = 'hadd '+filename+' '
    cmd += 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms_sample*.root'
    print('will hadd files with following command:')
    print(cmd)
    os.system(cmd)

# run plotting command
print('now plotting the fake rate map...')
filename  = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
if not os.path.exists(filename):
    print('### ERROR ###: file '+filename+' not found, skipping it.')
    sys.exit()
command = '../plotMCFakeRateMeasurement {} {}'.format(flavour,year)
os.system(command)
print('done making fake rate map for {}.'.format(filename))
