#####################################################################################
# a Python script to merge the output of runsystematics.py, basically advanced hadd #
#####################################################################################

# this script performs several tasks
# - merging the correct root files, depending on the year, region and nonprompt mode
# - clipping al histograms to minimum zero
# - copying all systematic histograms with year suffix to systematic for (de)correlating

import os
import sys
import ROOT
# imports for job submission
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
# imports for sample list reading 
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist
from extendsamplelist import extendsamplelist
# import tools
sys.path.append(os.path.abspath('../tools'))
import histtools as histtools
import smalltools as tls
# import local functions
from correlations import implementcorrelations
from systematicstools import getfilestomerge

##### help functions #####

def removehistogramsbytags(histfile,tags=[[]]):
    ### read a root file and delete histograms containing some tags
    # note that tags is a list of list of strings
    # histograms contain ALL strings in (at least) one of the lists in tags are removed
    histlist = histtools.loadallhistograms(histfile)
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

##### main function #####

def mergeoutput(topdir,year,region,npmode,samplelistdir):

    print('#####')
    print('merging all files for year: {}, region: {}, npmode: {}'
	    .format(year,region,npmode))
    print('#####')

    # step 1: merge all relevant files using hadd
    combpath = os.path.join(topdir,year+'combined',region,npmode)
    if os.path.exists(combpath):
        os.system('rm -r '+combpath)
    os.makedirs(combpath)
    outputfile = os.path.join(combpath,'combined.root')
    tempfilename = outputfile[:-5]+'_temp.root'
    print('found following files:')
    filestomerge = getfilestomerge(topdir,year,region,npmode,samplelistdir) # implicit printing
    if len(filestomerge)==0: return
    print('running hadd command...')
    command = 'hadd '+outputfile
    for f in filestomerge: command += ' '+f
    os.system(command)

    # remove some histograms from the file
    print('removing superfluous histograms...')
    removehistogramsbytags(outputfile,removesystematics)
    # clip histograms to minimum zero
    print('clipping all histograms...')
    histtools.clipallhistograms(outputfile)
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

    # global setting of sample list directory (to switch between old and new sample lists)
    samplelistdir = os.path.abspath('../samplelists/samplelists_tzq_v_iTuple')

    # global setting of years and regions to run on (ignored if year and region are in arguments)
    yearlist = []
    yearlist.append('2016')
    yearlist.append('2017')
    yearlist.append('2018')
    regionlist = []
    regionlist.append('signalregion_cat1')
    for r in ['signalregion_cat2','signalregion_cat3']: regionlist.append(r)
    regionlist.append('signalregion_cat123')
    regionlist.append('wzcontrolregion')
    regionlist.append('zzcontrolregion')
    regionlist.append('zgcontrolregion')
    regionlist.append('ttzcontrolregion')
    regionlist.append('signalsideband_noossf_cat1')
    regionlist.append('signalsideband_noz_cat1')
    #regionlist.append('signalsideband_noossf_cat12')
    #regionlist.append('signalsideband_noz_cat12')
    #regionlist.append('signalsideband_noz_cat123')
    #regionlist.append('signalsideband_noossf')
    #regionlist.append('signalsideband_noz')
    #regionlist.append('signalsideband_noz_cat1_ch02')
    #regionlist.append('signalsideband_noz_cat1_ch13')

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
    removesystematics.append(['_pdfShapeEnv'])
    #removesystematics.append(['_pdfShapeRMS'])
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
	commands = []
	for year in yearlist:
	    for region in regionlist:
		command = 'python mergeoutput.py {} {} {}'.format(
                        topdir, year, region )
		# old qsub way:
		script_name = 'mergeoutput.sh'
		with open(script_name,'w') as script:
		    initializeJobScript(script)
		    script.write('cd {}\n'.format(cwd))
		    script.write(command+'\n')
		#submitQsubJob(script_name)
		# alternative: run locally
		#os.system('bash '+script_name)
		commands.append(command)
	# new condor way:
	ct.submitCommandsAsCondorCluster('cjob_mergeoutput',commands)
    
    else:
	for npmode in npmodelist:
	    mergeoutput(topdir,year,region,npmode,samplelistdir)

    sys.stderr.write('###done###\n')
