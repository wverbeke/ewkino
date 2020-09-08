##################################################
# a python looper for plotFakeRateMeasurement.cc #
##################################################
import os
import sys

years = ['2016','2017','2018']
flavours = ['muon','electron']
use_mT = True

# check if executable exists
if not os.path.exists('./plotFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makePlotFakeRateMeasurement first.')
    sys.exit()

# hadd files if needed
for year in years:
    for flavour in flavours:
	basename = 'fakeRateMeasurement_data_'+flavour+'_'+year
	if use_mT: basename += '_mT'
	else: basename += '_met'
	basename += '_histograms'
	filename = basename + '.root'
	if os.path.exists(filename): continue
	subfolder = 'FakeRateMeasurementSubFiles'
	cmd = 'hadd '+filename+' '
	cmd += os.path.join(subfolder,basename+'_sample*.root')
	print(cmd)
	os.system(cmd)

cwd = os.getcwd()
for year in years:
    for flavour in flavours:
	# check file
	basename = 'fakeRateMeasurement_data_'+flavour+'_'+year
        if use_mT: basename += '_mT'
        else: basename += '_met'
        basename += '_histograms'
        filename = basename + '.root'
	if not os.path.exists(filename):
	    print('### ERROR ###: file '+filename+' not found, skipping it.')
	    continue
	command = './plotFakeRateMeasurement {} {} {}'.format(str(use_mT),flavour,year)
	os.system(command)

