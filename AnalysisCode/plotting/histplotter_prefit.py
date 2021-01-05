###################################################################################
# Extension of histplotter.py reading input histograms from a different structure #
# and also implementing the plotting of systematic variations			  #
###################################################################################
import ROOT
import sys
import numpy as np
import os
import json
import plottools as tools
import histplotter as hp
sys.path.append(os.path.abspath('../tools'))
import histtools as ht
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

def loadallhistograms(histfile,variable):
    ### read file and load all histograms for a given variable in a dictionary 

    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    histdict = {}
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        hist.SetDirectory(0)
	# check if histogram needs to be included
        if not variable in hist.GetName(): continue
        # check if histogram is readable
        try:
            nentries = hist.GetEntries() # maybe replace by more histogram-specific function
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
        # add hist to dict
        histdict[hist.GetName()] = hist
    return histdict


def selecthistograms(histdict,processes,variable,variation):
    ### select a subset of histograms from a dictionary read by loadallhistograms

    histlist = []
    for key in list(histdict.keys()):
	keep = False
	for process in processes:
	    if(key==process+'_'+variable+'_'+variation): keep=True; break
	if keep: histlist.append(histdict[key].Clone())
    return histlist


def getallsystematics(histdict,variable):
    ### get a list of all systematics present in a dictionary read by loadallhistograms
    
    # open root file and read processes with systematics
    slist = []
    for key in list(histdict.keys()):
        systematic = key.split(variable)[-1].strip('_')
        if(systematic[-2:]=='Up'): systematic = systematic[:-2]
        elif(systematic[-4:]=='Down'): systematic = systematic[:-4]
        if systematic == 'nominal': continue
        if not systematic in slist: slist.append(systematic)
    return slist


def getsystematichistogram(histdict,variedprocesses,nominalprocesses,variable,systematicslist):
    ### extract a histogram containing combined systematic uncertainty

    # remove duplicates between variedprocesses and nominalprocesses
    newnominalprocesses = [p for p in nominalprocesses if p not in variedprocesses]
    nominalprocesses = newnominalprocesses

    # for each systematic, sum up and down variations for all processes
    totalvar = {}

    for systematic in systematicslist:
	for suffix in ['Up','Down']:
	    histlist = []
	    # for all varied processes, check if variation is found, if not, use nominal
	    for p in variedprocesses:

		# temp for testing:
		#if(systematic=='pdfShapeRMS' and p=='nonprompt'): continue	

		hists = selecthistograms(histdict,[p],variable,systematic+suffix)
		if len(hists)==1:
		    histlist.append(hists[0])
		else:
		    print('### WARNING ###: number of histograms for {}/{}/{} is {}'.format(
			    p,variable,systematic+suffix,len(hists)))
		    print('                 using nominal instead...')
		    hists = selecthistograms(histdict,[p],variable,'nominal')
		    if len(hists)==1:
			histlist.append(hists[0])
		    else:
			print('### ERROR ###: nominal histogram not found either...')
			sys.exit()
	    histlist += selecthistograms(histdict,nominalprocesses,variable,'nominal')
	    firsthist = histlist[0]
	    for hist in histlist[1:]: firsthist.Add(hist)
	    totalvar[systematic+suffix] = firsthist

    # print
    #for var in list(totalvar.keys()):
	#print(var)
	#for i in range(1,totalvar[var].GetNbinsX()+1):
	    #print(totalvar[var].GetBinContent(i))
    
    # now get the nominal sum of processes
    histlist = selecthistograms(histdict,variedprocesses+nominalprocesses,variable,'nominal')
    nominalhist = histlist[0]
    for hist in histlist[1:]: nominalhist.Add(hist)

    # for each systematic, get the bin-per-bin max of (abs(up-nominal),abs(down-nominal))
    maxhistlist = []
    for systematic in systematicslist:
	uphist = totalvar[systematic+'Up']
	downhist = totalvar[systematic+'Down']
	maxhist = ht.binperbinmaxvar( [uphist,downhist], nominalhist )
	totalvar[systematic+'Max'] = maxhist
	maxhistlist.append(maxhist)
    
    # add resulting systematic histograms in quadrature
    syshist = ht.rootsumsquare(maxhistlist)
    return syshist


