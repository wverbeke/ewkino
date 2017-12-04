#!/bin/bash
for jetCat in 0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets; do
    for mllCat in onZ offZ; do
        ./trainBDT $jetCat $mllCat
        for method in BDT BDTG BDTGAlt BDTGAlt_200trees BDTGAlt_shrinkage04 BDTGAlt_minNode005 BDTGAlt_negWeights BDTGAlt_MaxDepth4 BDTGAlt_20Cuts BDTGOtherAn BDTD BDTB MLP DNN; do
            mv dataset/weights/TMVAClassification_${method}.weights.xml bdtWeights/${jetCat}_${mllCat}_${method}.weights.xml
        done
    done
done
