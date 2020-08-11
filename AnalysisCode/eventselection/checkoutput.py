#################################################################################
# check current directory for .sh.e files and scanning for failed skim commands #
#################################################################################
import os

def check_done(filename,filetext):
    ndone = filetext.count('done')
    if(ndone==1): return 0
    print('found issue in file '+filename+':')
    print('   finishing command "done" was not written')
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

files = [fname for fname in os.listdir(os.getcwd()) if '.sh.e' in fname]
print('found '+str(len(files))+' error log files.')
print('start scanning...')

nerror = 0
for fname in files:
    f = open(fname)
    c = f.read()
    f.close()
    nerror += check_content(fname,c,['SysError'])

files = [fname for fname in os.listdir(os.getcwd()) if '.sh.o' in fname]
for fname in files:
    f = open(fname)
    c = f.read()
    f.close()
    nerror += check_done(fname,c)
    nerror += check_content(fname,c,['SysError'])

if(nerror==0):
    print('no problematic files were found by this automated checking!')
    print('(however this does not guarantee everything went completely fine...)')
