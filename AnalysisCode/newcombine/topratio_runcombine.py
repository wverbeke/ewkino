##################################################################
# run the combine executable for measuring the top/antitop ratio #
##################################################################
# note: input datacards can be the same as those produced with topchannels_makedatacards.py

import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
sys.path.append(os.path.abspath('../tools'))
import listtools as lt
import combinetools as cbt
from topchannels_makedatacards import get_topchannel_parameters

def cardtochannel( card ):
    return card.replace('datacard_','').replace('dc_combined','').replace('.txt','')

def getcardcombinations(datacarddir,verbose=False):
    ### get a dictionary linking combined names to lists of corresponding datacards
    # note: assumed to be run on a clean data card directory, 
    #       containing only elementary datacards and corresponding histograms
    combineddict = {}
    cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
    cards_all = lt.subselect_strings(cards_all,maynotcontainone=['out.txt'])[1]
    combineddict['dc_combined_2016.txt'] = lt.subselect_strings(cards_all,
						mustcontainall=['2016'])[1]
    combineddict['dc_combined_2017.txt'] = lt.subselect_strings(cards_all,
						mustcontainall=['2017'])[1]
    combineddict['dc_combined_2018.txt'] = lt.subselect_strings(cards_all,
						mustcontainall=['2018'])[1]
    combineddict['dc_combined_all.txt'] = cards_all
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

def get_po_map( channels, topchannels ):
    ### define the mapping of signal strengths
    # arguments are not used for now, but keep for easy generalization later.
    # the current approach assumes that tZq, tZq_top and tZq_antitop are all present
    # in the datacard to which the pomap will be applied (else errors?).
    # input arguments:
    # - channels: a list of channel names (present in a (combined) datacard)
    #   (based on the names, it will be decided what parameters are present)
    # - topchannels: a dict mapping channel suffixes (e.g. _ch0) to names (e.g. eee)
    pomap = '-P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO verbose'
    parameters = ['r_ratio','r_tZq_antitop']
    # scale tZq (in controlregions) by 1 (i.e. treat it as background)
    pomap += ' --PO "map=.*/tZq:1"'
    # scale tZq_antitop by r_tZq_antitop
    pomap += ' --PO "map=.*/tZq_antitop:r_tZq_antitop[1,0,5]"'
    # scale tZq_top by r_tZq_antitop*r_ratio
    pomap += ' --PO "map=.*/tZq_top:scale_top=expr;;'
    pomap += 'scale_top(\\"@0*@1\\",r_tZq_antitop,r_ratio[1,0,3])"'
    return (parameters,pomap)

if __name__=="__main__":

    # parse command line args
    if len(sys.argv)<2:
	print('### ERROR ###: need at least one command line arg (input folder)')
	sys.exit()
    datacarddir = os.path.abspath(sys.argv[1])
    usedata = False
    only2016 = False
    runlocal = False
    runqsub = False
    doseparateSR = False
    for arg in sys.argv[2:]:
	if arg=='usedata': usedata = True
	elif arg=='only2016': only2016 = True
	elif arg=='runlocal': runlocal = True
	elif arg=='runqsub': runqsub = True
	elif arg=='doseparate': doseparateSR = True
    runcondor = True
    if(runlocal or runqsub): runcondor = False
    dostatonly = True # maybe later add as command line arg

    # remove all previous output
    cbt.cleandatacarddir(datacarddir)
    
    cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
 
    # part 1: run combine command for all signal regions separately
    #         (not normally needed for per-channel measurement)
    if doseparateSR:
	cards_sr = lt.subselect_strings(cards_all,mustcontainall=['signalregion'])[1]
	if only2016: cards_sr = lt.subselect_strings(cards_sr,mustcontainall=['2016'])[1]
	for card in sorted(cards_sr):
	    print('running combine for '+card)
	    commands = cbt.get_default_commands( datacarddir, card, 
			    includestatonly=dostatonly, includedata=usedata )
	    if( not runlocal ):
		ct.submitCommandsAsCondorJob( 'cjob_runcombine', commands )
	    # alternative: run locally (for testing and debugging)
	    else:
		script_name = 'runlocal_runcombine.sh'
		with open( script_name, 'w' ) as script:
		    initialsizeJobScript( script )
		    for c in commands: script.write(c+'\n')
		os.system('bash {}'.format(script_name))

    # part 2: run combined fit of all channels
    combinationdict = getcardcombinations(datacarddir,verbose=True)
    for cname,cdict in combinationdict.items():
	if(only2016 and not '2016' in cname): continue
	# put makecombinedcards inside the loop since the list of elementary channels is required
	cards = cbt.makecombinedcards( datacarddir, {cname:cdict} )
	if len(cards)==0: continue
	if len(cards)>1:
	    raise Exception('ERROR: unexpected number of combined cards returned '
			    +'for what should be a single combination')
	card = cards[0]
        print('running combine for '+card)
	commands = []
        (pois,pomap) = get_po_map( cdict.values(), get_topchannel_parameters()[1] )
        commands += cbt.get_workspace_commands( datacarddir, card, options=pomap )
        commands += cbt.get_multidimfit_commands(
                        datacarddir, card, dostatonly=False, usedata=False,
                        pois=pois )
        if dostatonly:
            commands += cbt.get_multidimfit_commands(
                            datacarddir, card, dostatonly=True, usedata=False,
                            pois=pois )
        if usedata:
            commands += cbt.get_multidimfit_commands(
                            datacarddir, card, dostatonly=False, usedata=True,
                            pois=pois )
        if( usedata and dostatonly ):
            commands += cbt.get_multidimfit_commands(
                            datacarddir, card, dostatonly=True, usedata=True,
                            pois=pois )
	if( runcondor ):
            ct.submitCommandsAsCondorJob( 'cjob_runcombine', commands )
        # alternative: run locally (for testing and debugging)
        else:
            script_name = 'qsub_runcombine.sh'
            with open( script_name, 'w' ) as script:
		initializeJobScript( script )
                for c in commands: script.write(c+'\n')
	    if runlocal:
		os.system('bash {}'.format(script_name))
	    elif runqsub:
		submitQsubJob( script_name )
