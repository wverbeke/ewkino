##################################
# tools for reading sample lists #
##################################

import sys

def readsamplelist( samplelistpaths, unique=False ):
    ### returns a list of dicts containing the process names, sample names and cross sections.
    # input arguments:
    # - samplelistpaths: either string or list of strings representing path(s) to sample list file(s) 
    # - unique: if set to True, only one entry per sample is returned ignoring version info
    # note: each line of the sample list is assumed to be of the form
    #       process_name sample_name/version_name cross_section
    #       where the individual elements are separated by spaces and should not contain spaces,
    #	    the version_name is optional (defaults to empty string),
    #       and the cross_section is optional (defaults to 0)
    
    # initializations
    samples = []
    unique_samples = []
    unique_sample_names = []
    if isinstance(samplelistpaths,str): samplelistpaths = [samplelistpaths]

    # loop over sample lists
    for samplelist in samplelistpaths:
	with open(samplelist) as f:
	    for line in f:
		# ignore blank or commented lines
		if(line[0] == '#'): continue
		if(len(line)<=1): continue
		# split the line by spaces
		line = line.strip(' ').split(' ')
		# first extract the tag (short name) of the process
		process_name = line[0]
		# now extract sample name (and version name if present)
		sample_name = line[1].rstrip('\n')
		if '/' in sample_name:
		    sample_name = sample_name.split('/')
		    version_name = sample_name[1]
		    sample_name = sample_name[0]
		else: version_name = ''
		# finally extract cross-section
		cross_section = 0.
		if len(line)>2:
		    xsstr = line[2].rstrip('\n')
		    try: cross_section = float(xsstr)
		    except: print('WARNING in readsamplelist: found incompatible cross-section "'
				+xsstr+'". Using zero as default.')
		# fill the lists with a dict for this sample
		samples.append({'process_name':process_name,'sample_name':sample_name,
		            'version_name':version_name,'cross_section':cross_section})
		if not sample_name in unique_sample_names:
		    unique_sample_names.append(sample_name)
		    unique_samples.append({'process_name':process_name,'sample_name':sample_name,
					'cross_section':cross_section})
    if unique: return unique_samples
    return samples
