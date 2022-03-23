####################################
# simple looper for recalchcounter #
####################################

import sys 
import os

if __name__=='__main__':

    inputdir = sys.argv[1]
    years = ['2018']

    for year in years:
	samplelist = '../samplelists/copyfromliam/samples_{}_noskim.txt'.format(year)
	outputfile = 'hcounters_samples_{}'.format(year)
	cmd = 'python recalchcounter.py {} {} {}'.format(inputdir, samplelist, outputfile)
	os.system(cmd)
