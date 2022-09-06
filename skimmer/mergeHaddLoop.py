####################################################
# run the mergeHadd.py command for number of files #
####################################################

import os
import sys

# settings
topdir = '/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate_UL'

# initializations
inputs = []
outputs = []

### 2016PreVFP data
# SingleMuon
outputs.append(os.path.join(topdir,'2016PreVFP_data','SingleMuon_Summer16_Run2016PreVFP.root'))
inputs.append(os.path.join(topdir,'2016PreVFP_data','SingleMuon_Summer16_Run2016?.root'))
# SingleElectron
outputs.append(os.path.join(topdir,'2016PreVFP_data','SingleElectron_Summer16_Run2016PreVFP.root'))
inputs.append(os.path.join(topdir,'2016PreVFP_data','SingleElectron_Summer16_Run2016?.root'))
# DoubleMuon
outputs.append(os.path.join(topdir,'2016PreVFP_data','DoubleMuon_Summer16_Run2016PreVFP.root'))
inputs.append(os.path.join(topdir,'2016PreVFP_data','DoubleMuon_Summer16_Run2016?.root'))
# DoubleEG
outputs.append(os.path.join(topdir,'2016PreVFP_data','DoubleEG_Summer16_Run2016PreVFP.root'))
inputs.append(os.path.join(topdir,'2016PreVFP_data','DoubleEG_Summer16_Run2016?.root'))
# MuonEG
outputs.append(os.path.join(topdir,'2016PreVFP_data','MuonEG_Summer16_Run2016PreVFP.root'))
inputs.append(os.path.join(topdir,'2016PreVFP_data','MuonEG_Summer16_Run2016?.root'))

### 2016PostVFP data
# SingleMuon
outputs.append(os.path.join(topdir,'2016PostVFP_data','SingleMuon_Summer16_Run2016PostVFP.root'))
inputs.append(os.path.join(topdir,'2016PostVFP_data','SingleMuon_Summer16_Run2016?.root'))
# SingleElectron
outputs.append(os.path.join(topdir,'2016PostVFP_data','SingleElectron_Summer16_Run2016PostVFP.root'))
inputs.append(os.path.join(topdir,'2016PostVFP_data','SingleElectron_Summer16_Run2016?.root'))
# DoubleMuon
outputs.append(os.path.join(topdir,'2016PostVFP_data','DoubleMuon_Summer16_Run2016PostVFP.root'))
inputs.append(os.path.join(topdir,'2016PostVFP_data','DoubleMuon_Summer16_Run2016?.root'))
# DoubleEG
outputs.append(os.path.join(topdir,'2016PostVFP_data','DoubleEG_Summer16_Run2016PostVFP.root'))
inputs.append(os.path.join(topdir,'2016PostVFP_data','DoubleEG_Summer16_Run2016?.root'))
# MuonEG
outputs.append(os.path.join(topdir,'2016PostVFP_data','MuonEG_Summer16_Run2016PostVFP.root'))
inputs.append(os.path.join(topdir,'2016PostVFP_data','MuonEG_Summer16_Run2016?.root'))

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

for ip,op in zip(inputs,outputs):
    cmd = 'python mergeHadd.py -f {} {}'.format(op,ip)
    os.system(cmd)
