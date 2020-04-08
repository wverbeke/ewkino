#################################################################################
# check current directory for .sh.e files and scanning for failed skim commands #
#################################################################################
import os

files = [fname for fname in os.listdir(os.getcwd()) if '.sh.e' in fname]
print('found '+str(len(files))+' error log files.')
print('start scanning...')

nerror = 0
for fname in files:
    f = open(fname)
    c = f.read()
    f.close()
    nstarted = c.count('###starting###')
    if(nstarted==0):
	print('-------------')
	print('### WARNING: file '+fname+' contains no valid starting tag.')
	print('             does the process write the correct tags to the error files?')
	continue
    ndone = c.count('###done###')
    if(nstarted==ndone): continue
    nerror += 1
    print('-------------')
    print('found issue in file '+fname+':')
    print('   '+str(nstarted)+' commands were initiated.')
    print('   '+str(ndone)+' seem to have finished normally.')

if(nerror==0):
    print('no problematic files were found by this automated checking!')
    print('(however this does not guarantee everything went completely fine...)')
