########################################################################################
# simple script to automatically create a combined root file for each output directory #
########################################################################################

import os
import sys
import ROOT
sys.path.append('../plotting')
from effplotter import loadobjects,plotefficiencies

if len(sys.argv)<2:
    print('### ERROR ###: mergeoutput.py needs one command line argument.')
    print('		  normal usage: mergeoutput <output_directory> (<skipmerging=False>'
			    +'<skipplotting=False>)')
    sys.exit()

output_directory = sys.argv[1]
skipmerging = False
skipplotting = False
for arg in sys.argv[1:]: # include 1 to obsolete extra length check
    if arg.split('=')[0]=='skipmerging': skipmerging = (arg.split('=')[-1]=='True')
    if arg.split('=')[0]=='skipplotting': skipplotting = (arg.split('=')[-1]=='True')
directories = []

# first determine directories to run on
for root,dirs,files in os.walk(output_directory):
    for dirname in dirs:
	# check if the directory contains at least one root file
	hasrootfile = False
        for filename in os.listdir(os.path.join(root,dirname)):
	    if filename[-5:]=='.root':
		hasrootfile = True
		break
	if hasrootfile: directories.append(os.path.join(root,dirname))

for directory in directories:
    filename = 'combined.root'
    path_to_file = os.path.join(directory,filename)

    # merging
    if not skipmerging:
	if os.path.exists(path_to_file):
	    os.system('rm '+path_to_file)
	# list files in directory and choose what MC to use (tZq or TT)
	datafiles = [f for f in os.listdir(directory) if (f[-5:]=='.root' and 'data' in f)]
	tag = 'TTTo2L2Nu' if '3tightveto' in directory else 'TTZ'
	mcfile = [f for f in os.listdir(directory) if (f[-5:]=='.root' and tag in f)]
	print(mcfile)
	if len(mcfile)!=1:
	    print('### WARNING ###: found unexpected number of MC files in directory '+directory)
	    continue
	command = 'hadd '+path_to_file
	for f in datafiles+mcfile:
	    command += ' '+os.path.join(directory,f)
	os.system(command)

    # plotting
    if not skipplotting:
	print(directory)
	variables = [
		    {'name':'leptonptleading','xaxtitle':r'lepton p_{T}^{leading} (GeV)'},
		    {'name':'leptonptsubleading','xaxtitle':r'lepton p_{T}^{subleading} (GeV)'},
		    {'name':'leptonpttrailing','xaxtitle':r'lepton p_{T}^{trailing} (GeV)'},
		    {'name':'yield','xaxtitle':r'total efficiency'}
		    ]
        mode = 'triggerefficiency_singleyear'
        if 'allyears' in path_to_file: mode='triggerefficiency_allyears'
        for variable in variables:
            try:
                mcgraphlist = loadobjects(path_to_file,mustcontain=['mc',variable['name']+'_eff'])
                datagraphlist = loadobjects(path_to_file,mustcontain=['data',variable['name']+'_eff'])
                if not len(mcgraphlist)==1:
                    print('### ERROR ###: list of MC histograms has unexpected length: '
                            +str(len(mcgraphlist)))
                    sys.exit()
                mcgraph = mcgraphlist[0]
                mchist = loadobjects(path_to_file,mustcontain=['mc',variable['name']+'_tot'])[0]
                datahistlist = loadobjects(path_to_file,mustcontain=['data',variable['name']+'_tot'])
		# make sure the order in datagraphlist and datahistlist corresponds
                datagraphlist.sort(key = lambda x: x.GetTitle())
                datahistlist.sort(key = lambda x: x.GetTitle())
		# set plot properties
                yaxtitle = 'trigger efficiency'
                xaxtitle = variable['xaxtitle']
                lumi = 0.
                if '2016' in path_to_file: lumi = 35900.
                elif '2017' in path_to_file: lumi = 41500.
                elif '2018' in path_to_file: lumi = 59700.
                elif 'allyears' in path_to_file: lumi = 137100.
                figname = os.path.join(directory,filename.rstrip('.root')+'_'
                                                    +variable['name']+'.png')
                plotefficiencies(datagraphlist,mcgraph,datahistlist=datahistlist,mchist=mchist,
				mcsysthist=None,mode=mode,yaxtitle=yaxtitle,xaxtitle=xaxtitle,
				lumi=lumi,outfile=figname)
            except:
                print('### WARNING ###: something went wrong for '+path_to_file+','+variable['name'])
