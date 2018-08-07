#!/bin/bash

CMSSW="CMSSW_9_4_9"

#set up a CMSSW environment if there is none
if [ ! -d /user/${USER}/${CMSSW} ]; then
    cd ~
    cmsrel $CMSSW
    cd ${CMSSW}/src
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
    output=~/outputCheck.txt
    qsub $1 -l walltime=$2 > $output 2>> $output
    while grep "Invalid credential" $output || grep "Error" $output || grep "Expired credential" $output; do
        echo "Invalid credential or batch protocol error caught, resubmitting"
        sleep 2  #sleep 2 seconds before attemtping resubmission
        qsub $1 -l walltime=$2 > $output 2>> $output
    done
    cat $output
    rm $output
}
