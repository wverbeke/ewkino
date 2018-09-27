source setCMSSW.sh
years=("" 2017)
for process in tZq controlRegions tZq_lowBDT tZq_highBDT tZq_splitChannels nonpromptControlRegions; do
    for year in "${years[@]}"; do
        script=${process}${year}.sh
        makeSubmit $script ${PWD}/..
        echo "make -f makeFiles/make${process}${year}" >> $script
        echo "./${process}${year}" >> $script
        submitJob $script "24:00:00"
    done
done

for process in tZq_combinedPlots tZq_withPsUnc2017; do
   script=${process}.sh
   makeSubmit $script ${PWD}/..
   echo "make -f makeFiles/make${process}" >> $script
   echo "./${process}" >> $script
   submitJob $script "24:00:00"
done
