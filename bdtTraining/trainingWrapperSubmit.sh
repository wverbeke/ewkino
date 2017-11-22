#!/bin/bash
source ../scripts/setCMSSW.sh

for jetCat in  0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets; do
    for mllCat in onZ offZ; do
        > mvaTraining.sh
        makeSubmit mvaTraining.sh /user/wverbeke/Work/AnalysisCode/ewkino/bdtTraining/
        echo "./trainBDT $jetCat $mllCat" >> mvaTraining.sh
        echo "for method in BDT BDTG BDTD BDTB MLP DNN; do" >> mvaTraining.sh
        echo '  mv dataset/weights/TMVAClassification_${method}.weights.xml submitWeights/${jetCat}_${mllCat}_${method}.weights.xml' >> mvaTraining.sh
        echo "done">> mvaTraining.sh
        qsub mvaTraining.sh -q highmem -lnodes=1:ppn=16 -l walltime=20:00:00
    done
done
rm mvaTraining.sh
