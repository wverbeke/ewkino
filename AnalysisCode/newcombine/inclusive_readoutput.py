#################################################
# read the output of the inclusive measurements #
#################################################

import sys
import os
import math
import outputparsetools as opt

def formatline( info ):
    ### format a line for printing
    # info is a dict (e.g. output of readdatacarddir) containing the following keys:
    # 'card','type',
    # for type 'signalstrength': 'r','uperror','downerror','uperror_stat','downerror_stat'
    # for type 'significance': 'sigma'
    titlelen = 25
    numlen = 8
    res = str('{:<'+str(titlelen)+'}').format( info['card'] )
    res += ':'
    if info['type']=='significance':
	sigma = info['sigma']
        res += str('s = {:<'+str(numlen)+'}').format(str('{:.'+str(numlen-3)+'f}').format(sigma))
    elif info['type']=='signalstrength':
	r = info['r']
	up = info['uperror']
	down = info['downerror']
	upstat = info['uperror_stat']
	downstat = info['downerror_stat']
        res += str('r = {:<'+str(numlen)+'}').format(str('{:.'+str(numlen-3)+'f}').format(r))
        res += '+'
        res += str('{:<'+str(numlen)+'}').format(str('{:.'+str(numlen-3)+'f}').format(up))
        if upstat > 0:
            res += '(stat: '
            res += str('{:<'+str(numlen)+'}').format(str('{:.'+str(numlen-3)+'f}').format(upstat))
            res += ') '
        res += '-'
        res += str('{:<'+str(numlen)+'}').format(str('{:.'+str(numlen-3)+'f}').format(down))
        if downstat > 0:
            res += '(stat: '
            res += str('{:<'+str(numlen)+'}').format(str('{:.'+str(numlen-3)+'f}').format(downstat))
            res += ') '
    res += '\n'
    res += '-'*(titlelen+3*numlen)+'\n'
    return res

def formatlatextableentries( infolist ):
    ### format a list of infos in latex table entries
    # infolist is a list of dicts (e.g. output of readdatacarddir) with the following keys:
    # 'card','type',
    # for type 'signalstrength': 'r','uperror','downerror','uperror_stat','downerror_stat'
    # for type 'significance': 'sigma'
    tableentries = {}
    for info in infolist:
        key = info['card']
        value = ''
        if info['type']=='significance':
            value = '{:.2f}'.format(info['sigma'])
        if info['type']=='signalstrength':
	    r = info['r']
            uperror = info['uperror']
            upstat = info['uperror_stat']
            downerror = info['downerror']
            downstat = info['downerror_stat']
	    if upstat>uperror or downstat>downerror:
                print('WARNING in formatlatextableentries: unphysical uncs for card {},'.format(key))
                print('setting entry to zero')
		r = 0; uperror = 0; downerror = 0; upstat = 0; downstat = 0
            upsys = math.sqrt(uperror**2-upstat**2)
	    downsys = math.sqrt(downerror**2-downstat**2)
            if upstat < 1e-12 or downstat < 1e-12:
                value = '${:.2f}_{{-{:.2f} }}^{{+{:.2f} }}$'.format(r,downerror,uperror)
            else:
                value = '${:.2f}_{{-{:.2f} ({:.2f} / {:.2f}) }}'
                value += '^{{+{:.2f} ({:.2f} / {:.2f}) }}$'
                value = value.format(r,downerror,downsys,downstat,uperror,upsys,upstat)
        tableentries[key] = value
    return tableentries

def get(adict,akey):
    if akey in adict.keys(): return adict[akey]
    else: return '0'

