#!/bin/bash

#include functions to export CMSSW in submit
source setCMSSW.sh

fillJob(){
    echo "
    name=${1%/} \n
    name=${name##*/} \n
    name=${name#ntuples_temp_*} \n
    name=${name}.root \n
    if [ ! -d ~/Work/ntuples_ewkino ] \n
        then mkdir ~/Work/ntuples_ewkino \n
    fi \n
    if [ -f ~/Work/ntuples_ewkino/$name ] \n
        then rm ~/Work/ntuples_ewkino/$name \n
    fi \n
    hadd ~/Work/ntuples_ewkino/$name ${1}/*root \n
    " >> $2
}

mergeTuple(){
    > mergeJob.sh
    fillJob $1 mergeJob.sh 
    bash mergeJob.sh
    rm mergeJob.sh
}

submitMergeTuple(){
    setCMSSW mergeJob.sh
    fillJob $1 mergeJob.sh
    qsub mergeJob.sh -l walltime=02:00:00 
    rm mergeJob.sh
}

skimmedTuples=~/Work/ntuples_temp_*
mergeTuples(){
    for d in $skimmedTuples
        do mergeTuple $d
        #rm -r $d
    done
}

submitMergeTuples(){
    for d in $skimmedTuples
        do submitMergeTuple $d
    done
}
