##############################################################
# run the combine executable for fully inclusive measurement #
##############################################################

import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
sys.path.append(os.path.abspath('../tools'))
import listtools as lt
import combinetools as cbt

def cardtochannel( card ):
    return card.replace('datacard_','').replace('dc_combined','').replace('.txt','')

def getcardcombinations(datacarddir,verbose=False):
    ### get a dictionary linking combined names to lists of corresponding datacards
    # note: assumed to be run on a clean data card directory, 
    #       containing only elementary datacards and corresponding histograms
    combineddict = {}
    cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
    cards_all = lt.subselect_strings(cards_all,maynotcontainone=['_out_'])[1]
    cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
    cards_cr = lt.subselect_strings(cards_all,mustcontainone=['controlregion','signalsideband'])[1]
    # combinations of signal regions per year
    combineddict['dc_combined_sr_2016.txt'] = lt.subselect_strings(cards_sr,
						mustcontainall=['2016'])[1]
    combineddict['dc_combined_sr_2017.txt'] = lt.subselect_strings(cards_sr,
						mustcontainall=['2017'])[1]
    combineddict['dc_combined_sr_2018.txt'] = lt.subselect_strings(cards_sr,
						mustcontainall=['2018'])[1]
    combineddict['dc_combined_sr_1617.txt'] = lt.subselect_strings(cards_sr,
                                            mustcontainone=['2016','2017'])[1]
    combineddict['dc_combined_sr_all.txt'] = cards_sr[:]
    # combinations of years per signal region
    combineddict['dc_combined_sr_cat1.txt'] = lt.subselect_strings(cards_sr,
						mustcontainall=['_cat1_'])[1]
    combineddict['dc_combined_sr_cat2.txt'] = lt.subselect_strings(cards_sr,
						mustcontainall=['_cat2_'])[1]
    combineddict['dc_combined_sr_cat3.txt'] = lt.subselect_strings(cards_sr,
						mustcontainall=['_cat3_'])[1]
    # combinations of years per signal region (+ control regions!)
    combineddict['dc_combined_cat1.txt'] = (lt.subselect_strings(cards_sr,
						    mustcontainall=['_cat1_'])[1]
                                                    + cards_cr)
    combineddict['dc_combined_cat2.txt'] = (lt.subselect_strings(cards_sr,
						    mustcontainall=['_cat2_'])[1]
                                                    + cards_cr)
    combineddict['dc_combined_cat3.txt'] = (lt.subselect_strings(cards_sr,
						    mustcontainall=['_cat3_'])[1]
                                                    + cards_cr)
    # combinations of signal regions per year (+ control regions!)
    combineddict['dc_combined_2016.txt'] = lt.subselect_strings(cards_all,
						mustcontainall=['2016'])[1]
    combineddict['dc_combined_2017.txt'] = lt.subselect_strings(cards_all,
						mustcontainall=['2017'])[1]
    combineddict['dc_combined_2018.txt'] = lt.subselect_strings(cards_all,
						mustcontainall=['2018'])[1]
    combineddict['dc_combined_1617.txt'] = lt.subselect_strings(cards_all,
						mustcontainone=['2016','2017'])[1]
    combineddict['dc_combined_all.txt'] = cards_all[:]
    # convert list of cards to dictionary of cards to channel names
    for combcard,elcards in combineddict.items():
        elcarddict = {}
        for card in elcards: elcarddict[card] = cardtochannel(card)
        combineddict[combcard] = elcarddict

    # print out which combinations will be made
    if not verbose: return combineddict
    print('found following card combinations:')
    for combcard in sorted(list(combineddict.keys())):
        print('  '+combcard)
        for card,ch in combineddict[combcard].items():
            print('    {} (channel {})'.format(card,ch))
    return combineddict


if __name__=="__main__":

    # parse command line args
    if len(sys.argv)<2:
	print('### ERROR ###: need at least one command line arg (input folder)')
	sys.exit()
    datacarddir = os.path.abspath(sys.argv[1])
    usedata = False
    only2016 = False
    only2017 = False
    only2018 = False
    runelementary = True
    runcombinations = True
    runfast = False
    dostatonly = True
    runlocal = False
    runqsub = False
    for arg in sys.argv[2:]:
	if arg=='usedata': usedata = True
	elif arg=='only2016': only2016 = True
	elif arg=='only2017': only2017 = True
	elif arg=='only2018': only2018 = True
	elif arg=='noelementary': runelementary = False
	elif arg=='nocombinations': runcombinations = False
	elif arg=='runfast': runfast = True
	elif arg=='nostat': dostatonly = False
	elif arg=='runlocal': runlocal = True
	elif arg=='runqsub': runqsub = True
    runcondor = True
    if( runqsub or runlocal ): runcondor = False 

    # remove all previous output
    cbt.cleandatacarddir(datacarddir)

    # choose method
    method = 'fitdiagnostics'
    if runfast: method = 'multidimfit'
    #method = 'initimpacts' # temp for testing
 
    # part 1: run combine command for all signal regions separately
    if runelementary:
	cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
	cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
	if only2016: cards_sr = lt.subselect_strings(cards_sr,mustcontainall=['2016'])[1]
	if only2017: cards_sr = lt.subselect_strings(cards_sr,mustcontainall=['2017'])[1]
	if only2018: cards_sr = lt.subselect_strings(cards_sr,mustcontainall=['2018'])[1]

	# temporary
	#cards_sr = lt.subselect_strings(cards_sr,mustcontainall=['cat1','2018'])[1]

	for card in sorted(cards_sr):
	    print('running combine for '+card)
	    commands = cbt.get_default_commands( datacarddir, card, method=method,
						includesignificance=not runfast,
						includestatonly=dostatonly, 
						includedata=usedata )
	    if( runcondor ):
		ct.submitCommandsAsCondorJob( 'cjob_runcombine', commands )
	    else:
		script_name = 'qsub_runcombine.sh'
		with open( script_name, 'w' ) as script:
		    initializeJobScript( script )
		    for c in commands: script.write(c+'\n')
		if runlocal:
		    os.system('bash {}'.format(script_name))
		elif runqsub:
		    submitQsubJob( script_name ) 

    # part 2: run combine command for a number of combination of cards
    if runcombinations:
	combinationdict = getcardcombinations(datacarddir,verbose=True)
	combinedcards = cbt.makecombinedcards(datacarddir,combinationdict)
	for card in sorted(combinedcards):
	    if(only2016 and not '2016' in card): continue
	    if(only2017 and not '2017' in card): continue
	    if(only2018 and not '2018' in card): continue
	    print('running combine for '+card)
	    commands = cbt.get_default_commands( datacarddir, card, method=method,
						includesignificance=not runfast, 
						includestatonly=dostatonly, 
						includedata=usedata )
	    if( runcondor ):
		ct.submitCommandsAsCondorJob( 'cjob_runcombine', commands,
						cmssw_version = 'CMSSW_10_2_16_patch1' )
	    else:
		script_name = 'qsub_runcombine.sh'
		with open( script_name, 'w' ) as script:
		    initializeJobScript( script )
		    for c in commands: script.write(c+'\n')
		if runlocal:
		    os.system('bash {}'.format(script_name))
		elif runqsub:
		    submitQsubJob( script_name )
