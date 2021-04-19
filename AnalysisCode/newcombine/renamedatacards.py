#############################################################################
# small script to rename some datacards (useful for e.g. grouping channels) #
#############################################################################
# run on a clean datacard directory, containing only elementary datacards and histograms

import sys 
import os

def getnewcards_topchannels(datacards):

    newcards = []
    for card in datacards:
        if( 'signalregion' in card and 'antitop' in card ):
            newcards.append(card.replace('antitop','anti'))
        elif( 'signalregion' in card and 'top' in card ):
            newcards.append(card)
        else:
            newcards.append(card.replace('.txt','_top_anti.txt'))
    return newcards

def getnewcards_lepchannels(datacards):

    newcards = []
    for card in datacards:
        if not ('ch0' in card or 'ch1' in card or 'ch2' in card or 'ch3' in card):
	    newcards.append(card.replace('.txt','_ch0_ch1_ch2_ch3.txt'))
	else:
	    newcards.append(card)
    return newcards

if __name__=='__main__':

    datacarddir = ''
    if len(sys.argv)==2:
	datacarddir = os.path.abspath(sys.argv[1])
    else:
	raise Exception('ERROR: wrong number of command line arguments')

    datacards = sorted([f for f in os.listdir(datacarddir) if f[-4:]=='.txt'])
    newcards = getnewcards_lepchannels(datacards)

    print('propose following renamings:')
    for card,newcard in zip(datacards,newcards):
	print('  - {} --> {}'.format(card,newcard))
    print('continue?')
    go = raw_input()
    if not go=='y': sys.exit()

    for card,newcard in zip(datacards,newcards):
        cmd = 'mv {} {}'.format(os.path.join(datacarddir,card),os.path.join(datacarddir,newcard))
	os.system(cmd)
    print('done')
