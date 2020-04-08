source /cvmfs/cms.cern.ch/cmsset_default.sh
cd
cd CMSSW_10_2_16_patch1/src
eval `scram runtime -sh`
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/bdt/2016cat1_opt/jobn16
cd 2016cat1_opt/jobn16
python /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/bdt/tmvatrain.py treename=blackJackAndHookers/treeCat1 lopts=SplitMode=Random:NormMode=None varlist=["\"_abs_eta_recoil\"","\"_Mjj_max\"","\"_lW_asymmetry\"","\"_deepCSV_max\"","\"_lT\"","\"_MT\"","\"_dPhill_max\"","\"_pTjj_max\"","\"_dRlb_min\""] fopts=!H:!V:NTrees=400:BoostType=Grad:MinNodeSize=1%:MaxDepth=3:nCuts=50:UseBaggedGrad=True:BaggedSampleFraction=0.5:Shrinkage=0.05 sigtag=tZq indir=/user/llambrec/Files/signalregion/2016MC_flat
echo ---command---
echo python /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/bdt/tmvatrain.py treename=blackJackAndHookers/treeCat1 lopts=SplitMode=Random:NormMode=None varlist=["\"_abs_eta_recoil\"","\"_Mjj_max\"","\"_lW_asymmetry\"","\"_deepCSV_max\"","\"_lT\"","\"_MT\"","\"_dPhill_max\"","\"_pTjj_max\"","\"_dRlb_min\""] fopts=!H:!V:NTrees=400:BoostType=Grad:MinNodeSize=1%:MaxDepth=3:nCuts=50:UseBaggedGrad=True:BaggedSampleFraction=0.5:Shrinkage=0.05 sigtag=tZq indir=/user/llambrec/Files/signalregion/2016MC_flat