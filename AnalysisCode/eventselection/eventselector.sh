echo "Setting CMSSW version to CMSSW_10_2_16_patch1."
echo "To change the CMSSW version, go to ewkino/skimmer/jobSubmission.py"
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd ~/CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
./eventselector /user/llambrec/Files/trileptonskim/WWW_4F_DiLeptonFilter_TuneCUETP8M1_13TeV-amcatnlo-pythia8_Summer16.root /user/llambrec/Files/signalregion signalregion
cd /user/llambrec/Files/signalregion
mv *WWW_4F_DiLeptonFilter_TuneCUETP8M1_13TeV-amcatnlo-pythia8_Summer16.root WWW_4F_DiLeptonFilter_TuneCUETP8M1_13TeV-amcatnlo-pythia8_Summer16.root
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/eventselection
