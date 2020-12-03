######################################################
# submitting a single test job to the condor cluster #
######################################################

import sys
import os
sys.path.append(os.path.abspath('..'))
import condorTools as ct

# define the command to be submitted as a job
cmd = 'echo "this is a test job"'

# make the executable
with open('test.sh','w') as script:
    script.write('#!/bin/bash\n')
    script.write(cmd+'\n')

# make the job description
ct.makeJobDescription('test','test.sh')

# submit the job
ct.submitCondorJob('test.txt')
