######################################################
# script to run combine executable on all datacards  #
######################################################
import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
from cleandatacarddir import cleandatacarddir

def subselect(stringlist,mustcontain=[],maynotcontain=[]):
    ### select strings that contain ALL tags in mustcontain and NO tags in maynotcontain
    sellist = []
    for s in stringlist:
        keep = True
        for t in mustcontain:
            if t not in s: keep = False; break
        if not keep: continue
	for t in maynotcontain:
	    if t in s: keep = False; break
	if not keep: continue
	sellist.append(s)
    return sellist

def getcardcombinations(datacarddir,doprint=True):
    ### get a dictionary linking combined names to lists of corresponding datacards
    combineddict = {}
    cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
    cards_all = subselect(cards_all,maynotcontain=['out.txt'])
    cards_sr = subselect(cards_all,mustcontain=['signalregion'])
    cards_cr = subselect(cards_all,mustcontain=['controlregion'])
    # combinations of signal regions per year
    combineddict['dc_combined_signalregion_2016'] = subselect(cards_sr,mustcontain=['2016'])
    combineddict['dc_combined_signalregion_2017'] = subselect(cards_sr,mustcontain=['2017'])
    combineddict['dc_combined_signalregion_2018'] = subselect(cards_sr,mustcontain=['2018'])
    combineddict['dc_combined_signalregion_1617'] = (subselect(cards_sr,mustcontain=['2016']) 
						    + subselect(cards_sr,mustcontain=['2017']))
    # combinations of years per signal region (+ control regions!)
    combineddict['dc_combined_signalregion_1'] = (subselect(cards_sr,mustcontain=['_1_'])
						    + cards_cr)
    combineddict['dc_combined_signalregion_2'] = (subselect(cards_sr,mustcontain=['_2_'])
                                                    + cards_cr)
    combineddict['dc_combined_signalregion_3'] = (subselect(cards_sr,mustcontain=['_3_'])
                                                    + cards_cr)
    # combinations of signal regions per year (+ control regions!)
    combineddict['dc_combined_2016'] = subselect(cards_all,mustcontain=['2016'])
    combineddict['dc_combined_2017'] = subselect(cards_all,mustcontain=['2017'])
    combineddict['dc_combined_2018'] = subselect(cards_all,mustcontain=['2018'])
    combineddict['dc_combined_1617'] = (subselect(cards_all,mustcontain=['2016'])
					+ subselect(cards_all,mustcontain=['2017']))
    combineddict['dc_combined_all'] = cards_all
    if not doprint: return combineddict
    for comb in sorted(list(combineddict.keys())):
	print('--- '+comb+' ---')
	for card in combineddict[comb]:
	    print('  '+card)
    return combineddict

def makecombinedcards(datacarddir):
    ### call combineCards functionality on combinations of datacards
    combineddict = getcardcombinations(datacarddir)
    cwd = os.getcwd()
    os.chdir(datacarddir)
    keys = sorted(list(combineddict.keys()))
    for comb in keys:
	if len(combineddict[comb])==0:
	    print('### WARNING ###: no cards found for '+comb+', skipping it.')
	    del combineddict[comb]
	    continue
	combcard = comb+'.txt'
	command = 'combineCards.py'
	for c in combineddict[comb]:
	    channelname = c.strip('datacard_').rstrip('.txt')
	    command += ' '+channelname+'='+c
	command += ' &> '+combcard
	print(command)
	os.system(command)
    os.chdir(cwd)
    return combineddict

def addcombinecommands(script,datacard,runblind):
    # make workspace
    script.write('text2workspace.py '+datacard+'\n')
    workspace = datacard.replace('.txt','.root')
    name = datacard.replace('.txt','')
    # make outputfile
    ss_obs_outfile = datacard.replace('.txt','_out_signalstrength_obs.txt')
    ss_exp_outfile = datacard.replace('.txt','_out_signalstrength_exp.txt')
    sig_obs_outfile = datacard.replace('.txt','_out_significance_obs.txt')
    sig_exp_outfile = datacard.replace('.txt','_out_significance_exp.txt')

    # run FitDiagnostics to compute signal strength
    ss_command = 'combine -M FitDiagnostics '+workspace+' -n '+name
    ss_command += ' --rMin 0 --rMax 5'
    #ss_command += ' --saveShapes --saveWithUncertainties'
    ss_command += ' --cminDefaultMinimizerStrategy 0'
    ss_command += ' --robustFit=1'
    # run blind:
    script.write(ss_command+' -t -1 --expectSignal=1 > '+ss_exp_outfile+' 2> '+ss_exp_outfile+'\n')
    # run with data:
    if not runblind: script.write(ss_command+' > '+ss_obs_outfile+' 2> '+ss_obs_outfile+'\n')

    # run ProfileLikelihood to compute significance
    sig_command = 'combine -M Significance '+workspace+' -n '+name+' --signif'
    sig_command += ' --cminDefaultMinimizerStrategy 0'
    # run blind:
    script.write(sig_command+' -t -1 --expectSignal=1 > '+sig_exp_outfile+' 2> '+sig_exp_outfile+'\n')
    if not runblind: script.write(sig_command+' > '+sig_obs_outfile+' 2> '+sig_obs_outfile+'\n')
    
if __name__=="__main__":

    # global settings    
    datacarddir = 'datacards_newbins'
    runblind = True
    only2016 = False

    # remove all previous output
    cleandatacarddir(datacarddir)
 
    # part 1: run combine command for all signal regions separately
    cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
    cards_signalregion = subselect(cards_all,mustcontain=['signalregion'])
    for card in sorted(cards_signalregion):
	if(only2016 and not '2016' in card): continue
        print('running combine for '+card)
	#make a job script 
        script_name = 'runcombine.sh'
        with open( script_name, 'w') as script:
            initializeJobScript( script )
            script.write('cd {}\n'.format( datacarddir ) )
	    addcombinecommands(script,card,runblind)
        # submit job and catch errors 
        #submitQsubJob( script_name )
        # alternative: run locally (for testing and debugging)
        os.system('bash '+script_name)

    # part 2: run combine command for a number of combination of cards
    combineddict = makecombinedcards(datacarddir)
    for comb in sorted(list(combineddict.keys())):
	if(only2016 and not '2016' in comb): continue
	#if( not ('dc_combined_2016' in comb
	#	 or 'dc_combined_2017' in comb 
	#	 or 'dc_combined_1617' in comb) ): continue # temp!!!
	print('running combine for '+comb)
	card = comb+'.txt'
	#make a job script 
        script_name = 'runcombine.sh'
        with open( script_name, 'w') as script:
            initializeJobScript( script )
            script.write('cd {}\n'.format( datacarddir ) )
            addcombinecommands(script,card,runblind)
        # submit job and catch errors 
        #submitQsubJob( script_name )
        # alternative: run locally (for testing and debugging)
        os.system('bash '+script_name)
