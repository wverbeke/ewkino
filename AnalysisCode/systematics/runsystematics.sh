source /cvmfs/cms.cern.ch/cmsset_default.sh
cd ~/CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/systematics
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/systematics
./runsystematics /user/llambrec/Files/tthid/signalregion_test/2016MC/tZq_ll_4f_13TeV-amcatnlo-pythia8_Summer16.root 0.00414030341603 /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/systematics/test/tZq_ll_4f_13TeV-amcatnlo-pythia8_Summer16.root tth _abs_eta_recoil apath 0.0 5.0 20 JEC pileup
