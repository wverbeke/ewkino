#!/bin/bash
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
