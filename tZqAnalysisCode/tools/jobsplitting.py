###################################################################
# some functions to automatically determine optimal job splitting #
###################################################################

import os
import sys
import ROOT
import numpy as np

def listnentries(inputstruct):
    ### retrieve the number of entries from a collection of root trees
    # inputstruct is a list of dicts containing sample information
    # each dict must have the key 'file' containing the path the root file
    # a new entry 'nentries' will be created with the number of entries
    # the tree name within each file is assumed to be the conventional one...
    for el in inputstruct:
	if 'file' not in el.keys():
	    print('### WARNING ###: key "file" not found for inputstruct entry')
	    continue
	inputfile = el['file']
	nentries = -1
	try:
	    temp = ROOT.TFile(inputfile)
	    nentries = temp.Get("blackJackAndHookers/blackJackAndHookersTree").GetEntries()
	except:
	    print('### WARNING ###: number of entries for {} could not be found'.format(inputfile))
	el['nentries'] = nentries
    # return not needed as modifications are in-place (?) but keep for calling flexibility
    return inputstruct

def sortbynentries(inputstruct):
    ### sort a list of sample info dicts by number of entries per sample
    # inputstruct is a list of dicts containing sample information
    # each dict must have the key 'nentries' containing the number of entries for that sample
    # note that the sorting order is descending, but entries<0 are placed in front
    nentries = []
    for el in inputstruct: nentries.append(el['nentries'])
    nentries = np.array(nentries)
    maxnentries = nentries.max()
    nentries = np.where(nentries<0,maxnentries+1,nentries)
    sorted_indices = np.argsort(nentries)[::-1]
    return [inputstruct[i] for i in sorted_indices]
    
def splitbynentries(inputstruct,nentries=-1):
    ### divide a list of samples into jobs by the number of entries
    # inputstruct is a list of dicts containing sample information
    # each dict must have the key 'file' containing the path the root file
    # the samples will be grouped in a way to make the total number of entries per group
    # as close as possible to nentries
    # note: if nentries<0, it will be set to the maximum number of entries per sample
    # note: if the number of entries for a sample cannot be determined, 
    #       it will always be grouped separately
    
    inputstruct = sortbynentries(listnentries(inputstruct))
    # prints for testing
    #for el in inputstruct: print(el['nentries'])
    if nentries<0:
	# set threshold as function of the maximum number of entries per sample
	# (use the fact that the list is already sorted!)
	for el in inputstruct: 
	    if el['nentries']>0: 
		nentries = el['nentries']/float(3)
		break
    splitlist = []
    currentcounter = 0
    currentlist = []
    for index,el in enumerate(inputstruct):
	# if no interpretable number of entries, add separate sample
	if el['nentries']<0:
	    splitlist.append([el])
	    continue
	# add sample to current set
	currentlist.append(el)
	currentcounter += el['nentries']
	# close the current set when the threshold has been reached
	if( currentcounter>=nentries or index==len(inputstruct)-1 ):
	    splitlist.append([e for e in currentlist])
	    currentlist = []
	    currentcounter = 0
    return splitlist
