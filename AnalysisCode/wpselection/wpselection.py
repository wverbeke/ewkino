#####################################################################
# a python submission script for lepton MVA working point selection #
#####################################################################
import sys
import os
import ROOT
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../tools'))
import smalltools as tls
sys.path.append(os.path.abspath('../samplelists'))
from extendsamplelist import extendsamplelist

# set global properties
wps = ([0.,0.1,0.2,0.3,0.4,0.5,0.525,0.55,0.575,0.6,0.625,0.65,0.675,
	0.7,0.725,0.75,0.775,0.8,0.825,0.85,0.875,0.9,0.925,0.95,0.975,1.])
years = ['2016','2018']
useMVA = True
pathtoxmlfile = os.path.abspath('../bdt/outdata_wp0p8_bdt2/weights/tmvatrain_BDT.weights.xml')
#inputdir = os.path.abspath('/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples')
#samplelistdir = os.path.abspath('/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples')
inputdir = os.path.abspath('/user/llambrec/Files/trileptonskim')
samplelistdir = os.path.abspath('/user/llambrec/ewkino/AnalysisCode/samplelists')
outputdir = os.path.abspath('output_wp0p8_bdt2')
if(os.path.exists(outputdir)):
    print('### WARNING ###: output directory already exists. Clean it? (y/n)')
    go = raw_input()
    if not go == 'y': sys.exit()
    os.system('rm -r '+outputdir)
os.makedirs(outputdir)
cwd = os.getcwd()

# check input directories and sample files
yeardirdict = {}
yearsldict = {}
for year in years:
    thisyearinputdir = os.path.join(inputdir,year+'MC')
    if(not os.path.exists(thisyearinputdir)):
        print('### ERROR ###: some of the input directories do not seem to exist')
        sys.exit()
    thisyearsamplelist = os.path.join(samplelistdir,'samplelist_tzq_'+year+'_MC.txt')
    if(not os.path.exists(thisyearsamplelist)):
        print('### ERROR ###: some of the samplelists are not found')
        sys.exit()
    yeardirdict[year] = thisyearinputdir
    yearsldict[year] = thisyearsamplelist

# check xml file for event BDT
if(useMVA and not os.path.exists(pathtoxmlfile)):
    print('### ERROR ###: XML file for event BDT does not exist')
    sys.exit()

# check if executable is present
exe_name = 'wpselection'
if not os.path.exists('./'+exe_name):
    print('### ERROR ###: executable not found')
    sys.exit()

# run over all years
for year in years:
    
    # read sample list
    filelist = extendsamplelist(yearsldict[year],yeardirdict[year])
    _,lumi = tls.year_and_lumi_from_samplelist(yearsldict[year]) 

    # make output directories
    thisyearoutputdir = os.path.join(outputdir,year)
    os.makedirs(thisyearoutputdir)
	
    # submit a job for each file
    for fdict in filelist:
	inf = fdict['file']
	xsec = fdict['cross_section']
	temp = ROOT.TFile(inf)
	hcounter = temp.Get('blackJackAndHookers/hCounter').GetSumOfWeights()
	norm = xsec*lumi/hcounter
	outf = inf.split('/')[-1].rstrip('.root')+'.txt'
	outf = os.path.join(thisyearoutputdir,outf)
	script_name = exe_name+'.sh'
	with open(script_name,'w') as script:
	    initializeJobScript(script)
	    script.write('cd {}\n'.format(cwd))
	    command = './'+exe_name+' {} {} {} {} {}'.format(inf,norm,outf,
					    useMVA,pathtoxmlfile)
	    for wp in wps:
		command += ' {}'.format(wp)
	    print(command)
	    script.write(command+'\n')
	submitQsubJob(script_name)
	# alternative: run locally
	#os.system('bash '+script_name) 
