source /cvmfs/cms.cern.ch/cmsset_default.sh
cd
cd CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/skimming
cd /storage_mnt/storage/user/llambrec/ewkino/skimmer
./skimmer /storage_mnt/storage/user/llambrec/ewkino/test/testData/unskimmed/DoubleMuon/Run2017E_test/testdata_Run2017EDoubleMuon.root /storage_mnt/storage/user/llambrec/ewkino/test/testData/skimmed/ntuples_skimmed_DoubleMuon_Fall17_version_Run2017E_test trilepton
if [ $? -ne 0 ]
then
	echo "###failed###"
else
	echo "###succeeded###"
fi