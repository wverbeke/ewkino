################################################
# read the output of lepchannels_runcombine.py #
################################################

import sys
import os
sys.path.append(os.path.join('../tools'))
import listtools as lt
import outputparsetools as opt
from lepchannels_makedatacards import get_lepchannel_parameters

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

def readdatacarddir( datacarddir, pois, tags=[], mode='txt', usedata=False ):
    ### read the relevant results from a datacard directory
    # input parameters:
    # - datacarddir: directory to scan
    # - extra tags that the datacards must contain at least one of
    # - pois: list of parameters of interest to look for
    # output: dict mapping datacard name to dict mapping poi to dict containing keys:
    # 'r','uperror','downerror','uperror_stat','downerror_stat'

    if not os.path.exists(datacarddir):
        print('### WARNING ### (in readdatacarddir): datacard dir {} not found'.format(datacarddir))
        print('                returning None object')
        return None

    # make a list of all relevant files, i.e. datacards that have been processed
    cards = ([c for c in os.listdir(datacarddir) if (c[-4:]=='.txt'
                and '_out_' not in c)])
    cards = [c for c in cards if ('signalregion' in c or 'dc_combined' in c)]
    cards = lt.subselect_strings( cards, mustcontainone=tags )[1]
    # loop over datacards
    resdict = {}
    for dc in cards:
	name = dc.replace('datacard_','').replace('dc_combined','').replace('.txt','')
	dcresult = {}
	# read pois with total uncertainties
	try:
	    poiresults = opt.read_multisignalstrength( datacarddir, dc, pois=pois,
			mode=mode, usedata=usedata, statonly=False )
	    for poi in pois: 
		dcresult[poi] = ({'r':poiresults[poi][0],'downerror':poiresults[poi][1],
							 'uperror':poiresults[poi][2],
							 'downerror_stat':0,
							 'uperror_stat':0})
	except:
	    print('WARNING in readdatacarddir: pois for'
                        +' {} could not be read'.format(dc))
	# read pois with statistical uncertainties
	try:
            poiresults = opt.read_multisignalstrength( datacarddir, dc, pois=pois,
                        mode=mode, usedata=usedata, statonly=True )
            print(poiresults)
            for poi in pois:
                dcresult[poi]['downerror_stat'] = poiresults[poi][1]
		dcresult[poi]['uperror_stat'] = poiresults[poi][2]
        except:
            print('WARNING in readdatacarddir: stat only pois for'
                        +' {} could not be read'.format(dc))
	resdict[name] = dcresult
    return resdict
	
if __name__=='__main__':

    perchanneldir = ''
    incldir = ''
    if len(sys.argv)==3:
        perchanneldir = sys.argv[1]
	incldir = sys.argv[2]
    else:
        print('### ERROR ###: wrong number of command line arguments.')
        print('               need <directory per channel>, <directory inclusive>')
    usedata = True

    # read results per channel
    pois = ['r_tZq'+lepch for lepch in get_lepchannel_parameters()[1].values()]
    print('scanning directory {} for pois {}'.format(perchanneldir,pois))
    resdict = readdatacarddir( perchanneldir, pois, tags=['all'], usedata=usedata )
    if len(resdict)!=1:
	raise Exception('length of dict per channel has uncexpected length:'
			+' {} (while expecting only one datacard to be read)'.format(len(resdict)))
    resdict = resdict[resdict.keys()[0]]

    # read inclusive result
    res = opt.read_r( incldir, 'dc_combined_all.txt', usedata=usedata )
    resdict['r_tZq'] = {'r':res[0],'downerror':res[1],'uperror':res[2]}
    res = opt.read_r( incldir, 'dc_combined_all.txt', statonly=True, usedata=usedata )
    resdict['r_tZq']['downerror_stat'] = res[1]
    resdict['r_tZq']['uperror_stat'] = res[2]

    # define a list for sorting
    orderlist = ['r_tZq_eee','r_tZq_eem','r_tZq_emm','r_tZq_mmm','r_tZq']
    # define labels
    for key,el in resdict.items():
	label = key.replace('r_','')
	label = label.replace('m','#mu')
	el['label'] = label

    # write file for plotting
    writeplottingfile( resdict, 'lepchannels_readoutput_result.txt',
			orderlist=orderlist )