def histplotter_prefit(histfile,variables,mcprocesses,dataprocesses,npdataprocesses,varstoblind=[]):
    ### main function, called by __main__ either directly or in job submission

    histdir = histfile[:histfile.rfind('/')]
    lumi = 0.
    if '2016' in histfile: lumi = 35900
    elif '2017' in histfile: lumi = 41500
    elif '2018' in histfile: lumi = 59700
    elif '1617' in histfile: lumi = 77400
    elif 'allyears' in histfile: lumi = 137100

    # loop over input variables
    for k,vardict in enumerate(variables):
        varname = str(vardict['name'])
	print('running on variable: '+varname)
        # (explicit conversion from unicode to str seems necessary...)
	
        # load all histograms for this variable
        allhists = loadallhistograms(histfile,varname)
	# if no histograms are found, skip to next variable without crashing
	if len(allhists)==0: 
	    print('### WARNING ###: no histograms were found for '+varname+' in '+histfile)    
	    continue

        # load nominal histograms
        mchistlist = selecthistograms(allhists,mcprocesses,varname,'nominal')
        datahistlist = selecthistograms(allhists,dataprocesses,varname,'nominal')
        if not len(datahistlist)==1:
            print('### ERROR ###: list of data histograms has unexpected length: '+str(len(datahistlist)))
	    sys.exit()
        datahist = datahistlist[0]
        npdatahistlist = selecthistograms(allhists,npdataprocesses,varname,'nominal')
        if not (len(npdatahistlist)==0 or len(npdatahistlist)==1):
            print('### ERROR ###: list of nonprompt data histograms has unexpected length.')
            sys.exit()
        if len(npdatahistlist)>0:
            npdatahist = npdatahistlist[0]
            mchistlist.append(npdatahist)

	# if running blindly, reset the data histogram
	if varname in varstoblind:
	    datahist.Reset()

        # get total systematic variation
        slist = getallsystematics(allhists,varname)
        syshist = getsystematichistogram(allhists,mcprocesses,[],varname,slist)
	#print('--------')
	#for i in range(1,syshist.GetNbinsX()+1):
	#    print(syshist.GetBinContent(i))

        # set plot properties
        binwidth = datahist.GetBinWidth(1)
        if binwidth.is_integer():
            yaxtitle = 'events / '+str(int(binwidth))+' '+vardict['unit']
        else:
            yaxtitle = 'events / {0:.2f}'.format(binwidth)+' '+vardict['unit']
        xaxtitle = vardict['title']
        if not vardict['unit']=='':
            xaxtitle += '('+vardict['unit']+')'
        figname = os.path.join(histdir,varname)

	print(datahist)
	print(mchistlist)
	print(syshist)

        hp.plotdatavsmc(datahist,mchistlist,syshist,yaxtitle,False,xaxtitle,lumi,figname+'_lin')
        hp.plotdatavsmc(datahist,mchistlist,syshist,yaxtitle,True,xaxtitle,lumi,figname+'_log')

