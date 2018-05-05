#!/bin/bash
source ../scripts/setCMSSW.sh

for year in 2016 2017; do
    #for jetCat in  0bJets01Jets 0bJets2Jets 1bJet01jets 1bJet23Jets 1bJet4Jets 2bJets; do
    for jetCat in  1bJet23Jets 1bJet4Jets 2bJets; do
        #for mllCat in onZ offZ; do
        for mllCat in onZ; do
            script=mvaTraining_${jetCat}_${mllCat}_${year}.sh
            > $script
            makeSubmit $script $PWD
            echo "./trainBDT $jetCat $mllCat $year" >> $script
            echo "for method in BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1; do" >> $script
            echo "  mv dataset${jetCat}_${mllCat}_${year}/weights/TMVAClassification${jetCat}_${mllCat}_${year}_\${method}.weights.xml bdtWeights/${jetCat}_${mllCat}_${year}_\${method}.weights.xml" >> $script
            echo "done">> $script
            #qsub $script -q highmem -lnodes=1:ppn=16 -l walltime=40:00:00
            #qsub $script -l walltime=40:00:00
            submitJob $script
            #cat $script
            rm $script
        done
    done
done
