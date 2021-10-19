########################################################
# generate a latex table from an impact plot json file #
########################################################

import sys
import os
import json
from collections import OrderedDict

def impact_json_to_latex_table( jsonfilename ):

    # initialize the table
    table = ''
    table += r'\begin{table}[!hbtp]'+'\n'
    table += r'    \centering'+'\n'
    table += r'    \topcaption{}'+'\n'
    table += r'    \label{tab:}'+'\n'
    table += r'    \begin{tabular}{llc}'+'\n'
    table += r'        \hline'+'\n'
    table += r'        & Systematic uncertainty & Impact (\%) \\'+'\n'
    table += r'        \hline'+'\n'

    # read the json file
    jsonobj = None
    with open(jsonfilename) as f:
        jsonobj = json.load(f)
    poi = jsonobj['POIs']
    if len(poi)!=1:
        raise Exception('ERROR: found unexpected number of POIs: {}'.format(len(poi)))
    poi = poi[0]

    # get the params and their corresponding impacts
    paramlist = []
    paramnames = []
    for param in jsonobj['params']:
	name = param['name']
	cat = ''
	impact = param['impact_{}'.format(poi['name'])]
	# implement rules for ignoring or grouping below:
	if name[:8]=='prop_bin': continue
	elif name[:7]=='JECAll_': 
	    name = 'jet energy corrections'
	    cat = 'Experimental'
	elif name[:3]=='JER': 
	    name = 'jet energy corrections'
	    cat = 'Experimental'
	elif name[:4]=='lumi': 
	    name = 'luminosity'
	    cat = 'Experimental'
	elif name[:8]=='trigger_': 
	    name = 'trigger efficiency'
	    cat = 'Experimental'
	elif name[:10]=='bTag_shape': 
	    name = 'b tagging'
	    cat = 'Experimental'
	elif name[:4]=='Uncl': 
	    name = 'unclustered energy'
	    cat = 'Experimental'
	elif name[:9]=='qcdScales': 
	    name = 'renormalization and factorization scales'
	    cat = 'Theoretical'
	elif name[:6]=='muonID': 
	    name = 'lepton reco and identification'
	    cat = 'Experimental'
	elif name[:10]=='electronID': 
	    name = 'lepton reco and identification'
	    cat = 'Experimental'
	elif name[:12]=='electronReco': 
	    name = 'lepton reco and identification'
	    cat = 'Experimental'
	elif name[:3]=='isr': 
	    name = 'final state radiation'
	    cat = 'Theoretical'
	elif name[:3]=='fsr': 
	    name = 'initial state radiation'
	    cat = 'Theoretical'
	elif name[:3]=='pdf': 
	    name = 'PDF'
	    cat = 'Theoretical'
	elif name[:5]=='norm_': 
	    name = name.split('_',1)[-1] + ' normalization'
	    cat = r'Backgrounds'
	elif name[:3]=='CR_': 
	    name = 'color reconnection and underlying event'
	    cat = 'Theoretical'
	elif name[:2]=='UE': 
	    name = 'color reconnection and underlying event'
	    cat = 'Theoretical'
	elif name=='extrapol_WZ':
	    name = 'WZ extrapolation'
	    cat = r'Backgrounds'
	elif name=='pileup':
	    cat = 'Experimental'
	elif name=='prefire':
	    cat = 'Experimental'
	# add param to dict
	if name in paramnames:
	    idx = paramnames.index(name)
	    paramlist[idx]['impacts'].append(impact)
	    if impact<paramlist[idx]['minimpact']:
		paramlist[idx]['minimpact'] = impact
	    if impact>paramlist[idx]['maximpact']:
		paramlist[idx]['maximpact'] = impact
        else:
	    paramlist.append({'name':name,'cat':cat,'impacts':[impact],
			      'maximpact':impact,'minimpact':impact})
	    paramnames.append(name)

    # sort the total list by category, then by impact
    paramlist.sort( key=lambda x: (x['cat'], -x['maximpact']) )

    # write to the table
    for i,item in enumerate(paramlist):
	param = item['name']
	cat = item['cat']
	impacts = item['impacts']
	maximpact = item['maximpact']
	minimpact = item['minimpact']
	prefix = '          & '
	if(i==0 or cat!=paramlist[i-1]['cat']):
	    nitems = len( list(el for el in paramlist if el['cat']==cat) )
	    prefix = ''
	    if(i>0):
		prefix = r'          \hline'+'\n'
	    catentry = r'\rotatebox[origin=c]{90}{'+str(cat)+'}'
	    prefix +=    r'          \multirow{'+str(nitems)+r'}{*}{'+catentry+'} & '
	if len(impacts)==1:
	    table += prefix + '{} & {:.1f}'.format(param,impacts[0]*100)
	    table += r' \\'+'\n'
	else:
	    table += prefix + '{} & {:.1f}--{:.1f}'.format(param,minimpact*100,maximpact*100)
	    table += r' \\'+'\n'

    # finish the table
    table += r'    \end{tabular}'+'\n'
    table += r'\end{table}'

    return table


if __name__=='__main__':

    jsonfile = sys.argv[1]
    tab = impact_json_to_latex_table( jsonfile )
    print(tab)
