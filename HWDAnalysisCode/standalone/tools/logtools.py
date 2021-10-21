#########################################################
# tools for redirecting stdout and stderr to a log file #
#########################################################

import sys
import os

class StdOutRedirector:
    ### helper class to redirect print output to a file
    # use as follows:
    #   stdout = sys.stdout
    #   sys.stdout = StdOutRedirector(<a file name>)
    #   ... <some code execution containing print statements>
    #   sys.stdout = stdout
    # note: both sys.stdout and sys.stderr can refer to the same StdOutRedirector

    def __init__(self, logfile):
	print('creating log file {}'.format(logfile))
        self.logfile = logfile
	if not os.path.exists(os.path.dirname(self.logfile)):
	    os.makedirs(os.path.dirname(self.logfile))
	with open(self.logfile, 'w') as f:
	    pass

    def write(self, text):
        with open(self.logfile, 'a') as f:
	    f.write(text)
	    f.flush()

