#################################################################
# some tools to read and parse output from combine calculations #
#################################################################
# note: most methods so far are based on reading the output txt files,
#       possibly extend to reading output root files later on...

import os
import sys
import math
import ROOT

def isnumber( string ):
    try:
	fl = float(string)
	return True
    except:
	return False

##### tools for reading single signal strength (output from FitDiagnostics) 

def read_r_from_txt( filepath ):
    ### read signal strength (r) from the output of a combine command directed to a file
    # based on the presence of 'Best fit r: x  -x/+x (68% CL)' in the output file
    # (works for the output of FitDiagnostics)
    # output: tuple of the form (central,downerror,uperror)
    
    # check if file exists
    if not os.path.exists(filepath):
	raise Exception('ERROR in read_r_from_txt: file {} does not seem to exist'.format(filepath))
    # initializations
    r = 0.
    downerror = 0.
    uperror = 0.
    nvalid = 0
    # open file and read lines
    with open(filepath,'r') as f:
        for l in f.readlines():
            if l[:11]=='Best fit r:':
                l = l.replace('Best fit r: ','').replace(' (68% CL)','')
                l = l.split(' ')
                r = l[0]
                uperror = l[2].split('/')[1].strip('+')
                downerror = l[2].split('/')[0].strip('-')
		if not (isnumber(r) and isnumber(uperror) and isnumber(downerror)): continue
		nvalid += 1
    if nvalid != 1:
	raise Exception('ERROR in read_r_from_txt: found {} valid entries'.format(nvalid)
			    +' in file {}'.format(filepath))
    return (float(r),float(downerror),float(uperror))

def read_r( datacarddir, card, statonly=False, usedata=False, mode='txt', 
	    interpendix='_out_fitdiagnostics', exp='_exp', obs='_obs', stat='_stat' ):
    ### read signal strength (r) for a given datacard
    # - mode is either 'txt' (to read from txt file) or 'root' (to read from root file)
    # - interpendix, exp, obs and stat are used to specify the name of the file to be read
    # output: tuples of the form (central, downerror, uperror)

    # define name of output file from input arguments
    if usedata: interpendix += obs
    else: interpendix += exp
    if statonly: interpendix += stat
    resultfile = card.replace('.txt',interpendix)

    # read output file
    if mode=='txt':
	txtfile = os.path.join(datacarddir,resultfile+'.txt')
	# temp for backward compatibility: allow 'signalstrength' instead of 'fitdiagnostics'
	if not os.path.exists(txtfile):
	    txtfile = txtfile.replace('_out_fitdiagnostics','_out_signalstrength')
        return read_r_from_txt( txtfile )
    elif mode=='root':
	raise Exception('ERROR in read_r: mode root not yet implemented')
        return read_r_from_root( os.path.join(datacarddir,'fitDiagnostics'+resultfile+'.root') )
    else: raise Exception('ERROR in read_r: mode {} not recognized'.format(mode))

##### tools for reading single signal strength that work with both FitDiagnostics and MultiDimFit

