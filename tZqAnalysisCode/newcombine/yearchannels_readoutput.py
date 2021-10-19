##############################################################################################
# read the output of inclusive_runcombine.py in such a way to make a year compatibility plot #
##############################################################################################
# notes:
# - use this script on the output of inclusive_runcombine.py,
#   i.e. the output txt files are assumed to be obtained through FitDiagnostics or MultiDimFit
# - no need for a dedicated datacard splitting (as with e.g. lepton or top channels)
#   since the cards are made per year by default
# - the fits for different years are completely independent from each other
# - the result is not necessarily equal to e.g. ChannelCompatibilityCheck, 
#   which performs a simultaneous fit with multiple signal strengths instead of independent fits

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

if __name__=='__main__':

    ### parse command line args
    # initializations
    datacarddir = ''
    if len(sys.argv)==2:
	datacarddir = sys.argv[1]
    usedata = True # maybe later add as command line argument
    
    resdict = {}

    for tag in ['2016','2017','2018','all']:
	print('searching for {} output'.format(tag))
	res = opt.read_r( datacarddir, 'dc_combined_{}.txt'.format(tag), usedata=usedata )
	resdict[tag] = {'r':res[0],'downerror':res[1],'uperror':res[2]}
	res = opt.read_r( datacarddir, 'dc_combined_{}.txt'.format(tag), usedata=usedata, 
			    statonly=True )
	resdict[tag]['downerror_stat'] = res[1]
	resdict[tag]['uperror_stat'] = res[2]

    # make the plotting file
    orderlist = ['2018','2017','2016','all']
    orderlist = [el for el in orderlist if el in resdict.keys()]
    for key,el in resdict.items():
	el['label'] = key
	if key=='all': el['label'] = 'total'

    # write file for plotting
    writeplottingfile( resdict, 'yearchannels_readoutput_result.txt', orderlist=orderlist )
