#####################################################################
# script to run the required checks for datacard and combine method #
#####################################################################
# see here: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsWG/HiggsPAGPreapprovalChecks
# note: best to copy the datacard + template files somewhere to keep output separated

import sys
import os

datacard = sys.argv[1]
[datacarddir,datacard] = datacard.split('/',-1)
dodcval = True
dotoybkg = True
dotoysig = True
scanparams = ['norm_nonprompt','JEC']

cwd = os.getcwd()
script_name = 'checkdatacard.sh'
if os.path.exists(script_name):
    os.system('rm '+script_name)

# datacard validation tool
if dodcval:
    command = 'ValidateDatacards.py '+datacard
    with open(script_name,'a') as script:
	script.write('cd '+datacarddir+'\n')
	script.write(command+'\n')
	script.write('cd '+cwd+'\n')

# run FitDiagnostics on asimov set without signal
if dotoybkg:
    name = datacard.replace('.txt','_bkg')
    workspace = name+'.root'

    wscommand = 'text2workspace.py '+datacard+' -o '+workspace
    fitcommand = 'combine -M FitDiagnostics -t -1 --expectSignal 0 '+workspace
    fitcommand += ' -n '+name

    diffcommand = 'python ~/CMSSW_10_2_16_patch1/src/HiggsAnalysis/CombinedLimit/'
    diffcommand += 'test/diffNuisances.py -a '+'fitDiagnostics'+workspace+' -g '+name+'_plots.root'

    ipcommand = 'python impactplot.py '+os.path.join(datacarddir,workspace)
    ipcommand += ' runblind=True expectsignal=False'

    with open(script_name,'a') as script:
	script.write('cd '+datacarddir+'\n')
	script.write(wscommand+'\n')
        script.write(fitcommand+'\n')
	script.write(diffcommand+'\n')
	script.write('cd '+cwd+'\n')
	script.write(ipcommand+'\n')

# run FitDiagnostics on asimov set with signal
if dotoysig:
    name = datacard.replace('.txt','_sig')
    workspace = name+'.root'

    wscommand = 'text2workspace.py '+datacard+' -o '+workspace
    fitcommand = 'combine -M FitDiagnostics -t -1 --expectSignal 1 '+workspace
    fitcommand += ' -n '+name

    diffcommand = 'python ~/CMSSW_10_2_16_patch1/src/HiggsAnalysis/CombinedLimit/'
    diffcommand += 'test/diffNuisances.py -a '+'fitDiagnostics'+workspace+' -g '+name+'_plots.root'

    ipcommand = 'python impactplot.py '+os.path.join(datacarddir,workspace)
    ipcommand += ' runblind=True expectsignal=True'

    with open(script_name,'a') as script:
        script.write('cd '+datacarddir+'\n')
        script.write(wscommand+'\n')
        script.write(fitcommand+'\n')
        script.write(diffcommand+'\n')
        script.write('cd '+cwd+'\n')
        script.write(ipcommand+'\n')

# scan parameters that are strongly constrained
if len(scanparams)>0:
    scancommands = []
    plotcommands = []
    for scanparam in scanparams:
	scancommand = 'combine '+datacard+' -M MultiDimFit'
	scancommand += ' --algo grid'
	scancommand += ' -t -1 --expectSignal 1'
	scancommand += ' --redefineSignalPOIs '+scanparam
	scancommand += ' --setParameterRanges '+scanparam+'=-3,3'
	scancommands.append(scancommand)

	plotcommand = 'plot1DScan.py higgsCombineTest.MultiDimFit.mH120.root'
	plotcommand += ' --POI '+scanparam
	plotcommand += ' -o scan_'+datacard.replace('.txt','_')+scanparam
	plotcommands.append(plotcommand)
    
    with open(script_name,'a') as script:
        script.write('cd '+datacarddir+'\n')
        for sc,pc in zip(scancommands,plotcommands): 
	    script.write(sc+'\n')
	    script.write(pc+'\n')
        script.write('cd '+cwd+'\n')
    
# run script 
os.system('bash '+script_name)