def formatlatextable_obs_and_exp(resdict_obs, resdict_exp):
    ### format a latex table for the inclusive results
    # input arguments:
    # - resdict_obs is a dict matching a (stripped) datacard name to a latex table entry
    # - same for resdict_exp but expected instead of observed
    t = r'\begin{table}[h]'+'\n'
    t += '\t'+r'\begin{center}'+'\n'
    t += '\t\t'+r'\caption{}'+'\n'
    t += '\t\t'+r'\label{}'+'\n'
    t += '\t\t'+r'\begin{tabular}{|l|c|c|c|}'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'channel & 2016 & 2017 & 2018 \\'+'\n'
    t += '\t\t\t'+r'\hline \hline'+'\n'
    t += '\t\t\t'+r'1 b-jet, 2-3 jets & '+'{} ({}) & {} ({}) & {} ({}) '.format(
            get(resdict_obs,'signalregion_cat1_2016'),get(resdict_exp,'signalregion_cat1_2016'),
            get(resdict_obs,'signalregion_cat1_2017'),get(resdict_exp,'signalregion_cat1_2017'),
            get(resdict_obs,'signalregion_cat1_2018'),get(resdict_exp,'signalregion_cat1_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'1 b-jet, $\geq$ 4 jets & '+'{} ({}) & {} ({}) & {} ({}) '.format(
            get(resdict_obs,'signalregion_cat2_2016'),get(resdict_exp,'signalregion_cat2_2016'),
            get(resdict_obs,'signalregion_cat2_2017'),get(resdict_exp,'signalregion_cat2_2017'),
	    get(resdict_obs,'signalregion_cat2_2018'),get(resdict_exp,'signalregion_cat2_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'$\geq$ 2 b-jets & '+'{} ({}) & {} ({}) & {} ({}) '.format(
            get(resdict_obs,'signalregion_cat3_2016'),get(resdict_exp,'signalregion_cat3_2016'),
            get(resdict_obs,'signalregion_cat3_2017'),get(resdict_exp,'signalregion_cat3_2017'),
            get(resdict_obs,'signalregion_cat3_2018'),get(resdict_exp,'signalregion_cat3_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'combination (+ control regions) & '+'{} ({}) & {} ({}) & {} ({}) '.format(
                    get(resdict_obs,'_2016'),get(resdict_exp,'_2016'),
                    get(resdict_obs,'_2017'),get(resdict_exp,'_2017'),
                    get(resdict_obs,'_2018'),get(resdict_exp,'_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'total combination (+ control regions) & \multicolumn{3}{c|}{'+'{} ({})'.format(
                    get(resdict_obs,'_all'),get(resdict_exp,'_all')
         )+r'} \\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t'+r'\end{tabular}'+'\n'
    t += '\t'+r'\end{center}'+'\n'
    t += r'\end{table}'
    return t

def formatlatextable( resdict1, resdict2=None ):
    ### same as above but with only a single dict
    # (combination of both observed and expected in single table is unreadable)
    if resdict2 is not None: return formatlatextable_obs_and_exp( resdict1, resdict2 )
    t = r'\begin{table}[h]'+'\n'
    t += '\t'+r'\begin{center}'+'\n'
    t += '\t\t'+r'\caption{}'+'\n'
    t += '\t\t'+r'\label{}'+'\n'
    t += '\t\t'+r'\begin{tabular}{|l|c|c|c|}'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'channel & 2016 & 2017 & 2018 \\'+'\n'
    t += '\t\t\t'+r'\hline \hline'+'\n'
    t += '\t\t\t'+r'1 b-jet, 2-3 jets & '+'{} & {} & {}'.format(
            get(resdict1,'signalregion_cat1_2016'),
            get(resdict1,'signalregion_cat1_2017'),
            get(resdict1,'signalregion_cat1_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'1 b-jet, $\geq$ 4 jets & '+'{} & {} & {} '.format(
            get(resdict1,'signalregion_cat2_2016'),
            get(resdict1,'signalregion_cat2_2017'),
            get(resdict1,'signalregion_cat2_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'$\geq$ 2 b-jets & '+'{} & {} & {}'.format(
            get(resdict1,'signalregion_cat3_2016'),
            get(resdict1,'signalregion_cat3_2017'),
            get(resdict1,'signalregion_cat3_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'combination (+ control regions) & '+'{} & {} & {}'.format(
                    get(resdict1,'_2016'),
                    get(resdict1,'_2017'),
                    get(resdict1,'_2018')
         )+r'\\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'total combination (+ control regions) & \multicolumn{3}{c|}{'+'{}'.format(
                    get(resdict1,'_all')
         )+r'} \\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t'+r'\end{tabular}'+'\n'
    t += '\t'+r'\end{center}'+'\n'
    t += r'\end{table}'
    return t

def readdatacarddir( datacarddir, target='significance', usedata=False, method='fitDiagnostics' ):
    ### read an entire datacarddir and return the result
    # input arguements:
    # - datacarddir: directory to scan
    # - target: type of output to look for ('significance' or 'signalstrength')
    # - usedata: whether to read observed or expected results
    # - method: used method for signal strength extraction ('fitDiagnostics' or 'multiDimFit')
    # the output object is a list of dicts; each dict has the following keys:
    #   'card','type',
    #   for type 'signalstrength': 'r','uperror','downerror','uperror_stat','downerror_stat'
    #   for type 'significance': 'sigma'

    if not os.path.exists(datacarddir):
        print('### WARNING ### (in readdatacarddir): datacard dir {} not found'.format(datacarddir))
        print('                returning None object')
        return None

    # make a list of all relevant files, i.e. datacards that have been processed
    cards = ([c for c in os.listdir(datacarddir) if (c[-4:]=='.txt' 
		and '_out_' not in c)])
    cards = [c for c in cards if ('signalregion' in c or 'dc_combined' in c)]
    cards = sorted(cards)
    reslist = []
    for card in cards:
        name = card.replace('datacard_','').replace('dc_combined','').replace('.txt','')
	if target=='significance':
	    # read significance
	    sigma = opt.read_sigma( datacarddir, card, usedata=usedata )
	    reslist.append({'card':name,'type':'significance','sigma':sigma})
        elif target=='signalstrength':
	    (r,downerror,uperror) = opt.read_signalstrength( datacarddir, card, 
					    statonly=False, usedata=usedata, method=method )
	    reslist.append({'card':name,'type':'signalstrength',
                                'r':r,'uperror':uperror,'downerror':downerror,
                                'uperror_stat':0,'downerror_stat':0})
	    (rstat,downerrorstat,uperrorstat) = opt.read_signalstrength( datacarddir, card, 
						    statonly=True, usedata=usedata, method=method)
	    reslist[-1]['uperror_stat'] = uperrorstat
	    reslist[-1]['downerror_stat'] = downerrorstat
            if abs(r-rstat) > 1e-3:
                print('WARNING in readdatacarddir: best fit signal strength does not agree.')
	    # even small differences in r vs rstat can cause issues,
	    # e.g. if r = 0.1 -0.1+0.17 and rstat = 0.11 -0.11+0.13
	    # so modify here
	    if r-downerror<1e-12: 
		reslist[-1]['downerror_stat'] = downerror
    return reslist

if __name__=='__main__':
    
    datacarddir = ''
    if len(sys.argv)==2:
        datacarddir = sys.argv[1]
    else:
        print('### ERROR ###: need one command line argument.')
        print('               normal use: python inclusive_readoutput.py <datacard directory>')

    dosignificance = True
    dosignalstrength = True
    usedata = True
    method = 'any'
    
    if dosignificance:
	print('### expected results for significance ###')
	s_exp_list = readdatacarddir(datacarddir,target='significance',usedata=False)
	for s_exp in s_exp_list: print(formatline( s_exp ) )
	s_exp_dict = formatlatextableentries( s_exp_list )
	s_obs_dict = {}
	if usedata:
	    print('### observed results for significance ###')
	    s_obs_list = readdatacarddir(datacarddir,target='significance',usedata=True)
	    for s_obs in s_obs_list: print(formatline( s_obs ) )
	    s_obs_dict = formatlatextableentries( s_obs_list )
	print('### results for significance in table format ###')
	print( formatlatextable(s_obs_dict,s_exp_dict) )

    if dosignalstrength:
	ss_exp_dict = {}
	print('### expected results for signalstrength ###')
        ss_exp_list = readdatacarddir(datacarddir,target='signalstrength',
			usedata=False,method=method)
        for ss_exp in ss_exp_list: print(formatline( ss_exp ) )
        ss_exp_dict = formatlatextableentries( ss_exp_list )
	print( formatlatextable(ss_exp_dict))
        ss_obs_dict = {}
        if usedata:
	    print('### observed results for signalstrength ###')
	    print('reading observed results...')
            ss_obs_list = readdatacarddir(datacarddir,target='signalstrength',
			    usedata=True,method=method)
            for ss_obs in ss_obs_list: print(formatline( ss_obs ) )
            ss_obs_dict = formatlatextableentries( ss_obs_list )
	    print( formatlatextable(ss_obs_dict) )
	
