#!/bin/bash
source ./setCMSSW.sh

for year in 2016 2017 do;
    for jetCat in  0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets; do
        for mllCat in onZ offZ; do
            script=mvaTraining_${jetCat}_${mllCat}_${year}.sh
            > $script
            makeSubmit $script $PWD
            echo "./trainBDT $jetCat $mllCat $year" >> $script
            echo "for method in BDTG_m1Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1; do" >> $script
            echo "  mv dataset${jetCat}${mllCat}${year}/weights/TMVAClassification${jetCat}${mllCat}${year}_\${method}.weights.xml submitWeights/${jetCat}_${mllCat}_${year}_\${method}.weights.xml" >> $script
            echo "done">> $script
            #qsub $script -q highmem -lnodes=1:ppn=16 -l walltime=40:00:00
            qsub $script -l walltime=40:00:00
            rm $script
        done
    done
done
