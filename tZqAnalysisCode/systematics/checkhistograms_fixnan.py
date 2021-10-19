#####################################################
# small script to fix some nan values in histograms #
#####################################################
# NOT a good approach, use only under time pressure...

import sys
import os
import math
import ROOT
sys.path.append('../tools')
import histtools as ht
sys.path.append('../../jobSubmission')
import condorTools as ct

def fixnan( histfile ):

    print('calling fixnan for file {}'.format(histfile))
    histlist = ht.loadallhistograms( histfile )
    histdict = {}
    for hist in histlist: histdict[hist.GetName()] = hist
    for hist in histlist:
	for i in range(0,hist.GetNbinsX()+2):
	    if math.isnan(hist.GetBinContent(i)):
		hname = hist.GetName()
		# find if this histogram is up or down
		isup = None
		if hname[-2:]=='Up': isup = True
		elif hname[-4:]=='Down': isup = False
		if isup is None:
		    print('WARNING in fixnan: histogram {} is nor up nor down'.format(hname)
			    +', will not replace nan values')
		    continue
		althname = ''
		if isup: althname = hname[:-2]+'Down'
		else: althname = hname[:-4]+'Up'
		althist = histdict[althname]
		if math.isnan(althist.GetBinContent(i)):
		    print('WARNING in fixnan: alternative histogram for {}'.format(hname)
			    +' has nan as well in bin {}'.format(i))
		    continue
		hist.SetBinContent(i,althist.GetBinContent(i))
		hist.SetBinError(i,althist.GetBinError(i))
    tempfilename = histfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
        hist.Write()
    f.Close()
    os.system('mv '+tempfilename+' '+histfile) 
    
if __name__=='__main__':

    topdir = sys.argv[1]

    if topdir[-5:]=='.root':
	fixnan( topdir )

    else:
	commands = []
	for root,dirs,files in os.walk(topdir):
	    for rfile in files:
		if not rfile=='combined.root': continue
		filepath = os.path.abspath(os.path.join(root,rfile))
		print('submitting command for {}'.format(filepath))
		commands.append( 'python checkhistograms_fixnan.py '+filepath )
	ct.submitCommandsAsCondorCluster( 'cjob_checkhistograms_fixnan', commands )
	#for c in commands: os.system(c)

