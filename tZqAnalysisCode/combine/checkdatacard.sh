cd datacards_fortesting
ValidateDatacards.py datacard_signalregion_cat1_2016.txt
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/combine
cd datacards_fortesting
text2workspace.py datacard_signalregion_cat1_2016.txt -o datacard_signalregion_cat1_2016_bkg.root
combine -M FitDiagnostics -t -1 --expectSignal 0 datacard_signalregion_cat1_2016_bkg.root -n datacard_signalregion_cat1_2016_bkg
python ~/CMSSW_10_2_16_patch1/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py -a fitDiagnosticsdatacard_signalregion_cat1_2016_bkg.root -g datacard_signalregion_cat1_2016_bkg_plots.root
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/combine
python impactplot.py datacards_fortesting/datacard_signalregion_cat1_2016_bkg.root runblind=True expectsignal=False
cd datacards_fortesting
text2workspace.py datacard_signalregion_cat1_2016.txt -o datacard_signalregion_cat1_2016_sig.root
combine -M FitDiagnostics -t -1 --expectSignal 1 datacard_signalregion_cat1_2016_sig.root -n datacard_signalregion_cat1_2016_sig
python ~/CMSSW_10_2_16_patch1/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py -a fitDiagnosticsdatacard_signalregion_cat1_2016_sig.root -g datacard_signalregion_cat1_2016_sig_plots.root
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/combine
python impactplot.py datacards_fortesting/datacard_signalregion_cat1_2016_sig.root runblind=True expectsignal=True
cd datacards_fortesting
combine datacard_signalregion_cat1_2016.txt -M MultiDimFit --algo grid -t -1 --expectSignal 1 --redefineSignalPOIs norm_nonprompt --setParameterRanges norm_nonprompt=-3,3
plot1DScan.py higgsCombineTest.MultiDimFit.mH120.root --POI norm_nonprompt -o scan_datacard_signalregion_cat1_2016_norm_nonprompt
combine datacard_signalregion_cat1_2016.txt -M MultiDimFit --algo grid -t -1 --expectSignal 1 --redefineSignalPOIs JEC --setParameterRanges JEC=-3,3
plot1DScan.py higgsCombineTest.MultiDimFit.mH120.root --POI JEC -o scan_datacard_signalregion_cat1_2016_JEC
cd /storage_mnt/storage/user/llambrec/ewkino/AnalysisCode/combine
