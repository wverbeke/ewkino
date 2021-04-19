############################################################################
# a script for merging systematically varied samples with the nominal ones #
############################################################################

import os
import sys
import ROOT
sys.path.append('../tools')
import histtools as ht

def systematicname( samplename ):
    ### extract a suitable name for a systematic based on samplename
    if 'CR1_QCDbased' in samplename: return 'CR_QCD'
    if 'CR2_GluonMove' in samplename: return 'CR_GluonMove'
    if 'TuneCP5down' in samplename: return 'UEDown'
    if 'TuneCP5up' in samplename: return 'UEUp'
    raise Exception('ERROR in systematicname: samplename {}'.format(samplename)
		    +' could not be associated to a systematic')

def renamehistograms( samplename, samplepath ):
    ### copy a histogram file to a file where the histograms have been renamed to allow merging
    histlist = ht.loadallhistograms(samplepath)
    sysname = systematicname( samplename )
    newhistlist = []
    for hist in histlist:
	# shortcut to allow multiple sequential processings
	if sysname in hist.GetName():
	    newhistlist.append( hist )
	    continue
	# ignore all but nominal histograms
	if not 'nominal' in hist.GetName(): continue
	# rename nominal histogram
	if( sysname[-2:]=='Up' or sysname[-4:]=='Down'):
	    hist.SetName( hist.GetName().replace('nominal',sysname) )
	    newhistlist.append( hist )
	else:
	    uphist = hist.Clone()
	    uphist.SetName( hist.GetName().replace('nominal',sysname+'Up') )
	    downhist = hist.Clone()
            downhist.SetName( hist.GetName().replace('nominal',sysname+'Down') )
            newhistlist.append( uphist )
	    newhistlist.append( downhist )
    # write output file
    tempfilename = samplename.replace('.root','_renamed.root')
    tempfilepath = samplepath.replace('.root','_renamed.root')
    f = ROOT.TFile.Open(tempfilepath,'recreate')
    for hist in newhistlist:
        hist.Write()
    f.Close()
    return (tempfilename,tempfilepath)

if __name__=='__main__':

    sysdir = sys.argv[1]
    nomdir = sys.argv[2]

    years = ['2017','2018']
    regions = ['all']
   
    dynamic_regions = False
    if(len(regions)==1 and regions[0]=='all'): dynamic_regions = True 
    for year in years:
	if dynamic_regions:
	    regions = os.listdir(os.path.join(sysdir,year+'MC'))
	for region in regions:
	    thisdir = os.path.join(sysdir,year+'MC',region)
	    samples = os.listdir(thisdir)
	    for sample in samples:
		print('running on {}'.format(os.path.join(thisdir,sample)))
		# rename the histograms
		(newfilename,newfilepath) = renamehistograms( sample, os.path.join(thisdir,sample) )
		# move the file
		nomdirloc = os.path.join(nomdir,year+'MC',region)
		if not os.path.exists(nomdirloc):
		    raise Exception('ERROR: folder {}'.format(nomdirloc)
			    +' (i.e. copy target location) does not exist...')
		os.system('cp {} {}'.format(newfilepath,os.path.join(nomdirloc,
					newfilename.replace('_renamed',''))))
