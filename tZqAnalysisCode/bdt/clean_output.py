##################################################################
# script to read the output log files from the submission script #
##################################################################
import os
import sys
from read_output import getlogfiles,sortbyauc

sdirname = 'configs' # subdirectory to scan for output files
if len(sys.argv)==2:
    sdirname = sys.argv[1]
else: 
    print('Use hard-coded arguments? Beware as this script removes files. (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()

clist = sortbyauc(getlogfiles(sdirname),printout=True)
ftrm = ['out.root','out_all_data']
for el in clist[:-10]:
    thisdir = el[0]
    thisdir = thisdir[:thisdir.rfind('/')]
    for f in ftrm:
	if os.path.exists(os.path.join(thisdir,f)):
	    cmd = 'rm -r '+os.path.join(thisdir,f)
	    print(cmd)
	    os.system(cmd)
print('done')
