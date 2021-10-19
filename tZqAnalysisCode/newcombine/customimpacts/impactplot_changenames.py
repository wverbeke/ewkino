#####################################################
# modify a json object used for making impact plots #
#####################################################
# usage: 
# use this script for impact plots that are supposed to be made public,
# in order to make the names of the nuisance parameters more clearly interpretable.

import sys
import os
import json
import re

def formatprocess( process ):
    fprocess = process.replace('gamma','#gamma').replace('tt','t#bar{t}').replace('ZZH','ZZ / H')
    return fprocess

def replace( orig ):
    # names of systematic uncertainties
    if re.match('prop_bin.+',orig):
	return 'stat. ({})'.format(orig.replace('prop_bin',''))
    if re.match('qcdScalesShapeEnv_.+',orig):
	process = orig.split('_',1)[1]
	process = formatprocess( process )
	return '{} renorm./fact. scale (acceptance)'.format(process)
    if re.match('qcdScalesShapeNorm_.+',orig):
        process = orig.split('_',1)[1]
        process = formatprocess( process )
        return '{} renorm./fact. scales'.format(process)
    if re.match('norm_.+',orig):
	process = orig.split('_',1)[1]
	process = formatprocess( process )
	if process=='nonprompt': process = 'Nonprompt'
	return '{} normalization'.format(process)
    if re.match('lumi_.+',orig): 
	year = orig.split('_',1)[1]
	return 'luminosity ({})'.format(year)
    if re.match('JECAll_.+',orig): 
	jec = orig.split('_',1)[1]
	return 'jet energy corrections ({})'.format(jec)
    if re.match('bTag_shape_.+',orig):
	bt = orig.replace('bTag_shape_','')
	return 'b tagging efficiency ({})'.format(bt)
    if re.match('trigger_.+',orig):
	year = orig.split('_',1)[1]
	return 'trigger efficiency ({})'.format(year)
    if re.match('isrShape_.+',orig):
	process = orig.split('_')[1]
	process = formatprocess( process )
	return '{} intial state radiation'.format(process)
    if orig=='CR_GluonMove': return 'Color-reconnection model'
    if orig=='fsrShape': return 'final state radiation'
    if orig=='prefire': return 'prefire'
    if orig=='lumi': return 'luminosity'
    if orig=='muonIDSyst': return 'muon identification (syst)'
    if orig=='muonIDStat': return 'muon identification (stat)'
    if orig=='electronIDSyst': return 'electron identification (syst)'
    if orig=='electronIDStat': return 'electron identification (stat)'
    if orig=='pdfShapeRMS': return 'parton distribution function'
    # names of POIs
    if orig=='r': return 'mu'
    return orig

if __name__=='__main__':

    if len(sys.argv)==3:
	inputfile = sys.argv[1]
	outputfile = sys.argv[2]
    else:
	raise Exception('ERROR: wrong number of command line arguments')

    # check if input file is a json file and if it exists
    if not inputfile[-5:]=='.json':
	raise Exception('ERROR: input file must be a json file')
    if not os.path.exists(inputfile):
        raise Exception('ERROR: impact json file '+inputfile+' does not seem to exist...')

    # make the output dir if needed
    outputdir = os.path.dirname(outputfile)
    if len(outputdir)>0 and not os.path.exists(outputdir):
	os.makedirs(outputdir)

    # read the json file
    jsonobj = None
    with open(inputfile) as f:
	jsonobj = json.load(f)
    poi = jsonobj['POIs']
    if len(poi)!=1:
	raise Exception('ERROR: found unexpected number of POIs: {}'.format(len(poi)))
    poi = poi[0]

    # change the name of the poi
    oldpoi = poi['name']
    newpoi = replace(poi['name'])
    poi['name'] = replace(newpoi)
    for param in jsonobj['params']:
	param['impact_'+newpoi] = param['impact_'+oldpoi]
	del param['impact_'+oldpoi]
	param[newpoi] = param[oldpoi]
	del param[oldpoi]

    # change the name of the  params
    for param in jsonobj['params']:
	# special case for color reconnection: set down to nominal,
	# down is equal to up by construction but nicer plot if only one variation
	if 'CR_' in param['name']:
	    param['mu'][0] = param['mu'][1]
	param['name'] = replace(param['name'])


    # write the modified json
    with open(outputfile,'w') as f:
	json.dump(jsonobj,f)
    print('{} has been written'.format(outputfile))
