#####################################################################################
# a Python script to merge the output of runsystematics.py, basically advanced hadd #
#####################################################################################

# this script performs several tasks
# - merging the correct root files, depending on the year, region and nonprompt mode
# - clipping al histograms to minimum zero
# - perform rebinning of the discriminating variable
# - copying all systematic histograms with year suffix to systematic for (de)correlating

import os
import sys
import ROOT
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist
from extendsamplelist import extendsamplelist
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls
from correlations import implementcorrelations

def clipallhistograms(histfile):
    ### read a root file and clip all histograms in it to minimum zero
    histlist = tls.loadallhistograms(histfile)
    for hist in histlist:
	for i in range(0,hist.GetNbinsX()+2):
	    if hist.GetBinContent(i)<0:
		hist.SetBinContent(i,0)
		hist.SetBinError(i,0)
    tempfilename = histfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
	hist.Write()
    f.Close()
    os.system('mv '+tempfilename+' '+histfile)

def subselect_by_tag(filedirectory,samplelist,tagstodiscard=[],tagstokeep=[]):
    # make list of all tags
    filedict = extendsamplelist(samplelist,filedirectory)
    tags = []
    for f in filedict: 
	if not f['process_name'] in tags: tags.append(f['process_name'])
    # keep only selected tags
    tags = tls.subselect(tags,tagstodiscard,tagstokeep)
    # make list of files to keep
    filelist = []
    for f in filedict:
	if f['process_name'] in tags: filelist.append(f['file'])
    return filelist

def printfilestomerge(mcfiles,datafiles):
    ### for debugging or checking
    
    print('--- merging files: ---')
    print('  - MC files -')
    for f in mcfiles:
	print('    '+f)
    print('  - data files -')
    for f in datafiles:
	print('    '+f)

def getfilestomerge(topdir,year,region,npmode):
    ### get a list of files to merge for this specific year, region and nonprompt mode
    mcpath = os.path.join(topdir,year+'MC',region)
    mcsamplelist = os.path.abspath('../samplelists/samplelist_tzq_'+year+'_MC.txt')
    datapath = os.path.join(topdir,year+'data',region)
    datasamplelist = os.path.abspath('../samplelists/samplelist_tzq_'+year+'_data.txt')
    # check if the necessary paths exist
    if npmode=='npfromsim': 
	if( not (os.path.exists(mcpath+'_3tight') 
		and os.path.exists(datapath+'_3tight') )): 
	    print('### ERROR ###: necessary folders do not exist for '+year+' '+region+' '+npmode)
	    return []
    elif npmode=='npfromdata':
	if( not (os.path.exists(mcpath+'_3prompt')
		and os.path.exists(mcpath+'_fakerate')
		and os.path.exists(datapath+'_3tight')
		and os.path.exists(datapath+'_fakerate') )): 
	    print('### ERROR ###: necessary folders do not exist for '+year+' '+region+' '+npmode)
	    return []
    else:
	print('### ERROR ###: npmode "'+npmode+'" not recognized')
	return []
    # intialize list of input files
    selmcfiles = []
    seldatafiles = []
    selnpdatafiles = []
    selpcorrectionfiles = []

    # select data files
    alldatafiles = [os.path.join(datapath+'_3tight',f) for f in os.listdir(datapath+'_3tight') 
			if f[-5:]=='.root']
    tokeep = ['data_combined']
    seldatafiles = tls.subselect(alldatafiles,tagstokeep=tokeep)	
    # select mc files
    if npmode=='npfromsim':
	# use all files with regular 3-tight selection (ZGToLL is assumed to be excluded)
	allmcfiles = extendsamplelist(mcsamplelist,mcpath+'_3tight')
	selmcfiles = tls.subselect_inputfiles(allmcfiles,'3tight')
	selmcfiles = [f['file'] for f in selmcfiles]
    elif npmode=='npfromdata':
	# use prompt files with 3-prompt selection
	allmcfiles = extendsamplelist(mcsamplelist,mcpath+'_3prompt')
	selmcfiles = tls.subselect_inputfiles(allmcfiles,'3prompt')
	selmcfiles = [f['file'] for f in selmcfiles]
	allnpdatafiles = [os.path.join(datapath+'_fakerate',f) 
			    for f in os.listdir(datapath+'_fakerate')
			    if f[-5:]=='.root']
	tokeep = ['data_combined']
	selnpdatafiles = tls.subselect(allnpdatafiles,tagstokeep=tokeep)
	allpcorrectionfiles = extendsamplelist(mcsamplelist,mcpath+'_fakerate')
	selpcorrectionfiles = tls.subselect_inputfiles(allpcorrectionfiles,'fakerate')
	selpcorrectionfiles = [f['file'] for f in selpcorrectionfiles]

    printfilestomerge(selmcfiles+selpcorrectionfiles,seldatafiles+selnpdatafiles)
    return selmcfiles+selpcorrectionfiles+seldatafiles+selnpdatafiles

if __name__=='__main__':

    if len(sys.argv)==2:
	topdir = sys.argv[1]
    else:
	print('### ERROR ###: wrong number of command line arguments.')
	print('               need <topdir>')
	sys.exit()

    regionlist = []
    for r in ['signalregion_1','signalregion_2','signalregion_3']: regionlist.append(r)
    for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regionlist.append(r)
    yearlist = []
    yearlist.append('2016')
    yearlist.append('2017')
    yearlist.append('2018')
    npmodelist = []
    npmodelist.append('npfromsim')
    npmodelist.append('npfromdata')

    for year in yearlist:
	for region in regionlist:
	    for npmode in npmodelist:
		print('#####')
		print('merging all files for year: {}, region: {}, npmode: {}'
			.format(year,region,npmode))
		print('#####')
		combpath = os.path.join(topdir,year+'combined',region,npmode)
		if os.path.exists(combpath):
		    os.system('rm -r '+combpath)
		os.makedirs(combpath) 
		outputfile = os.path.join(combpath,'combined.root')
		print('found following files:')
		filestomerge = getfilestomerge(topdir,year,region,npmode) # implicit printing
		# do hadd
		print('running hadd command...')
		command = 'hadd '+outputfile
		for f in filestomerge:
		    command += ' '+f
		os.system(command)
		# clip histograms to minimum zero
		print('clipping all histograms...')
		clipallhistograms(outputfile)
		# rebin discriminating variable (calls external c++ executable)
		print('rebinning...')
		if not os.path.exists('./rebinoutput'):
		    print('### WARNING ###: rebinoutput executable does not seem to exist')
		    print('		    calling make command but program will crash if that command fails')
		    os.system('make -f makeRebinOutput')
		tempfilename = outputfile[-5:]+'_temp.root'
		if os.path.exists(tempfilename):
		    os.system('rm '+tempfilename)
		command = './rebinoutput '+outputfile+' '+tempfilename+' _fineBinnedeventBDT 1 2'
		os.system(command)
		os.system('mv '+tempfilename+' '+outputfile)
		# rename systematic histograms (calls external python script)
		print('implementing correlations in histogram names...')
		implementcorrelations(outputfile,tempfilename)
		os.system('mv '+tempfilename+' '+outputfile)
		
