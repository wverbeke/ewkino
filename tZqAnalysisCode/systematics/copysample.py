###################################################################
# script to copy a sample from one directory structure to another #
###################################################################
# e.g. when reprocessing a single sample for all years and regions

import sys
import os
import re

sourcedir = ''
targetdir = ''

if len(sys.argv)==3:
    sourcedir = os.path.abspath(sys.argv[1])
    targetdir = os.path.abspath(sys.argv[2])
else:
    raise Exception('ERROR: wrong number of command line args')  

samplelist = ([
		'tZq.+'
	     ])
yearstypes = ['all'] # e.g. '2016MC' or '2018data' or 'all'
regions = ['all'] # e.g. 'signalregion_cat1_3prompt' or 'all'
commands = []

dynamicyt = False
if( len(yearstypes)==1 and yearstypes[0]=='all'): dynamicyt = True
dynamicregions = False
if( len(regions)==1 and regions[0]=='all'): dynamicregions = True

if dynamicyt: yearstypes = ([ f for f in os.listdir(sourcedir) 
				if (re.match('201.MC',f) or re.match('201.data',f)) ])
for yeartype in yearstypes:
    if dynamicregions: thisregions = [f for f in os.listdir(os.path.join(sourcedir,yeartype))]
    else: thisregions = regions[:]
    for region in thisregions:
	thissourcedir = os.path.join(sourcedir,yeartype,region)
	thistargetdir = os.path.join(targetdir,yeartype,region)
	samples = [f for f in os.listdir(thissourcedir)]
	samplestocopy = []
	for s in samples:
	    for s2 in samplelist:
		if re.match(s2,s): samplestocopy.append(s)
	if len(samplestocopy)==0: continue
	if not os.path.exists(thistargetdir):
	    raise Exception('ERROR: requested to copy samples from {} to {}'.format(
				thissourcedir,thistargetdir)+' but target dir does not exist')
	for s in samplestocopy: commands.append('cp {} {}'.format(os.path.join(thissourcedir,s),
						    thistargetdir+'/'))

print('will execute following copy commands:')
for c in commands: print('  '+c)
print('total: {} commands'.format(len(commands)))
print('continue? (y/n)')
go = raw_input()
if not go=='y': sys.exit()

for c in commands: os.system(c)
