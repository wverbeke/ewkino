#######################################
# merge output files from eventbinner #
#######################################

import sys
import os
sys.path.append('../../Tools/python')
import histtools as ht

def select_files_to_merge(directory, npmode):
    ### select which files to merge, based on type of nonprompt background esitmation
    # input arguments:
    # - directory: a file directory for a specific year and region,
    #              supposed to contain subdirectories '3tight' (for npfromsim)
    #              and '3prompt' and 'fakerate' (for npfrom sim)
    # - npmode: either 'npfromsim' or 'npfromdata'
    if npmode=='npfromsim':
	# return all files in 3tight subdirectory
	subdir = os.path.join(directory,'3tight')
	if not os.path.exists(subdir):
	    raise Exception('ERROR in select_files_to_merge:'
		    +' requested npmode "npfromsim", but this requires a subdirectory "3tight",'
		    +' which does not exist.')
	files = [os.path.join(subdir,f) for f in os.listdir(subdir) if f[-5:]=='.root']
	return files
    elif npmode=='npfromdata':
	# get the files in 3prompt and fakerate subdirectory
	newfilelist = []
	psubdir = os.path.join(directory,'3prompt')
	if not os.path.exists(psubdir):
            raise Exception('ERROR in select_files_to_merge:'
                    +' requested npmode "npfromdata", but this requires a subdirectory "3prompt",'
                    +' which does not exist.')
	pfiles = [os.path.join(psubdir,f) for f in os.listdir(psubdir) if f[-5:]=='.root']
	frsubdir = os.path.join(directory,'fakerate')
	if not os.path.exists(frsubdir):
            raise Exception('ERROR in select_files_to_merge:'
                    +' requested npmode "npfromdata", but this requires a subdirectory "fakerate",'
                    +' which does not exist.')
	frfiles = [os.path.join(frsubdir,f) for f in os.listdir(frsubdir) if f[-5:]=='.root']
	# preliminary implementations
	for f in pfiles: newfilelist.append(f)
	for f in frfiles: newfilelist.append(f)
	return newfilelist
    else:
	raise Exception('ERROR in select_files_to_merge:'
			+' npmode "{}" not recognized'.format(npmode))

if __name__=="__main__":

    directory = sys.argv[1]
    outputfile = sys.argv[2]
    npmode = sys.argv[3]

    # get files to merge
    # note: it is assumed that the naming convention in the files is chosen properly,
    #       such that histograms that should be added have the same name and title;
    #       in this case hadd will automatically merge the right histograms.
    selfiles = select_files_to_merge(directory, npmode)
    
    # printouts
    print('will merge the following files:')
    for f in selfiles: print('  - {}'.format(f))
    print('into {}'.format(outputfile))

    # make output directory
    outputdirname, outputbasename = os.path.split(outputfile)
    if not os.path.exists(outputdirname):
	os.makedirs(outputdirname)

    # do hadd
    cmd = 'hadd -f {}'.format(outputfile)
    for f in selfiles: cmd += ' {}'.format(f)
    os.system(cmd)

    # clip all resulting histograms to minimum zero
    ht.clipallhistograms(outputfile)
