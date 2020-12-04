######################################################
# submitting a single test job to the condor cluster #
######################################################

import sys
import os
sys.path.append(os.path.abspath('..'))
import condorTools as ct

# define the command to be submitted as a job
cmd = 'echo "this is a test job"'

### method 1: manually make an executable script and job description file

# make the executable
with open('test.sh','w') as script:
    script.write('#!/bin/bash\n')
    script.write(cmd+'\n')

# make the job description
ct.makeJobDescription('test','test.sh')

# submit the job
ct.submitCondorJob('test.txt')

### method 2: using more high-level functions

ct.submitCommandAsCondorJob('test',cmd)
# note: can use same name 'test', counters to avoid overwriting are automatically appended
