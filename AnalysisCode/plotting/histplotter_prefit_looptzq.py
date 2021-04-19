##############################################################################
# run histplotter_prefit with specific input/output file/directory structure #
##############################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

# 2 command line arguments: input directory and output directory
if len(sys.argv)!=3:
    raise Exception('ERROR: wrong command line arguments')

inputdir = os.path.abspath(sys.argv[1])
outputdir = os.path.abspath(sys.argv[2])

# define (regions, years, npdatatype)
npfromdata = True
regions = []
regions.append('all')
#regions.append('signalregion_cat1')
years = ['2016','2017','2018','years']
# define what processes to consider as signal
signals = ['tZq'] # remnant from other code, ignored here, hardcoded further downstream

# automatically determine what regions to consider if requested
regionsdict = {}
if( len(regions)==1 and regions[0]=='all' ):
    regionsdict[years[0]] = os.listdir(os.path.join(inputdir,years[0]+'combined'))
    for year in years[1:]:
        regionsdict[year] = os.listdir(os.path.join(inputdir,year+'combined'))
else:
    for year in years: regionsdict[year] = regions[:]

# make a list of channel info dicts for easy looping
channels = []
suffix = 'npfromdata' if npfromdata else 'npfromsim'
for year in years:
    for region in regionsdict[year]:
        channels.append( {'name':region+'_'+year,
            'region':region,
            'year':year,
            'npfromdata':npfromdata,
            'path':os.path.join(inputdir,year+'combined',region,suffix,'combined.root')})

# define the commands
commands = []
for c in channels:
    print('running on channel '+c['name'])
    # define input file
    thisinputfile = c['path']
    # defiine output file
    yearsuffix = c['year']
    if c['year']=='years': yearsuffix = 'yearscombined'
    thisoutputdir = os.path.join(outputdir,c['region']+'_'+yearsuffix)
    if not os.path.exists(thisoutputdir):
	os.makedirs(thisoutputdir)
    thisoutputfile = os.path.join(thisoutputdir,c['region']+'_'+yearsuffix)
    cmd = 'python histplotter_prefit.py {} {}'.format(thisinputfile,thisoutputfile)
    commands.append(cmd)

# run the commands
# either locally:
#for cmd in commands:
#    os.system(cmd)
# or with condor:
ct.submitCommandsAsCondorCluster('cjob_histplotter_prefit',commands)

