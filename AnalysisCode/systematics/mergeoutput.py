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
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist
from extendsamplelist import extendsamplelist
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls
from correlations import implementcorrelations

##### help functions #####

def clipallhistograms(histfile):
    ### read a root file and clip all histograms in it to minimum zero
    histlist = tls.loadallhistograms(histfile)
    for hist in histlist:
	for i in range(0,hist.GetNbinsX()+2):
	    if hist.GetBinContent(i)<0:
		hist.SetBinContent(i,0)
		hist.SetBinError(i,0)
	# special: check if histogram is empty after clipping and if so, fill it with dummy value
	if hist.GetSumOfWeights()<1e-12: hist.SetBinContent(1,1e-6)
    tempfilename = histfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
	hist.Write()
    f.Close()
    os.system('mv '+tempfilename+' '+histfile)

def removehistogramsbytags(histfile,tags=[[]]):
    ### read a root file and delete histograms containing some tags
    # note that tags is a list of list of strings
    # histograms contain ALL strings in (at least) one of the lists in tags are removed
    histlist = tls.loadallhistograms(histfile)
    newhistlist = []
    for hist in histlist:
	keep = True
        for taglist in tags:
	    keep = False
	    for tag in taglist:
		if tag not in hist.GetName(): keep = True
	    if not keep: break
	if keep: newhistlist.append(hist)
    tempfilename = histfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in newhistlist:
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

##### main function #####

def mergeoutput(topdir,year,region,npmode):

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
    if len(filestomerge)==0: return
    # do hadd
    print('running hadd command...')
    command = 'hadd '+outputfile
    for f in filestomerge: command += ' '+f
    os.system(command)
    # remove some histograms from the file
    print('removing superfluous histograms...')
    removehistogramsbytags(outputfile,removesystematics)
    # clip histograms to minimum zero
    print('clipping all histograms...')
    clipallhistograms(outputfile)
    # rebin discriminating variable (calls external c++ executable)
    print('rebinning...')
    if not os.path.exists('./rebinoutput'):
        print('### WARNING ###: rebinoutput executable does not seem to exist')
        print('                 calling make command but program will crash if that command fails')
        os.system('make -f makeRebinOutput')
    tempfilename = outputfile[:-5]+'_temp.root'
    if os.path.exists(tempfilename):
        os.system('rm '+tempfilename)
    command = './rebinoutput '+outputfile+' '+tempfilename+' _fineBinnedeventBDT 3 1.5'
    os.system(command)
    os.system('mv '+tempfilename+' '+outputfile)
    # rename systematic histograms (calls external python script)
    print('implementing correlations in histogram names...')
    implementcorrelations(outputfile,tempfilename)
    os.system('mv '+tempfilename+' '+outputfile)

if __name__=='__main__':

    sys.stderr.write('###starting###\n')
 
    # command line arguments: 
    # - in case of 1 argument, it is assumed to be a directory,
    #   and job will be submitted for each year and region specified in the global arguments.
    # - in case of 3 arguments, they are assumed to be a directory, year and region respectively
    #   and the program will run on this combination only.
    runmultiple = True
    topdir = ''
    year = ''
    region = ''
    if len(sys.argv)==2:
	topdir = sys.argv[1]
    elif len(sys.argv)==4:
	topdir = sys.argv[1]
	year = sys.argv[2]
	region = sys.argv[3]
	runmultiple = False
    else:
	print('### ERROR ###: wrong number of command line arguments.')
	print('               need at least <topdir>')
	sys.exit()

    # global setting of years and regions to run on (ignored if year and region are in arguments)
    yearlist = []
    yearlist.append('2016')
    yearlist.append('2017')
    yearlist.append('2018')
    regionlist = []
    regionlist.append('signalregion_1')
    for r in ['signalregion_2','signalregion_3']: regionlist.append(r)
    regionlist.append('wzcontrolregion')
    regionlist.append('zzcontrolregion')
    regionlist.append('zgcontrolregion')
    #for r in ['signalsideband_noossf_1']: regionlist.append(r)
    #for r in ['signalsideband_noz_1']: regionlist.append(r)

    # global setting of nonprompt mode to run with
    npmodelist = []
    npmodelist.append('npfromsim')
    npmodelist.append('npfromdata')

    # global setting of which systematics to take into account which to throw out
    removesystematics = []
    # choose between separate renormalization and factorization scale variations,
    # correlated variations, or their envelope
    removesystematics.append(['_rScale'])
    removesystematics.append(['_fScale'])
    removesystematics.append(['_rfScales'])
    #removesystematics.append(['_qcdScalesShapeEnv'])
    removesystematics.append(['_qcdScalesNorm','tZq'])
    removesystematics.append(['_qcdScalesNorm','WZ'])
    removesystematics.append(['_qcdScalesNorm','ZZH'])
    removesystematics.append(['_qcdScalesNorm','Xgamma'])
    removesystematics.append(['_qcdScalesNorm','tbartZ'])
    # choose between pdf envelope or rms
    #removesystematics.append(['_pdfShapeEnv'])
    removesystematics.append(['_pdfShapeRMS'])
    removesystematics.append(['_pdfNorm','tZq'])
    removesystematics.append(['_pdfNorm','WZ'])
    removesystematics.append(['_pdfNorm','ZZH'])
    removesystematics.append(['_pdfNorm','Xgamma'])
    removesystematics.append(['_pdfNorm','tbartZ'])
    # remove individual variations (if not done so before)
    removesystematics.append(['qcdScalesShapeVar'])
    removesystematics.append(['pdfShapeVar'])

    cwd = os.getcwd()
    if runmultiple:
	for year in yearlist:
	    for region in regionlist:
		script_name = 'mergeoutput.sh'
		with open(script_name,'w') as script:
		    initializeJobScript(script)
		    script.write('cd {}\n'.format(cwd))
		    command = 'python mergeoutput.py {} {} {}'.format(
			topdir, year, region )
		    script.write(command+'\n')
		submitQsubJob(script_name)
		# alternative: run locally
		#os.system('bash '+script_name)
    
    else:
	for npmode in npmodelist:
	    mergeoutput(topdir,year,region,npmode)

    sys.stderr.write('###done###\n')
