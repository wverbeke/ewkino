echo "Setting CMSSW version to CMSSW_10_2_16_patch1."
echo "To change the CMSSW version, go to ewkino/skimmer/jobSubmission.py"
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd ~/CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
hadd /user/llambrec/Files/test/GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8_Summer16.root /user/llambrec/Files/trileptonskim/ntuples_skimmed_GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8_Summer16_version_singlelepton_MC_2016_v1/*.root
rm -r /user/llambrec/Files/trileptonskim/ntuples_skimmed_GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8_Summer16_version_singlelepton_MC_2016_v1
