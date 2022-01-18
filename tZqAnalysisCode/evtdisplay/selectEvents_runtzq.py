###############################################################
# run selectEvents.py for default tZq datasets stored on pnfs #
###############################################################

import os
import sys

years = ['2016', '2017', '2018']
basepath = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_tzq_final'
bdtcut = 0.9

cmds = []
for year in years:
    dpath = os.path.join(basepath,year+'data')
    dfiles = [f for f in os.listdir(dpath)]
    for f in dfiles:
	inputfile = os.path.join(dpath,f)
	bdtcutstr = str(bdtcut).replace('.','p')
	outputfile = 'evtids_{}_{}_bdtcut{}.txt'.format(year,f.split('_')[0],bdtcutstr)
	cmd = 'python selectEvents.py {} {} {} {}'.format(inputfile, outputfile, bdtcut, 0)
	cmds.append(cmd)

for cmd in cmds:
    print('running {}'.format(cmd))
    os.system(cmd)
