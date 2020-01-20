source /cvmfs/cms.cern.ch/cmsset_default.sh
cd
cd CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
./eventcategorizer /user/llambrec/Files/signalregion/WWTo2L2Nu_DoubleScattering_13TeV-pythia8_Summer16.root /user/llambrec/Files/test signalregion
cd /user/llambrec/Files/test
mv *WWTo2L2Nu_DoubleScattering_13TeV-pythia8_Summer16.root WWTo2L2Nu_DoubleScattering_13TeV-pythia8_Summer16.root
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
