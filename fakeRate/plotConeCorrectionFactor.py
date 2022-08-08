###################################################
# a python looper for plotConeCorrectionFactor.cc #
###################################################
import os
import sys

flavours = ['electron','muon']
# (see fillConeCorrectionFactor.cc)
leptonMVAs = ['leptonMVATOPUL']
# (see fillConeCorrectionFactor.cc)
mvathresholds = [0.64,0.64]
# (see fillConeCorrectionFactor.cc)
years = ['2016PreVFP','2016PostVFP','2017','2018','all'] 
# (put 'all' here for combining all years)

# check if executable exists
exe = './plotConeCorrectionFactor'
if not os.path.exists(exe):
    raise Exception('ERROR: executable {} does not exist.'.format(exe))

# hadd files if needed
for flavour in flavours:
    for year in years:
	for leptonMVA in leptonMVAs:
	    filename = 'coneCorrectionFactor_'+leptonMVA+'_'
	    filename += flavour+'_'+year+'_histograms.root'
	    if os.path.exists(filename): continue
	    haddyears = year
	    if year=='all': haddyears = '*'
	    haddsource = 'coneCorrectionFactor_'+leptonMVA+'_'
	    haddsource += flavour+'_'+haddyears+'_histograms_sample*.root'
	    cmd = 'hadd {} {}'.format(filename, haddsource)
	    os.system(cmd)

cwd = os.getcwd()
for flavour,mvathreshold in zip(flavours,mvathresholds):
    for year in years:
	for leptonMVA in leptonMVAs:
	    # check file
	    filename = 'coneCorrectionFactor_'+leptonMVA+'_'
	    filename += flavour+'_'+year+'_histograms.root'
	    if not os.path.exists(filename):
		print('ERROR: file '+filename+' not found, skipping it.')
		continue
	    command = './plotConeCorrectionFactor {} {} {} {}'.format(
			flavour, year, leptonMVA, mvathreshold)
	    os.system(command)
