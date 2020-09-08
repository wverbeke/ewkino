import os
import sys

def cleandatacarddir(datacarddir,rmtxt=True,rmroot=True):
    ### remove all objects from datacarddir except original datacards and histograms
    if rmtxt:
        alltxtfiles = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']
        for f in alltxtfiles:
            if '_out.txt' in f: os.system('rm '+os.path.join(datacarddir,f))
            if 'combined' in f: os.system('rm '+os.path.join(datacarddir,f))
    if rmroot:
        allrootfiles = [f for f in os.listdir(datacarddir) if f[-5:]=='.root']
        for f in allrootfiles:
            if not f[:11]=='histograms_': os.system('rm '+os.path.join(datacarddir,f))
    allotherfiles = [f for f in os.listdir(datacarddir) if not (f[-5:]=='.root' or f[-4:]=='.txt')]
    for f in allotherfiles: os.system('rm -r '+os.path.join(datacarddir,f))


if __name__=='__main__':

    if len(sys.argv)<2:
	print('### ERROR ###: no directory provided as command line argument')
	print('               usage: python cleandatacarddir.py <directory> [rmtxt] [rmroot]')
	sys.exit()

    datacarddir = sys.argv[1]
    if not os.path.exists(datacarddir):
	print('### ERROR ###: directory does not seem to exist...')
	sys.exit()

    rmtxt = True
    rmroot = True
    if len(sys.argv)==4:
	rmtxt = (sys.argv[2]=='True' or sys.argv[2]=='true')
	rmroot = (sys.argv[3]=='True' or sys.argv[3]=='true')

    cleandatacarddir(datacarddir,rmtxt=rmtxt,rmroot=rmroot)

    # extra: also remove runcombine.sh* log files in current directory
    os.system('rm runcombine.sh*')