def read_signalstrength( datacarddir, card, statonly=False, usedata=False, mode='txt',
			interpendix=None, exp='_exp', obs='_obs', stat='_stat',
			method='any' ):
    ### read the signal strength (r) for a given datacard
    # - mode is either 'txt' (to read from txt file) or 'root' (to read from root file)
    #   note: mode root not yet implemented for method fitDiagnostics!
    # - interpendix, exp, obs and stat are used to specify the name of the file to be read
    #   (if interpendix is None, it will be set to _out_fitdiagnostics or _out_multidimfit)
    # - method is either 'FitDiagnostics' or 'MultiDimFit'
    #   it can also be 'any', meaning that MultiDimFit will be tried if FitDiagnostics fails.
    # output: tuple of (central, downerror, uperror)

    if method not in ['any','MultiDimFit','FitDiagnostics']: 
	raise Exception('ERROR in read_r: method {} not recognized'.format(method))

    if method=='MultiDimFit':
	if interpendix is None: interpendix = '_out_multidimfit'
	poidict = read_multisignalstrength(datacarddir, card, statonly=statonly,
		    usedata=usedata, pois=['r'], mode=mode,
		    interpendix=interpendix, exp=exp, obs=obs, stat=stat)
	if not (len(poidict.keys())==1 and poidict.keys()[0]=='r'):
	    raise Exception('ERROR in read_signalstrengh with method MultiDimFit:'
			    +' found following pois (while expecting only 'r'):'
			    +str(poidict.keys()))
	return poidict['r']
    
    elif method=='FitDiagnostics':
	if interpendix is None: interpendix = '_out_fitdiagnostics'
	res = read_r(datacarddir, card, statonly=statonly, usedata=usedata, mode=mode,
			interpendix=interpendix, exp=exp, obs=obs, stat=stat)
	return res

    elif method=='any':
	try:
	    res = read_signalstrength(datacarddir, card, statonly=statonly,
                    usedata=usedata, mode=mode,
                    interpendix=interpendix, exp=exp, obs=obs, stat=stat,
		    method='FitDiagnostics')
	    return res
	except:
	    try:
		res = read_signalstrength(datacarddir, card, statonly=statonly,
			usedata=usedata, mode=mode,
			interpendix=interpendix, exp=exp, obs=obs, stat=stat,
			method='MultiDimFit')
		return res
	    except:
		raise Exception('ERROR in read_signalstrength: none of the methods succeeded')

##### tools for reading significance (output from Significance)

def read_sigma_from_txt( filename ):
    ### read significance (sigma) from the output of a combine command directed to a file
    # based on the presence of 'Significance: x' in the output file
    # (works for the output of Significance)
    # output: float (significance)
    s = 0.
    nvalid = 0
    with open(filename,'r') as f:
        for l in f.readlines():
            if l[:13]=='Significance:':
                s = l.replace('Significance:','').strip(' ')
		nvalid += 1
    if nvalid != 1:
        raise Exception('ERROR in read_sigma_from_txt: found {} valid entries'.format(nvalid))
    return float(s)

def read_sigma( datacarddir, card, usedata=False, mode='txt',
		interpendix='_out_significance', exp='_exp', obs='_obs' ):
    ### read significance (sigma) for a given datacard
    # - mode is either 'txt' (to read from txt file) or 'root' (to read from root file)
    #   note: mode root not yet implemented!
    # - interpendix, exp and obs are used to specify the name of the file to read
    # output: float (significance)
    
    # define name of output file based on input arguments
    if usedata: interpendix += obs
    else: interpendix += exp
    resultfilename = card.replace('.txt',interpendix)
    # read output file
    if mode=='txt': return read_sigma_from_txt( os.path.join(datacarddir,resultfilename+'.txt') )
    else: raise Exception('ERROR in read_sigma: mode {} not recognized'.format(mode))

##### tools for reading multiple signal strengths (output from MultiDimFit)

def read_multisignalstrength_from_txt( filename, pois=['r'] ):
    ### read multiple signal strengths from the output of a combine command directed to a file
    # based on the presence of lines of the form '  <poi> :   x  -x/+x (68%)'
    # output: dict of poi names to tuples of the form (central, downerror, uperror)
    poidict = {}
    with open(filename,'r') as f:
        for l in f.readlines():
	    l = l.strip(' ')
            if( '(68%)' in l and len(l.split(' '))==10 ):
                l = l.split(' ')
		poi = l[0]
		if not poi in pois: continue
                r = l[5].strip('+')
                uperror = l[8].split('/')[1].strip('+')
                downerror = l[8].split('/')[0].strip('-')
		poidict[poi] = (float(r),float(downerror),float(uperror))
    return poidict

