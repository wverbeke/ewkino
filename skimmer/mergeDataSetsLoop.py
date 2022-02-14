########################################################
# run the mergeDataSets.py command for number of files #
########################################################

import os
import sys

inputs = []
outputs = []

topdir = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL_test'

# 2017 combined
outputs.append(os.path.join(topdir,'2017_data','data_combined_Fall17.root'))
inputs.append(os.path.join(topdir,'2017_data','*_Fall17_Run2017.root'))

# 2018 combined
outputs.append(os.path.join(topdir,'2018_data','data_combined_Autumn18.root'))
inputs.append(os.path.join(topdir,'2018_data','*_Autumn18_Run2018.root'))

for ip,op in zip(inputs,outputs):
    cmd = 'python mergeDataSets.py {} {}'.format(op,ip)
    os.system(cmd)
