#!/bin/bash

CMSSW="CMSSW_9_4_4"

#set up a CMSSW environment if there is none
if [ ! -d /user/${USER}/${CMSSW} ]; then
    cd ~
    cmsrel CMSSW_9_4_4
    cd CMSSW_9_4_4/src
    cmsenv
fi

#set up CMSSW in submission script
setCMSSW(){
    echo "cd /user/${USER}/${CMSSW}/src" >> $1
    echo "source /cvmfs/cms.cern.ch/cmsset_default.sh" >> $1
    echo "eval \`scram runtime -sh\`" >> $1
}

#prepare submission script
makeSubmit(){
    > $1
    setCMSSW $1
    echo "cd $2" >> $1
}

#submit a job and catch invalid credentials in the process
submitJob(){
    qsub $1 -l walltime=04:00:00 > outputCheck.txt 2>> outputCheck.txt
    while grep "Invalid credential" outputCheck.txt; do
        echo "Invalid credential caught, resubmitting"
        sleep 2  #sleep 2 seconds before attemtping resubmission
        qsub $1 -l walltime=04:00:00 > outputCheck.txt 2>> outputCheck.txt
    done
    cat outputCheck.txt
    rm outputCheck.txt
}
