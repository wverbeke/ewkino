#!/bin/bash

#include functions to export CMSSW in submit
source setCMSSW.sh

mergeTuple(){
    name=${1%/}
    name=${name##*/}
    name=${name#ntuples_temp_*}
    name=${name}.root
    if [ ! -d ~/Work/ntuples_ewkino ]
        then mkdir ~/Work/ntuples_ewkino
    fi
    if [ -f ~/Work/ntuples_ewkino/$name ]
        then rm ~/Work/ntuples_ewkino/$name
    fi
    hadd ~/Work/ntuples_ewkino/$name ${1}/*root
}

submitMerge(){
    job="mergeJob.sh"
    makeSubmit() $job
    
}

skimmedtuples=~/Work/ntuples_temp_*
mergeTuplesInteractive(){
    for d in $skimmedtupls
        do mergeTuple $d
        rm -r $d
    done
}


skimmedtuples=~/Work/ntuples_temp_*
for d in $skimmedtuples
    do name=${d%/}
    name=${name##*/}
    name=${name#ntuples_temp_*}
    name=${name}.root
    if [ ! -d ~/Work/ntuples_ewkino ]
        then mkdir ~/Work/ntuples_ewkino
    fi
    if [ -f ~/Work/ntuples_ewkino/$name ]
        then rm ~/Work/ntuples_ewkino/$name
    fi
    hadd ~/Work/ntuples_ewkino/$name ${d}/*root
    #rm -r $d
done
