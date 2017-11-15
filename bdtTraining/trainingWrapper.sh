#!/bin/bash
#0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets; do
for jetCat in  1bJet_01jets; do
    for mllCat in onZ offZ; do
        ./trainBDT $jetCat $mllCat
        mv dataset/weights/TMVAClassification_BDT.weights.xml bdtWeights/${jetCat}_${mllCat}_BDT.weights.xml
    done
done
