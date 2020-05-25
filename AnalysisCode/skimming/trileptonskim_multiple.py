###########################################################
# run multiple instances of trileptonskim.py sequentially #
###########################################################
import os

cmdlist = []

cmdlist.append('python trileptonskim.py /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino/ ../samplelists/samplelist_tzq_2016_MC.txt ~/Files/trileptonskim/2016MC')
cmdlist.append('python trileptonskim.py /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino/ ../samplelists/samplelist_tzq_2017_MC.txt ~/Files/trileptonskim/2017MC')
cmdlist.append('python trileptonskim.py /pnfs/iihe/cms/store/user/wverbeke/heavyNeutrino/ ../samplelists/samplelist_tzq_2018_MC.txt ~/Files/trileptonskim/2018MC')

for cmd in cmdlist:
    os.system(cmd)
