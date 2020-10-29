############################################################################
# check current directory for .sh.e files and scanning for failed commands #
############################################################################
import os
import sys

def check_start_done(filename,filetext):
    nstarted = filetext.count('###starting###')
    if(nstarted==0):
        print('### WARNING: file '+fname+' contains no valid starting tag.')
        print('             does the process write the correct tags to the error files?')
        return 1
    ndone = filetext.count('###done###')
    if(nstarted==ndone): return 0
    print('found issue in file '+filename+':')
    print('   '+str(nstarted)+' commands were initiated.')
    print('   '+str(ndone)+' seem to have finished normally.')
    return 1

def check_content(filename,filetext,contentlist):
    contains = False
    for content in contentlist:
	if filetext.count(content)>0:
	    contains = True
	    print('found issue in file '+filename+':')
	    print('   file contains the sequence '+content+' which was flagged as problematic.')
    if contains: return 1
    return 0

checkdir = os.getcwd()
deep = False
if len(sys.argv)==2:
    checkdir = sys.argv[1]
    deep = True
else:
    print('### WARNING ###: no directory name provided, scanning only working directory')

files = []
if not deep:
    files = [fname for fname in os.listdir(checkdir) if '.sh.e' in fname]
else:
    for root,drs,fls in os.walk(checkdir):
        for fname in fls:
            if '.sh.e' in fname:
                files.append(os.path.join(root,fname))
print('found '+str(len(files))+' error log files.')
print('start scanning...')

nerror = 0
for fname in files:
    f = open(fname)
    c = f.read()
    f.close()
    nerror += check_start_done(fname,c)
    nerror += check_content(fname,c,['SysError','/var/torque/mom_priv/jobs'])

if(nerror==0):
    print('no problematic files were found by this automated checking!')
    print('(however this does not guarantee everything went completely fine...)')
