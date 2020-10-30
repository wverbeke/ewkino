####################################################
# a python looper for plotMCFakeRateMeasurement.cc #
####################################################
import os
import sys

# set global properties
years = ['2016']
flavours = ['muon','electron']

# check if executable exists
if not os.path.exists('./plotMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCPlotFakeRateMeasurement first.')
    sys.exit()

# hadd files if needed
# note that the subfiles are assumed to be in a folder named MCFakeRateMeasurementSubFiles
# you may have to manually create that folder and move the subfiles in there
for year in years:
    for flavour in flavours:
	filename = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
	if os.path.exists(filename): continue
	subfolder = 'MCFakeRateMeasurementSubFiles'
	cmd = 'hadd '+filename+' '+subfolder
	cmd += '/fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms_sample*.root'
	#print(cmd)
	os.system(cmd)

# loop and run command
cwd = os.getcwd()
for year in years:
    for flavour in flavours:
	# check file
	filename  = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
	if not os.path.exists(filename):
	    print('### ERROR ###: file '+filename+' not found, skipping it.')
	    continue
        command = './plotMCFakeRateMeasurement {} {}'.format(flavour,year)
        os.system(command)

