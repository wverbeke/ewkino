######################################################################
# tools for dealing with lists of histogram variables in json format #
######################################################################

import sys
import os
import json

def readvariables( jsonfile ):
    ### read a collection of histogram variables
    # return type: same as encoded in json file,
    #              usually a list of dicts of histogram info.
    with open(jsonfile, 'r') as f:
        variables = json.load(f)
    return variables

def writevariablestxt( variables, txtfile ):
    ### write a collection of variables in plain txt format
    # (more useful than json for reading in c++)
    plaininfo = []
    for el in variables:
	reqkeys = ['name','title','nbins','xlow','xhigh']
	for reqkey in reqkeys:
	    if( reqkey not in el.keys() ):
		msg = 'ERROR in writevariablescc:'
		msg += ' variable does not contain required key {};'.format(reqkey)
		msg += ' found {}'.format(variable)
		raise Exception(msg)
	thisplaininfo = []
	thisplaininfo.append(el['name'])
	thisplaininfo.append(el['title'])
	thisplaininfo.append(el['nbins'])
	thisplaininfo.append(el['xlow'])
	thisplaininfo.append(el['xhigh'])
	plaininfo.append(thisplaininfo)
    with open(txtfile, 'w') as f:
	for line in plaininfo:
	    for el in line:
		f.write(str(el)+'\t')
	    f.write('\n')
