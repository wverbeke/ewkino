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
            for poi in pois:
                dcresult[poi]['downerror_stat'] = poiresults[poi][1]
		dcresult[poi]['uperror_stat'] = poiresults[poi][2]
        except:
            print('WARNING in readdatacarddir: stat only pois for'
                        +' {} could not be read'.format(dc))
	resdict[name] = dcresult
    return resdict

def calculate_ratio_error( A, B, sigmaA, sigmaB, corrAB ):
    ratio = float(A)/B
    relA = float(sigmaA)/A
    relB = float(sigmaB)/B
    relcov = float(corrAB*sigmaA*sigmaB)/(A*B)
    return ratio*math.sqrt( relA**2 + relB**2 - 2*relcov )
	
if __name__=='__main__':

    ### parse command line args
    # initializations
    perchanneldir = ''
    doperchannel = False
    incldir = ''
    doincl = False
    ratiodir = ''
    directratio = False
    doratio = False
    usedata = False
    args = sys.argv[1:]
    argscopy = args[:]
    # check keyword arguments
    for arg in argscopy:
	if not '=' in arg: continue
	argkey,argval = arg.split('=',1)
	if argkey=='perchanneldir':
	    perchanneldir = argval
	    doperchannel = True
	elif argkey=='incldir': 
	    incldir = argval
	    doincl = True
	elif argkey=='ratiodir':
	    ratiodir = argval 
	    directratio = True
	elif argkey=='doratio':
	    doratio = (argval=='true' or argval=='True')
	elif argkey=='usedata':
	    usedata = (argval=='true' or argval=='True')
	else:
	    raise Exception('ERROR: keyword arg {} not recognized'.format(argkey))
	args.remove(arg)
    if( doratio and (not directratio) and (not doperchannel) ):
	raise Exception('ERROR: incompatible arguments: doratio was requested, ' 
			+'but no direct ratio measurement directory was given, '
			+'nor a per-channel directory from which the ratio can be calculated')
    
    # print some info
    if not doincl:
	print('WARNING: no directory with an inclusive measurement was given,')
	print('         will ignore inclusive measurement')
    if not directratio:
	print('WARNING: no directory with a ratio measurement was given')
	if doratio:
	    print('         will calculate it from per-channel measurements')
	else:
	    print('         will ignore ratio measurement')
    if not usedata:
	print('WARNING: running in blinded mode')

    resdict = {}

    if doperchannel:
	pois = ['r_tZq'+topch for topch in get_topchannel_parameters()[1].values()]
	print('scanning directory {} for pois {}'.format(perchanneldir,pois))
	dircontent = readdatacarddir( perchanneldir, pois, tags=['all'], mode='root', 
					usedata=usedata )
	if len(dircontent)!=1:
	    raise Exception('ERROR length of dict per channel has uncexpected length:'
		    +' {} (while expecting only one datacard to be read)'.format(len(resdict)))
	dircontent = dircontent[dircontent.keys()[0]]
	for poi in dircontent.keys():
	    resdict[poi] = dircontent[poi]

    if doincl:
	print('scanning directory {} for inclusive result'.format(incldir))
	res = opt.read_r( incldir, 'dc_combined_all.txt', usedata=usedata )
	resdict['r_tZq'] = {'r':res[0],'downerror':res[1],'uperror':res[2]}
	res = opt.read_r( incldir, 'dc_combined_all.txt', usedata=usedata, statonly=True )
	resdict['r_tZq']['downerror_stat'] = res[1]
	resdict['r_tZq']['uperror_stat'] = res[2]

    if directratio:

	# method with FitDiagnostics or MultiDimFit with one signal strength
	#print('scanning directory {} for ratio result'.format(ratiodir))
	#res = opt.read_signalstrength( ratiodir, 'dc_combined_all.txt', mode='txt',
	#				usedata=usedata, method='any' )
	#resdict['r_ratio'] = {'r':res[0],'downerror':res[1],'uperror':res[2]}
	#res = opt.read_signalstrength( ratiodir, 'dc_combined_all.txt', mode='txt',
	#				statonly=True, usedata=usedata, method='any' )
        #resdict['r_ratio']['downerror_stat'] = res[1]
        #resdict['r_ratio']['uperror_stat'] = res[2]

	# method with MultiDimFit with multiple signal strengths
	print('scanning directory {} for ratio result'.format(ratiodir))
        totpoidict = opt.read_multisignalstrength( ratiodir, 'dc_combined_all.txt', mode='txt',
						    usedata=usedata, pois=['r_ratio'] )
	if len(totpoidict)!=1:
	    raise Exception('ERROR: found {}'.format(totpoidict))
	res = totpoidict[totpoidict.keys()[0]]
        resdict['r_ratio'] = {'r':res[0],'downerror':res[1],'uperror':res[2]}
        statpoidict = opt.read_multisignalstrength( ratiodir, 'dc_combined_all.txt', mode='txt',
                                       statonly=True, usedata=usedata, pois=['r_ratio'] )
	if len(statpoidict)!=1:
	    raise Exception('ERROR: found {}'.format(statpoidict))
	res = statpoidict[statpoidict.keys()[0]]
        resdict['r_ratio']['downerror_stat'] = res[1]
        resdict['r_ratio']['uperror_stat'] = res[2]	

    elif doratio:
    
	print('calculating ratio from per-channel results')
	# get values, uperrors, downerrors and correlation for total uncertainties
	(totpoidict,totcorrdict) = opt.read_multisignalstrength( perchanneldir, 
				    'dc_combined_all.txt',
				    pois=pois, mode='root', usedata=usedata, correlations=True )
	print('total uncertainties:')
	print(totpoidict)
	print(totcorrdict)
	# get values, uperrors, downerrors and correlation for total uncertainties
	(statpoidict,statcorrdict) = opt.read_multisignalstrength( perchanneldir, 
					'dc_combined_all.txt',
					pois=pois, mode='root', usedata=usedata,correlations=True, 
					 statonly=True )
	print('statistical uncertainties:')
	print(statpoidict)
	print(statcorrdict)

	# calculate ratio
	resdict['r_ratio'] = ({'r':totpoidict['r_tZq_top'][0]/totpoidict['r_tZq_antitop'][0],
			 'uperror':calculate_ratio_error( totpoidict['r_tZq_top'][0], 
							  totpoidict['r_tZq_antitop'][0],
							  totpoidict['r_tZq_top'][2],
							  totpoidict['r_tZq_antitop'][2],
							  totcorrdict['r_tZq_top']['r_tZq_antitop']),
			 'downerror':calculate_ratio_error( totpoidict['r_tZq_top'][0], 
                                                          totpoidict['r_tZq_antitop'][0],
                                                          totpoidict['r_tZq_top'][1],
                                                          totpoidict['r_tZq_antitop'][1],
                                                          totcorrdict['r_tZq_top']['r_tZq_antitop']),
			 'uperror_stat':calculate_ratio_error( statpoidict['r_tZq_top'][0], 
                                                          statpoidict['r_tZq_antitop'][0],
                                                          statpoidict['r_tZq_top'][2],
                                                          statpoidict['r_tZq_antitop'][2],
                                                          statcorrdict['r_tZq_top']['r_tZq_antitop']),
			 'downerror_stat':calculate_ratio_error( statpoidict['r_tZq_top'][0],
                                                          statpoidict['r_tZq_antitop'][0],
                                                          statpoidict['r_tZq_top'][1],
                                                          statpoidict['r_tZq_antitop'][1],
                                                          statcorrdict['r_tZq_top']['r_tZq_antitop'])})
    
    # make the plotting file
    orderlist = ['r_ratio','r_tZq_antitop','r_tZq_top','r_tZq']
    orderlist = [el for el in orderlist if el in resdict.keys()]
    for key,el in resdict.items():
	el['label'] = key.replace('r_','')

    # write file for plotting
    writeplottingfile( resdict, 'topchannels_readoutput_result.txt', orderlist=orderlist )
