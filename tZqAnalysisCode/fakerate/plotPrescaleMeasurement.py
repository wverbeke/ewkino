####################################################
# a python looper for plotMCPrescaleMeasurement.cc #
####################################################
import os
import sys

years = ['2016','2017','2018']
use_mT = True

# check if executable exists
if not os.path.exists('./plotPrescaleMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makePlotPrescaleMeasurement first.')
    sys.exit()

# hadd files if needed
for year in years:
    basename = 'prescaleMeasurement'
    if use_mT: basename += '_mT'
    else: basename += '_met'
    basename += '_histograms_'+year
    filename = basename + '.root'
    if os.path.exists(filename): continue
    subfolder = 'PrescaleMeasurementSubFiles'
    cmd = 'hadd '+filename+' '
    cmd += os.path.join(subfolder,basename+'_sample*.root')
    print(cmd)
    os.system(cmd)

cwd = os.getcwd()
for year in years:
    # check file
    basename = 'prescaleMeasurement'
    if use_mT: basename += '_mT'
    else: basename += '_met'
    basename += '_histograms_'+year
    filename = basename + '.root'
    if not os.path.exists(filename):
        print('### ERROR ###: file '+filename+' not found, skipping it.')
        continue
    command = './plotPrescaleMeasurement {} {}'.format(str(use_mT),year)
    os.system(command)

