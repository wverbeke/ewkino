###################################################################################################
# simple script to automatically rebin the combined root file for each signal selection directory #
###################################################################################################

### note ###
# this script is deprecated, as its functionality has been included in mergeoutput.py
# no need anymore to run this separately
# but kept in case rebinning is needed without re-merging everything

import os
import sys
import ROOT

if len(sys.argv)!=2:
    print('### ERROR ###: rebinoutput.py needs one command line argument.')
    print('		  normal usage: rebinoutput <output_directory>')
    sys.exit()

output_directory = sys.argv[1]
rootfiles = []

if not os.path.exists('./rebinoutput'):
    print('### ERROR ###: rebinoutput executable does not seem to exist')
    sys.exit()

for root,dirs,files in os.walk(output_directory):
    for fname in files:
	if fname != 'combined.root': continue
	fulldirname = os.path.join(root,fname)
	if not 'signalregion' in fulldirname: continue
	rootfiles.append(fulldirname)

for rootfile in rootfiles:
    outputfile = rootfile[-5:]+'_temp.root'
    if os.path.exists(outputfile):
	os.system('rm '+outputfile)
    command = './rebinoutput '+rootfile+' '+outputfile+' _fineBinnedEventBDT 1 2'
    print(command)
    # do not use job submission here since need to finish command before continuing
    os.system(command)
    # overwrite original file since original histograms are kept in new file anyway
    os.system('mv '+outputfile+' '+rootfile)
