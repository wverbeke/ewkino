##########################################
# a python looper for plotMagicFactor.cc #
##########################################
import os
import sys

flavours = ['electron','muon']
mvathresholds = [0.9,0.9]
years = ['2016','2017','2018','all'] # put 'all' here for combining all years

# check if executable exists
if not os.path.exists('./plotMagicFactor'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makePlotMagicFactor first.')
    sys.exit()

# hadd files if needed
for flavour in flavours:
    for year in years:
	filename = 'magicFactor_'+flavour+'_'+year+'_histograms.root'
	if os.path.exists(filename): continue
	subfolder = 'MagicFactorSubFiles'
	cmd = 'hadd '+filename+' '+subfolder
	haddyears = year
	if year=='all': haddyears = '201?'
	cmd += '/magicFactor_'+flavour+'_'+haddyears+'_histograms_sample*.root'
	#print(cmd)
	os.system(cmd)

cwd = os.getcwd()
for flavour,mvathreshold in zip(flavours,mvathresholds):
    for year in years:
	# check file
	filename  = 'magicFactor_'+flavour+'_'+year+'_histograms.root'
	if not os.path.exists(filename):
	    print('### ERROR ###: file '+filename+' not found, skipping it.')
	    continue
	command = './plotMagicFactor {} {} {}'.format(flavour, year, mvathreshold)
	os.system(command)

