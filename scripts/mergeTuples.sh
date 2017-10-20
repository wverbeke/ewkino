#!/bin/bash

#include functions to export CMSSW in submit
source setCMSSW.sh

fillJob(){
    echo "name=\${$1%/}" >> $2
    echo 'name=${name##*/}' >> $2
    echo 'name=${name#ntuples_temp_*}' >> $2
    echo 'name=${name}.root' >> $2
    echo 'if [ ! -d ~/Work/ntuples_ewkino ]' >> $2
    echo '    then mkdir ~/Work/ntuples_ewkino' >> $2
    echo 'fi' >> $2
    echo 'if [ -f ~/Work/ntuples_ewkino/$name ]' >> $2
    echo '    then rm ~/Work/ntuples_ewkino/$name' >> $2
    echo 'fi' >> $2
    echo 'hadd ~/Work/ntuples_ewkino/$name ${1}/*root' >> $2
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
