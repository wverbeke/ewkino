#!/bin/bash

CMSSW="CMSSW_9_2_4"

setCMSSW(){
    echo "cd /user/wverbeke/${CMSSW}/src" >> $1
    echo "source /cvmfs/cms.cern.ch/cmsset_default.sh" >> $1
    echo "eval \`scram runtime -sh\`" >> $1
}

makeSubmit(){
    > $1
    setCMSSW $1
    echo "cd $2" >> $1
}
