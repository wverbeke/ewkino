################################################
# read the output of topchannels_runcombine.py #
################################################

import sys
import os
import math
sys.path.append(os.path.join('../tools'))
import listtools as lt
import outputparsetools as opt
from topchannels_makedatacards import get_topchannel_parameters

def writeplottingfile( resdict, outputfile, orderlist=None ):
    ### write a txt file that can be used as input for channel compatibility plot
    # - input resdict is a dict matching poi to values (see readdatacarddir)
    #   with additional key 'label' (if no 'label', the dict key is used)
    keylist = resdict.keys()
    if orderlist is not None:
	if( sorted(orderlist) != sorted(resdict.keys()) ):
	    raise Exception('ERROR in writeplottingfile: orderlist and dict keys do not agree')
	keylist = orderlist
    with open(outputfile,'w') as f:
        for label in keylist:
	    res = resdict[label]
            if 'label' in res.keys(): label = res['label']
	    infostr = label+' '+str(res['r'])
            infostr += ' '+str(res['uperror_stat'])+' '+str(res['downerror_stat'])
            infostr += ' '+str(res['uperror'])+' '+str(res['downerror'])
            f.write(infostr+'\n')

def readdatacardres( datacarddir, card, mode='txt', usedata=False ):

    if mode != 'txt':
	raise Exception('not yet implemented')

    name = card.replace('datacard_','').replace('dc_combined','').replace('.txt','')
    dcresult = {}
    txtfilename = card.replace('.txt','_out_channelcompat')
    if usedata: txtfilename += '_obs.txt'
    else: txtfilename += '_exp.txt'
    stattxtfilename = txtfilename.replace('.txt','_stat.txt')
    txtfile = os.path.join(datacarddir,txtfilename)
    stattxtfile = os.path.join(datacarddir,stattxtfilename)
    # read pois with total uncertainties
    try:
	chresults = opt.read_channelcompatibility_from_txt( txtfile )
	for ch in chresults.keys(): 
	    dcresult[ch] = ({'r':chresults[ch][0],'downerror':chresults[ch][1],
						    'uperror':chresults[ch][2],
						    'downerror_stat':0,
						    'uperror_stat':0})
    except:
	print('WARNING in readdatacardres: channels for'
                +' {} could not be read'.format(card))
    # read pois with statistical uncertainties
    try:
        chresults = opt.read_channelcompatibility_from_txt( stattxtfile )
        for ch in chresults:
            dcresult[ch]['downerror_stat'] = chresults[ch][1]
	    dcresult[ch]['uperror_stat'] = chresults[ch][2]
    except:
        print('WARNING in readdatacardres: stat only channels for'
                +' {} could not be read'.format(card))
    return dcresult
	
if __name__=='__main__':

    ### parse command line args
    # initializations
    usedata = False
    args = sys.argv[1:]
    argscopy = args[:]
    # check keyword arguments
    for arg in argscopy:
	if not '=' in arg: continue
	argkey,argval = arg.split('=',1)
	if argkey=='usedata': usedata = (argval=='true' or argval=='True')
	else:
	    raise Exception('ERROR: keyword arg {} not recognized'.format(argkey))
	args.remove(arg)
    # check other arguments
    datacardpath = ''
    if len(args)==1:
	datacardpath = os.path.abspath(args[0])
    else:
	raise Exception('ERROR: wrong number of command line arguments')
    (datacarddir,card) = os.path.split(datacardpath)
    
    resdict = readdatacardres( datacarddir, card, usedata=usedata )

    # make the plotting file
    #orderlist = ['2018','2017','2016','total']
    #orderlist = ['anti','top','total']
    orderlist = ['ch0','ch1','ch2','ch3','total']
    orderlist = [el for el in orderlist if el in resdict.keys()]
    for key,el in resdict.items():
	label = key.replace('r_','')
	if label=='anti': label = 'antitop'
	elif label=='ch0': label = 'eee'
	elif label=='ch1': label = 'eem'
	elif label=='ch2': label = 'emm'
	elif label=='ch3': label = 'mmm'
	el['label'] = label

    # write file for plotting
    writeplottingfile( resdict, 'channelcompat_readoutput_result.txt', orderlist=orderlist )
