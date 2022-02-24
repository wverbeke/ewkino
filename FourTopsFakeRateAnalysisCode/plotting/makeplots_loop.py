########################################################################################
# Simple submitter that runs makeplots.py for a number of predefined regions and years #
########################################################################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct

inputdir = sys.argv[1]
outputdir = sys.argv[2]
runmode = 'condor'

regions = []
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)

years = ['2016PreVFP','2016PostVFP','2017','2018']

npmodes = ['npfromsim','npfromdata']

cmds = []
for year in years:
    for region in regions:
	for npmode in npmodes:
	    subdir = os.path.join(year+'_'+region, 'merged_'+npmode)
	    inputfile = os.path.join(inputdir, subdir, 'merged.root')
	    if not os.path.exists(inputfile):
		print('WARNING: input file {} does not exist; continuing...'.format(inputfile))
		continue
	    thisoutputdir = os.path.join(outputdir, year+'_'+region+'_'+npmode)
	    cmd = 'python makeplots.py '+inputfile+' '+year+' '+thisoutputdir
	    if runmode=='local':
		print('executing '+cmd)
		os.system(cmd)
	    elif runmode=='condor':
		print('submitting '+cmd)
		cmds.append(cmd)
	    else:
		raise Exception('ERROR: runmode "{}" not recognized')

if runmode=='condor':
    ct.submitCommandsAsCondorCluster('cjob_makeplots', cmds)
