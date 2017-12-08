#!/bin/bash

#include bash functiosn to set up CMSSW
source setCMSSW.sh

cwd=$(pwd)                                          #current working directory needed to locate code 

skimSample(){                                           #function to skim one sample
    name="${1%/*}"                                      #remove everything before the last "/" in the path to the sample
    echo "$name"
    outputDir=~/Work/ntuples_temp_${name}
    if [ ! -d "$outputDir" ]; then                      #make output directory if it doesn't exist 
        mkdir -p $outputDir
    fi
    submit=~/skimJob.sh
    makeSubmit $submit $2                               #make temporary submission script

    count=0                                             #file counter
    files=${1}/*/*/*root
    for f in $files
        do if (( $count % 50 == 0))
            then qsub $submit -l walltime=04:00:00; > ~/temp.txt 2>> ~/temp.txt
            while grep "Invalid credential;" ~/temp.txt; do
                qsub $submit -l walltime=04:00:00; > ~/temp.txt 2>> ~/temp.txt
            done 
            cat ~/temp.txt
            makeSubmit $submit $2
        fi
        #filename=${f##*/}                               
        filename=${f///}
        filename=${filename%.*}
        echo "${cwd}/../skimTree $f $outputDir/ > ${outputDir}/${filename}_log.txt 2> ${outputDir}/${filename}_err.txt" >> $submit
        count=$((count+1))
    done
    qsub $submit -l walltime=04:00:00 > ~/temp.txt 2>> ~/temp.txt
    while grep "Invalid credential;" ~/temp.txt; do
        qsub $submit -l walltime=04:00:00; > ~/temp.txt 2>> ~/temp.txt
    done 
    cat ~/temp.txt
    rm ~/temp.txt
    rm $submit                                          #remove temporary submit file
}

baseFolder=/pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino
cd $baseFolder
foldersMC=*/*ewkinoMCList-v9                               #add suffix for newer versions
foldersData=*/*Run2017C*2017LeptonicDataList_v9p1
for d in */*Run2017B*2017LeptonicDataList_v9p1 */*Run2017C*2017LeptonicDataList_v9p1 */*Run2017D*2017LeptonicDataList_v9p1 */*Run2017E*2017LeptonicDataList_v9p1 */*Run2017F*Prompt*2017LeptonicDataList_v9p1 $foldersMC                         #skim all samples 
    do skimSample $d $baseFolder
done
