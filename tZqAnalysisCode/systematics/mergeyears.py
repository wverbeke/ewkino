#############################################################
# a Python script to merge the histogram files of all years #
#############################################################
# to be used after mergeoutput.py!

# this script runs in several steps
# - extract nominal histograms
# - ordinary hadd
# - next loop over all histograms and add e.g. nominal of 17 and 18 to uncorrelated 16 variation

import os
import sys
import ROOT
sys.path.append(os.path.abspath('../tools'))
import histtools
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission/'))
import condorTools as ct

def getfilestomerge(topdir,region,npmode):
    ### get a list of files to merge for this specific region and nonprompt mode
    years = []
    files = {}
    for year in ['2016','2017','2018']:
	if os.path.exists(os.path.join(topdir,year+'combined',region,npmode,'combined.root')): 
	    years.append(year)
	    files[year] = os.path.join(topdir,year+'combined',region,npmode,'combined.root')
    print('for region '+region+' and npmode '+npmode+', found following years:')
    print(years)
    return files

def addnominals(histlist,nominalhists):
    ### read a root file and add nominal histograms to year-uncorrelated histograms
    # - histlist is a list of histograms, some of which might be
    # variations that are uncorrelated between years
    # - nominalhists is a dict matching years to lists of nominal histograms
    for hist in histlist:
        histname = hist.GetName()
	# check if histogram is a variation uncorrelated between years
	year = ''
	if '201' in histname.rsplit('_',1)[1]:
	    year = histname.rsplit('_',1)[1][:4]
	if year=='': continue
	# if so, find corresponing nominal ones of other years
	for nyear in nominalhists.keys():
	    if nyear==year: continue
	    for nhist in nominalhists[nyear]:
		if nhist.GetName().replace('nominal','') in histname:
		    hist.Add(nhist)
    return histlist

def mergeyears(topdir,region,npmode):
    print('#####')
    print('merging all files for region: {}, npmode: {}'
            .format(region,npmode))
    print('#####')
    combpath = os.path.join(topdir,'yearscombined',region,npmode)
    if os.path.exists(combpath):
        os.system('rm -r '+combpath)
    os.makedirs(combpath)
    outputfile = os.path.join(combpath,'combined.root')
    filestomerge = getfilestomerge(topdir,region,npmode)
    if len(filestomerge)==0: 
	raise Exception('no files found for this region / npmode combination...')
    # get nominal histograms
    nominalhists = {}
    for year in filestomerge.keys():
        histlist = histtools.loadallhistograms(filestomerge[year])
        nominalhists[year] = []
        for hist in histlist:
            if 'nominal' in hist.GetName(): nominalhists[year].append(hist.Clone())
    # do hadd
    # note: gives errors for rebinnedeventBDT histograms but these can be ignored;
    #       as long as we do another rebinning afterwards things should be fine.
    print('running hadd command...')
    command = 'hadd '+outputfile
    for year in filestomerge.keys():
        command += ' '+filestomerge[year]
    os.system(command)
    # take care of uncorrelated variations between years
    print('adding nominals to uncorrelated variations...')
    histlist = histtools.loadallhistograms(outputfile)
    addnominals(histlist,nominalhists)
    tempfilename = outputfile[:-5]+'_temp.root'
    f = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
        hist.Write()
    f.Close()
    os.system('mv '+tempfilename+' '+outputfile)
    # re-do rebinning of eventBDT
    '''print('rebinning...')
    if not os.path.exists('./rebinoutput'):
        print('### WARNING ###: rebinoutput executable does not seem to exist')
        print('                 calling make command but program will crash if that command fails')
        os.system('make -f makeRebinOutput')
    tempfilename = outputfile[-5:]+'_temp.root'
    if os.path.exists(tempfilename):
        os.system('rm '+tempfilename)
    command = './rebinoutput '+outputfile+' '+tempfilename+' _fineBinnedeventBDT 1 2'
    os.system(command)
    os.system('mv '+tempfilename+' '+outputfile) '''

if __name__=='__main__':

    sys.stderr.write('###starting###\n')

    runmultiple = False
    topdir = ''
    region = ''
    npmode = ''
    if len(sys.argv)==2:
	topdir = sys.argv[1]
	runmultiple = True
    elif len(sys.argv)==4:
	topdir = sys.argv[1]
	region = sys.argv[2]
	npmode = sys.argv[3]
	runmultiple = False
    else:
	print('### ERROR ###: wrong number of command line arguments.')
	print('               need at least <topdir>')
	sys.exit()

    if not runmultiple:
	mergeyears( topdir, region, npmode )
    else:
	regionlist = []
	#regionlist.append('signalregion_cat1')
	#regionlist.append('signalregion_cat2')
	#regionlist.append('signalregion_cat3')
	#regionlist.append('signalregion_cat123')
	#regionlist.append('wzcontrolregion')
	#regionlist.append('zzcontrolregion')
	regionlist.append('zgcontrolregion')
	#regionlist.append('ttzcontrolregion')
	#for r in ['signalsideband_noossf_cat1']: regionlist.append(r)
	#for r in ['signalsideband_noz_cat1']: regionlist.append(r)
	#for r in ['signalsideband_cat1']: regionlist.append(r)
	npmodelist = []
	#npmodelist.append('npfromsim')
	npmodelist.append('npfromdata')

	commands = []
	cwd = os.getcwd()
	for region in regionlist:
	    for npmode in npmodelist:
		print('now running on {}/{}'.format(region,npmode))
		command = 'python mergeyears.py {} {} {}'.format(
				topdir, region, npmode )
		commands.append(command)
		# old qsub way:
                script_name = 'mergeyears.sh'
                with open(script_name,'w') as script:
                    initializeJobScript(script)
                    script.write('cd {}\n'.format(cwd))
                    script.write(command+'\n')
                submitQsubJob(script_name)
                #os.system('bash '+script_name)
        # new condor way:
	#ct.submitCommandsAsCondorCluster('cjob_mergeyears', commands)

    sys.stderr.write('###done###\n')
