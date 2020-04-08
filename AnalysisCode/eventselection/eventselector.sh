source /cvmfs/cms.cern.ch/cmsset_default.sh
cd
cd CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
./eventselector /user/llambrec/Files/trileptonskim/2016data/MuonEG_Summer16.root /user/llambrec/Files/test/2016data_tZqID MuonEG_Summer16.root signalregion True
