######################################################################
# run the combine executable for a channel compatibility measurement #
######################################################################
# the input datacards can be made with inclusive_makedatacards.py

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
    # combinations of signal regions per year (+ control regions!)
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

def get_channel_options_years():
    ### get the options to ChannelCompatibility to do a per-year measurement
    # depends on the naming of the channels!
    res = '-g 2016 -g 2017 -g 2018'
    return res

def get_channel_options_topchannels():
    ### get the options to ChannelCompatibility to do a separate top/antitop measurement
    # depends on the naming of the channels!
    res = '-g top -g anti'
    return res

def get_channel_options_lepchannels():
    ### get the options to ChannelCompatibility to do a separate measurement per lepton channel
    # depends on the naming of the channels!
    res = '-g ch0 -g ch1 -g ch2 -g ch3'
    return res


if __name__=="__main__":

    # parse command line args
    if len(sys.argv)<2:
	print('### ERROR ###: need at least one command line arg (input folder)')
	sys.exit()
    datacarddir = os.path.abspath(sys.argv[1])
    usedata = False
    runlocal = False
    runqsub = False
    for arg in sys.argv[2:]:
	if arg=='usedata': usedata = True
	elif arg=='runlocal': runlocal = True
	elif arg=='runqsub': runqsub = True
    runcondor = True
    if( runqsub or runlocal ): runcondor = False 

    dostatonly = True # maybe later add as cmd arg

    # remove all previous output
    cbt.cleandatacarddir(datacarddir)

    # define what channels to consider
    channel_options = get_channel_options_lepchannels()
 
    # run combine command for a number of combination of cards
    if True:
	combinationdict = getcardcombinations(datacarddir,verbose=True)
	combinedcards = cbt.makecombinedcards(datacarddir,combinationdict)
	for card in sorted(combinedcards):
	    print('running combine for '+card)
	    commands = cbt.get_workspace_commands( datacarddir, card )
	    commands += cbt.get_channelcompatibility_commands( 
			datacarddir, card, dostatonly=False, usedata=False,
			channel_options=channel_options )
	    if dostatonly:
		commands += cbt.get_channelcompatibility_commands(
			    datacarddir, card, dostatonly=True, usedata=False,
			    channel_options=channel_options )
	    if usedata:
		commands += cbt.get_channelcompatibility_commands(
			    datacarddir, card, dostatonly=False, usedata=True,
			    channel_options=channel_options )
	    if( usedata and dostatonly ):
		commands += cbt.get_channelcompatibility_commands(
			    datacarddir, card, dostatonly=True, usedata=True,
			    channel_options=channel_options )
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
