#!/bin/bash
mkdir -p ../plots/
cd ../plots/
mkdir -p ttgamma
mkdir -p ewkino
mkdir -p tZq
#set up ewkino plots
cd ewkino
mkdir -p dilepCR
cd dilepCR
#make directories for every run
mkdir -p 2017
for dir in all2017 RunB RunC RunD RunE RunF
    do mkdir -p 2017/$dir
done
mkdir -p 2016
for dir in all2016 RunB RunC RunD RunE RunF RunG RunH
    do mkdir -p 2016/$dir
done

for dir in ./*/*
    do for subdir in inclusive ee em mm same-sign-ee same-sign-em same-sign-mm
        do for jetDir in nJetsInclusive 1pt40Jet
            do for puDir in noPuW PuW
                do mkdir -p $dir/$subdir/${jetDir}/${puDir}
            done
        done
    done
done
cd ../..

#set up ttgamma plots
cd ttgamma
for dir in inclusive ee em mm
    do mkdir -p $dir
done
cd ..

#set up tZq plots 
cd tZq 
for dir1 in 2016 2017 combined; do
    for dir2 in mllInclusive onZ offZ noOSSF; do
        for dir3 in nJetsInclusive 0bJets01Jets 0bJets2Jets 1bJet01jets 1bJet23Jets 1bJet4Jets 2bJets; do
            for dir4 in flavorInclusive eee eem emm mmm; do
                mkdir -p ${dir1}/${dir2}/${dir3}/${dir4}
            done
        done
    done 
    for dir2 in controlR; do
        for dir3 in WZ ZZ Xgamma; do
            mkdir -p ${dir1}/${dir2}/${dir3}
        done
        for dir3 in offZ noOSSF; do
            for dir4 in 1bJet23Jets 1bJet4Jets 2bJets; do
                mkdir -p ${dir1}/${dir2}/${dir3}/${dir4}
            done
        done
    done
    mkdir -p ${dir1}/lowBDT
    mkdir -p ${dir1}/final
done

for dir1 in TOP-16-020; do
    for dir2 in onZ; do
        for dir3 in nJetsInclusive 0bJets 1bJet23Jets 1bJet4Jets 2bJets; do
            for dir4 in flavorInclusive eee eem emm mmm; do
                mkdir -p ${dir1}/${dir2}/${dir3}/${dir4}
            done
        done
    done 
done
cd ..

#mkdir tZq/dataFakes/
#cd tZq/dataFakes/
#for d in . 36fb 81fb; do
#    for dir in mllInclusive onZ offZ; do
#        for subdir in nJetsInclusive 0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets
#            do mkdir -p ${d}/${dir}/${subdir}
#        done
#    done
#done
#cd ../..

#mkdir tZq/controlR/
#cd tZq/controlR/
#for d in . 36fb 81fb; do
#    for dir in nJetsInclusive 0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets
#        do mkdir -p ${d}/${dir}
#    done
#done
#
#for dir1 in 2016 2017; do
#    for dir2 in mllInclusive onZ offZ; do
#        for dir3 in inclusive 0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets; do
#            mkdir -p ${dir1}/${dir2}/${dir3}
#        done
#    done 
#done
cd ..


