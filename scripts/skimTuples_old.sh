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

skimSample(){
    name="${1%/*}"                                      #emove everything before the last "/" in the path to the sample
    echo "$name"
    outputDir=~/Work/ntuples_temp_${name}
    if [ ! -d "$outputDir" ]; then                      #make output directory if it doesn't exist 
        mkdir $outputDir
    fi
    submit=~/skimJob.sh
    makeSubmit $submit                                  #make temporary submission script
    
    count=0                                             #file counter
    files=${1}/*/*/*root
    for f in $files
        do if (( $count % 50 == 0))
            then qsub $submit -l walltime=04:00:00;
            makeSubmit $submit
        fi
        filename=${f##*/}                               
        filename=${filename%.*}
        echo "~/Work/AnalysisCode/ewkino2017/skimTree $f $outputDir/ > ${outputDir}/${filename}_log.txt 2> ${outputDir}/${filename}_err.txt" >> $submit
        count=$((count+1))
    done
    qsub $submit -l walltime=04:00:00;
    rm $submit                                          #Remove temporary submit file
}

baseFolder=/pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino
cd $baseFolder
foldersMC=*/*ewkinoMCList* 
foldersData=*/*ewkinoDataList*



for d in $foldersMC $foldersData
    do name="${d%/*}"
    echo "$name"
    outputDir=~/Work/ntuples_temp_${name}
    if [ ! -d "$outputDir" ]; then
        mkdir $outputDir
    fi
    files=${d}/*/*/*root
    submit=~/skimJob.sh
    > $submit
    echo "cd /user/wverbeke/CMSSW_9_2_4/src/" >> $submit
    echo "source /cvmfs/cms.cern.ch/cmsset_default.sh" >> $submit
    echo "eval \`scram runtime -sh\`" >> $submit
    echo "cd $baseFolder" >> $submit
    count=0    
    for f in $files
        do if (( $count % 50 == 0)); then
            qsub $submit -l walltime=04:00:00;
            #echo "~~~~~~~~~~~~~~~~~~~~~~"
            #cat $submit
            cat $submit >> ${outputDir}/${filename}_log.txt
            > $submit
            echo "cd /user/wverbeke/CMSSW_9_2_4/src/" >> $submit
            echo "source /cvmfs/cms.cern.ch/cmsset_default.sh" >> $submit
            echo "eval \`scram runtime -sh\`" >> $submit
            echo "cd $baseFolder" >> $submit
        fi 
        #do ~/Work/AnalysisCode/ttg/skimTree "$f"
        filename=${f##*/}
        filename=${filename%.*}
        echo "~/Work/AnalysisCode/ewkino2017/skimTree $f $outputDir/ > ${outputDir}/${filename}_log.txt 2> ${outputDir}/${filename}_err.txt" >> $submit
        #echo "skimmed $f"
        count=$((count+1))
    done
    qsub $submit -l walltime=04:00:00;
    #echo "~~~~~~~~~~~~~~~~~~~~~"
    #cat $submit
    rm $submit
    #add another qsub here
    #name="${d%/*}"
    #echo "$name"
    #hadd ~/Work/ntuples_ttg/${name} ~/Work/ntuples_temp/*root
    #rm ~/Work/ntuples_temp/*root
done