if __name__=="__main__":
    # read a root file containing histograms and make the plots
    # for all variables in an input dict

    # file to read the histograms from
    histfile = os.path.abspath('../systematics/output_tzqid/2016combined/wzcontrolregion/npfromdata/combined.root')
    # variables with axis titles, units, etc.
    variables = [
        {'name':'_abs_eta_recoil','title':r'#||{#eta}_{recoil}','unit':''},
        {'name':'_Mjj_max','title':r'M_{jet+jet}^{max}','unit':'GeV'},
        {'name':'_lW_asymmetry','title':r'asymmetry (lepton from W)','unit':''},
        {'name':'_deepCSV_max','title':r'highest deepCSV','unit':''},
        {'name':'_lT','title':'L_{T}','unit':'GeV'},
        {'name':'_MT','title':'M_{T}','unit':'GeV'},
        {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
        {'name':'_dRlb_min','title':r'#Delta R(lep,bjet)_{min}','unit':''},
        {'name':'_dPhill_max','title':r'#Delta #Phi (lep,lep)_{max}','unit':''},
        {'name':'_HT','title':r'H_{T}','unit':'GeV'},
        {'name':'_nJets','title':r'number of jets','unit':''},
        {'name':'_dRlWrecoil','title':r'#Delta R(lep_{W},jet_{recoil})','unit':''},
        {'name':'_dRlWbtagged','title':r'#Delta R(lep_{W},jet_{b-tagged})','unit':''},
        {'name':'_M3l','title':r'M_{3l}','unit':'GeV'},
	{'name':'_fineBinnedM3l','title':r'M_{3l}','unit':'GeV'},
        {'name':'_abs_eta_max','title':r'#||{#eta}_{max}','unit':''},
        {'name':'_eventBDT','title':r'event BDT output score','unit':''},
        {'name':'_nMuons','title':r'number of muons in event','unit':''},
        {'name':'_nElectrons','title':r'number of electrons in event','unit':''},
	{'name':'_yield','title':r'total yield','unit':''},
        #{'name':'_leptonMVATOP_min','title':r'minimum TOP MVA value in event','unit':''},
        #{'name':'_leptonMVAttH_min','title':r'minimum ttH MVA value in event','unit':''},
	{'name':'_rebinnedeventBDT','title':'event BDT output score','unit':''},
	{'name':'_leptonPtLeading','title':r'leading lepton p_{T}','unit':'GeV'},
	{'name':'_leptonPtSubLeading','title':r'subleading lepton p_{T}','unit':'GeV'},
	{'name':'_leptonPtTrailing','title':r'trailing lepton p_{T}','unit':'GeV'},
	{'name':'_leptonEtaLeading','title':r'leading lepton #eta','unit':''},
        {'name':'_leptonEtaSubLeading','title':r'subleading lepton #eta','unit':''},
        {'name':'_leptonEtaTrailing','title':r'trailing lepton #eta','unit':''},
    ]
    # some more arguments, hard coded for now
    mcprocesses = (['tZq','WZ','multiboson','tbarttX','tbartZ','ZZH'])
    mcprocesses.append('Xgamma')
    mcprocesses.append('nonprompt')
    dataprocesses = ['data']
    npdataprocesses = []
    # blind the event BDT variables
    varstoblind = [var['name'] for var in variables if 'eventBDT' in var['name']]

    # if no command line arguments, run for hard-coded parameters
    if(len(sys.argv)==1):
	histplotter_prefit(histfile,variables,mcprocesses,dataprocesses,npdataprocesses,
			    varstoblind=varstoblind)
    # if 1 command line argument and it is a root file, run on this file
    elif(len(sys.argv)==2 and sys.argv[1][-5:]=='.root'):
	histfile = os.path.abspath(sys.argv[1])
	if not os.path.exists(histfile):
	    print('### ERROR ###: requested to run on '+histfile+' but it does not seem to exist...')
	    sys.exit()
	histplotter_prefit(histfile,variables,mcprocesses,dataprocesses,npdataprocesses,
			    varstoblind=varstoblind)
    # if 1 command line argument and it is a folder, run on all files in it
    elif(len(sys.argv)==2):
	topdir = os.path.abspath(sys.argv[1])
	if not os.path.exists(topdir):
            print('### ERROR ###: requested to run on '+topdir+' but it does not seem to exist...')
            sys.exit()
	histfiles = []
	for root,dirs,files in os.walk(topdir):
	    for fname in files:
		if fname=='combined.root':
		    histfiles.append(os.path.join(root,fname))
	print('histplotter_prefit.py will run on the following files:')
	print(histfiles)
	#histfiles = histfiles[:1] # temp for testing
	commands = []
	for f in histfiles:
	    command = 'python histplotter_prefit.py '+f
	    # old qsub way:
	    #script_name = 'histplotter_prefit.sh'
	    #with open(script_name,'w') as script:
	    #	initializeJobScript( script )
	    #	script.write(command)
	    #submitQsubJob(script_name)
	    # alternative: run locally
	    #os.system('bash '+script_name)
	    commands.append(command)
	ct.submitCommandsAsCondorCluster('histplotter_prefit_cjob',commands)

    else:
	print('### ERROR ###: unrecognized command line arguments.')
	sys.exit()

