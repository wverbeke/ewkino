############################################################################################
# a looper to read the output for eee, eem, emm and mmm channel and make file for plotting #
############################################################################################

import sys
import os
from readoutput import readdatacarddir

def writeplottingfile(reslist,outputfile):
    ### write a txt file that can be used as input for channel compatibility plot
    # - inputs are a list of dicts; each dict should follow the same naming convention
    #   as in readoutput.py but with additional key 'label'
    with open(outputfile,'w') as f:
	for res in reslist:
	    infostr = res['label']+' '+str(res['r'])
	    infostr += ' '+str(res['uperror_stat'])+' '+str(res['downerror_stat'])
	    infostr += ' '+str(res['uperror'])+' '+str(res['downerror'])
	    f.write(infostr+'\n')

if __name__=="__main__":

    if len(sys.argv)!=3:
        print('### ERROR ###: need different number of command line args:')
        print('               <base datacard directory> and <output txt file>')
        sys.exit()
    datacarddir = sys.argv[1]
    outputfile = sys.argv[2]

    channels = ['_ch0','_ch1','_ch2','_ch3','']
    #channels = ['_ch0','_ch1','_ch2','_ch3']
    # following naming convention in runsystematics, i.e. count number of muons
    chtolabel = {'':'combined','_ch0':'eee','_ch1':'eem','_ch2':'emm','_ch3':'mmm'}

    channelresults = []

    for channel in channels:
        thisdatacarddir = datacarddir+channel
        tag = 'signalstrength_exp'
	res = readdatacarddir(thisdatacarddir,tag)
	if res is None: continue
	# (readdatacarddir returns None if dir is not found, skip and continue for testing)
	res = [el for el in res if 'all' in el['card']]
	if len(res)!=1:
	    raise Exception('ambiguous result found for channel {}: ({} candidates)'.format(
				channel,len(res)))
	res = res[0]
	res['label'] = chtolabel[channel]
	channelresults.append(res)

    writeplottingfile(channelresults, outputfile)