def read_multisignalstrength_from_root( rootfile, pois=['r'], correlations=False ):
    ### read output POIs from a multidimfit root file
    # output: dict of poi names to tuples of (central, downerror, uperror)
    # if correlations is True, a correlation table (2D dict) is returned as well
    if not os.path.exists( rootfile ):
        raise Exception('ERROR in read_multisignalstrength_from_root:'
                        + ' file {} does not seem to exist'.format(rootfile))
    resultfile = ROOT.TFile( rootfile, 'read' )
    if not 'fit_mdf' in [str(key.GetName()) for key in resultfile.GetListOfKeys()]:
	raise Exception('ERROR in read_multisignalstrength_from_root:'
			+ ' file {} dos not seem to contain required object'
			+ ' "fit_mdf"'.format(resultfile) )
    fitresult = resultfile.Get( 'fit_mdf' )
    poidict = {}
    for poi in pois:
        poiobj = fitresult.floatParsFinal().find(poi)
        poidict[poi] = (poiobj.getValV(),abs(poiobj.getErrorLo()),
					 abs(poiobj.getErrorHi()))
    if not correlations: return poidict
    corrdict = {}
    for poi1 in pois: 
	corrdict[poi1] = {}
	for poi2 in pois: 
	    corrdict[poi1][poi2] = fitresult.correlation( poi1, poi2 )
    return (poidict,corrdict)

def read_multisignalstrength( datacarddir, card, statonly=False, 
				usedata=False, pois=['r'], mode='txt',
				correlations=False, interpendix='_out_multidimfit', 
				exp='_exp', obs='_obs', stat='_stat' ):
    ### read multiple signal strengths for a given datacard
    # - mode is either 'txt' (to read from txt file) or 'root' (to read from root file)
    # - correlations: also return correlations between pois (only for mode='root')
    # - interpendix, exp, obs, and stat are used to specify the name of the file to read
    # output: dict of poi names to tuples of (central, downerror, uperror)
    # if correlations is True, a dict of dict of correlations is returned as well
    
    if( correlations and mode=='txt' ):
	raise Exception('ERROR in read_multisignalstrength: correlations can only'
			+' be read in mode=="root", not "txt"')

    # define name of output file from input arguments
    if usedata: interpendix += obs
    else: interpendix += exp
    if statonly: interpendix += stat
    resultfile = card.replace('.txt',interpendix)
    
    # read output file
    if mode=='txt': 
	txtfile = os.path.join(datacarddir,resultfile+'.txt')
        # temp for backward compatibility: allow 'signalstrength' instead of 'multidimfit'
        if not os.path.exists(txtfile):
            txtfile = txtfile.replace('_out_multidimfit','_out_signalstrength')
	return read_multisignalstrength_from_txt( txtfile, pois=pois )
    elif mode=='root':
	return read_multisignalstrength_from_root( os.path.join(datacarddir,
		    'multidimfit'+resultfile+'.root'),pois=pois, 
		    correlations=correlations )
    else: raise Exception('ERROR in read_multisignalstrength: mode {} not recognized'.format(mode))

### tools for reading output of channel compatibility check

def read_channelcompatibility_from_txt( filename ):
    chdict = {}
    with open(filename,'r') as f:
        for l in f.readlines():
            l = l.strip(' ')
	    l = l.strip('\n')
	    if( 'Nominal fit' in l and len(l.split(' '))==10):
		l = l.split(' ')
                r = l[7].strip('+')
                uperror = l[9].split('/')[1].strip('+')
                downerror = l[9].split('/')[0].strip('-')
                chdict['total'] = (float(r),float(downerror),float(uperror))
            if( 'Alternate fit' in l and len(l.split(' '))==13 ):
                l = l.split(' ')
                r = l[5].strip('+')
                uperror = l[7].split('/')[1].strip('+')
                downerror = l[7].split('/')[0].strip('-')
		channel = l[-1]
                chdict[channel] = (float(r),float(downerror),float(uperror))
    return chdict
