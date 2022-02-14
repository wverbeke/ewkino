####################################################
# run the mergeHadd.py command for number of files #
####################################################

import os
import sys

# settings
years = ['2017','2018']
topdir = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL_test'

# initializations
inputs = []
outputs = []

### 2017 data
# SingleMuon
outputs.append(os.path.join(topdir,'2017_data','SingleMuon_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','SingleMuon_Fall17_Run2017?.root'))
# SingleElectron
outputs.append(os.path.join(topdir,'2017_data','SingleElectron_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','SingleElectron_Fall17_Run2017?.root'))
# DoubleMuon
outputs.append(os.path.join(topdir,'2017_data','DoubleMuon_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','DoubleMuon_Fall17_Run2017?.root'))
# DoubleEG
outputs.append(os.path.join(topdir,'2017_data','DoubleEG_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','DoubleEG_Fall17_Run2017?.root'))
# MuonEG
outputs.append(os.path.join(topdir,'2017_data','MuonEG_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','MuonEG_Fall17_Run2017?.root'))
# JetHT
outputs.append(os.path.join(topdir,'2017_data','JetHT_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','JetHT_Fall17_Run2017?.root'))
# MET
outputs.append(os.path.join(topdir,'2017_data','MET_Fall17_Run2017.root'))
inputs.append(os.path.join(topdir,'2017_data','MET_Fall17_Run2017?.root'))

### 2018 data
# SingleMuon
outputs.append(os.path.join(topdir,'2018_data','SingleMuon_Autumn18_Run2018.root'))
inputs.append(os.path.join(topdir,'2018_data','SingleMuon_Autumn18_Run2018?.root'))
# EGamma
outputs.append(os.path.join(topdir,'2018_data','EGamma_Autumn18_Run2018.root'))
inputs.append(os.path.join(topdir,'2018_data','EGamma_Autumn18_Run2018?.root'))
# DoubleMuon
outputs.append(os.path.join(topdir,'2018_data','DoubleMuon_Autumn18_Run2018.root'))
inputs.append(os.path.join(topdir,'2018_data','DoubleMuon_Autumn18_Run2018?.root'))
# MuonEG
outputs.append(os.path.join(topdir,'2018_data','MuonEG_Autumn18_Run2018.root'))
inputs.append(os.path.join(topdir,'2018_data','MuonEG_Autumn18_Run2018?.root'))
# JetHT
outputs.append(os.path.join(topdir,'2018_data','JetHT_Autumn18_Run2018.root'))
inputs.append(os.path.join(topdir,'2018_data','JetHT_Autumn18_Run2018?.root'))
# MET
outputs.append(os.path.join(topdir,'2018_data','MET_Autumn18_Run2018.root'))
inputs.append(os.path.join(topdir,'2018_data','MET_Autumn18_Run2018?.root'))

for ip,op in zip(inputs,outputs):
    cmd = 'python mergeHadd.py -f {} {}'.format(op,ip)
    os.system(cmd)
