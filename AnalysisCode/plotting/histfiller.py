###############################################
# python script to fill and create histograms #
###############################################
# notice: in this script it is assumed that the input variables are read directly from a flat tree!

import ROOT
import numpy as np
from array import array
import glob
import os
import sys
import json
sys.path.append(os.path.abspath('../samplelists'))
from extendsamplelist import extendsamplelist

def getinputfiles(rootdir,samplelist):
    ### get list of input files in rootdir whose names (and cross-sections) are in samplelist
    samples_dict = extendsamplelist(samplelist,rootdir)
    return samples_dict

def removenonpromptmc(mcin):
    # modify the input file lists, removing all nonprompt processes
    mcinnew = [f for f in mcin if f['process_name']!='nonprompt']
    return mcinnew    

def initializehistograms(indict,titleprefix,variables):
    # initialize histograms for all variables and files
    # (2D lists: dimension 1: variables, dimension 2: files)
    outlist = []
    for k,vardict in enumerate(variables):
        bins = vardict['bins']
        varname = vardict['name']
        nbins = len(bins)-1
        bins = array('f',bins)
	if len(indict)==0: continue
        outlist.append([])
        for j in range(len(indict)):
            outlist[k].append(ROOT.TH1F(titleprefix+"_"+varname+"_"+str(j),
                                 indict[j]['process_name']+"/"+indict[j]['sample_name'],nbins,bins))
            outlist[k][j].SetDirectory(0)
    return outlist

def underflow(value,hist):
    lowX = hist.GetBinLowEdge(1)
    if value > lowX: return value
    highX = hist.GetBinLowEdge(2)
    return (lowX+highX)/2.

def overflow(value,hist):
    lowX = hist.GetBinLowEdge(hist.GetNbinsX())
    highX = lowX + hist.GetBinWidth(hist.GetNbinsX())
    if value < highX: return value
    return (lowX+highX)/2.

def addinputfile(histlist,filelist,treename,index,variables,
		    usenormweight,removeoverlap,normalization,lumi,evttags,
		    doextraselection):
    f = ROOT.TFile.Open(filelist[index]['file'])
    tree = f.Get("blackJackAndHookers/"+treename)
    sumweights = 1.
    xsection = 1.
    # old way of determining weight (does not work for reweighting, fake rates, etc.)
    try:
	if(usenormweight and not normalization==0):
	    sumweights = f.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
	    xsection = filelist[index]['cross_section']
    except:
	print('warning: no sumofweights or cross-section could be found')
    for j in range(int(tree.GetEntries()/1.)):
        if(j%5000==0):
            percent = j*100/tree.GetEntries()
            sys.stdout.write("\r"+str(percent)+'%')
            sys.stdout.flush()
        tree.GetEntry(j)
	if doextraselection:
	    if not passextraselection(tree): continue
	normweight = 1.
	weight = 1.
	scaledweight = 1.
	nonleptonreweight = 1.
        if(usenormweight and not normalization==0):
	    scaledweight = getattr(tree,'_scaledweight')
	    normweight = getattr(tree,'_normweight')
	    weight = getattr(tree,'_weight')*lumi*xsection/sumweights
	    nonleptonreweight = getattr(tree,'_nonleptonreweight')
	if removeoverlap:
	    evtid = str(getattr(tree,'_runNb'))
	    evtid += '/'+str(getattr(tree,'_lumiBlock'))
	    evtid += '/'+str(getattr(tree,'_eventNb'))
	    if evtid in evttags: continue
	    evttags.append(evtid) 
        for k,vardict in enumerate(variables):
            varname = vardict['name']
            varvalue = getattr(tree,varname)
            bins = vardict['bins']
            if varvalue<bins[0]: varvalue = underflow(varvalue,histlist[k][index])
	    if varvalue>bins[-1]: varvalue = overflow(varvalue,histlist[k][index])
	    # default filling method (deprecated, does not work for reweighting, fake rates, etc.):
            #histlist[k][index].Fill(varvalue,weight)
	    # alternative using precomputed normalized weight:
	    histlist[k][index].Fill(varvalue,normweight)
    f.Close()
    sys.stdout.write("\r"+'100%'+"\n")
    sys.stdout.flush()
    return evttags

def normalizemctodata(mchistlist,datahistlist,variables):
    # normalize MC to data.
    # note that the sum of weights has to be calculated for each variable separately,
    # since different ranges for different variables might hold a different number of events.
    print('post-processing simulation files...')
    nmcfiles = len(mchistlist[0])
    ndatafiles = len(datahistlist[0])
    for k in range(len(variables)):
        mchist = mchistlist[k][0].Clone()
        for i in range(1,nmcfiles):
            mchist.Add(mchistlist[k][i])
        ndataw = datahistlist[k].GetSumOfWeights()
        nmcw = mchist.GetSumOfWeights()
        scale = ndataw/nmcw
        for hist in mchistlist[k]:
            hist.Scale(scale)

def mergemchistograms(mchistlist):
    # merge histograms with the same process name (as indicated in the sample list)
    print('merging simulation histograms...')
    for k in range(len(mchistlist)):
        tags = []; newhistset = []
        for hist in mchistlist[k]:
            tag = hist.GetTitle()[:hist.GetTitle().rfind('/')]
            if tag in tags:
                index = tags.index(tag)
                newhistset[index].Add(hist)
            else:
                index = len(tags)
                tags.append(tag)
                newhistset.append(hist)
                newhistset[index].SetTitle(tag)
                newhistset[index].SetName(hist.GetName()[:hist.GetName().rfind("_")+1]+str(index))
        mchistlist[k] = newhistset

def mergedatahistograms(datahistlist,title):
    # merge all data histograms
    print('merging data histograms...')
    for k in range(len(datahistlist)):
        firsthist = datahistlist[k][0]
        newhist = firsthist.Clone()
        for hist in datahistlist[k][1:]:
            newhist.Add(hist)
        newhist.SetTitle(title)
        newhist.SetName(firsthist.GetName()[:firsthist.GetName().rfind("_")+1]+str(0))
        datahistlist[k] = newhist

def savehistograms(mchistlist,datahistlist,npdatahistlist,histfile,normalization,lumi):
    print('writing histograms to file...')
    f = ROOT.TFile.Open(histfile,"recreate")
    for histset in mchistlist:
        for hist in histset:
            hist.Write(hist.GetName())
    for hist in datahistlist:
        hist.Write(hist.GetName())
    for hist in npdatahistlist:
	hist.Write(hist.GetName())
    normalization_st = ROOT.TVectorD(1); normalization_st[0] = normalization
    normalization_st.Write("normalization")
    lumi_st = ROOT.TVectorD(1)
    lumi_st[0] = lumi
    lumi_st.Write("lumi")
    f.Close()

def passextraselection(tree):
    # warning: must have called tree.GetEntry(j) before calling this function
    passBDT = getattr(tree,'_eventBDT')>0
    passlepptlead = getattr(tree,'_leptonPtLeading')>25.
    passlepptsublead = getattr(tree,'_leptonPtSubLeading')>15.
    passleppttrail = getattr(tree,'_leptonPtTrailing')>10.
    return (passlepptlead and passlepptsublead and passleppttrail)
    

def input_from_cmd(arglist):
    # read command line arguments and return a dict.
    # the order of the arguments is fixed and no checking is done, but this can be generalized
    if not len(sys.argv)==13:
	print('### ERROR ###: wrong number of command line arguments.')
	sys.exit()
    res = {}
    res['mcrootdir'] = sys.argv[1]
    res['mcsamplelist'] = sys.argv[2]
    res['datarootdir'] = sys.argv[3]
    res['datasamplelist'] = sys.argv[4]
    res['histfile'] = sys.argv[5]
    res['variables'] = json.loads(sys.argv[6])
    res['treename'] = sys.argv[7]
    res['normalization'] = int(sys.argv[8])
    res['lumi'] = float(sys.argv[9])
    res['doextraselection'] = bool(sys.argv[10]=='True')
    res['usedata'] = bool(sys.argv[11]=='True')
    res['npfromdata'] = bool(sys.argv[12]=='True')
    return res

if __name__ == '__main__':
    # create histogram objects for an arbitrary number of variables
    # using a collection of root files in a single sample list, directory and tree.
  
    args = {}
    ### Configure input parameters (hard-coded)
    # folder to read mc root files from
    args['mcrootdir'] = '/user/llambrec/Files/tzqmedium0p4id/2016MC/wzcontrolregion'
    # folder to read data root files from
    args['datarootdir'] = '/user/llambrec/Files/tzqmedium0p4id/2016data/wzcontrolregion'
    #args['datarootdir'] = args['mcrootdir']
    # samplelist for simulation with process names and cross sections
    args['mcsamplelist'] = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2016_MC.txt'
    ##args['mcsamplelist'] = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/samplelist_tzq_2016_MC.txt'
    # samplelist for data with dataset names
    args['datasamplelist'] = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2016_data.txt' 
    #args['datasamplelist'] = args['mcsamplelist']
    # file to store histograms in:
    args['histfile'] = '/user/llambrec/ewkino/AnalysisCode/plotting/histograms/histograms.root'
    # list of dicts of variables to plot with corresponding bins:
    args['variables'] = [
        {'name':'_abs_eta_recoil','bins':list(np.linspace(0,5,num=21))},
        {'name':'_Mjj_max','bins':list(np.linspace(0,1200,num=21))},
        {'name':'_lW_asymmetry','bins':list(np.linspace(-2.5,2.5,num=21))},
        {'name':'_deepCSV_max','bins':list(np.linspace(0,1,num=21))},
        {'name':'_lT','bins':list(np.linspace(0,800,num=21))},
        {'name':'_MT','bins':list(np.linspace(0,300,num=21))},
        {'name':'_pTjj_max','bins':list(np.linspace(0,300,num=21))},
        {'name':'_dRlb_min','bins':list(np.linspace(0,3.14,num=21))},
        {'name':'_dPhill_max','bins':list(np.linspace(0,3.14,num=21))},
        {'name':'_HT','bins':list(np.linspace(0,800,num=21))},
        {'name':'_nJets','bins':list(np.linspace(-0.5,10.5,num=11))},
        {'name':'_dRlWrecoil','bins':list(np.linspace(0,10,num=21))},
        {'name':'_dRlWbtagged','bins':list(np.linspace(0,7,num=21))},
        {'name':'_M3l','bins':list(np.linspace(0,600,num=21))},
        {'name':'_abs_eta_max','bins':list(np.linspace(0,5,num=21))},
	{'name':'_eventBDT','bins':list(np.linspace(-1,1,num=16))},
	{'name':'_nMuons','bins':list(np.linspace(-0.5,3.5,num=5))},
	{'name':'_nElectrons','bins':list(np.linspace(-0.5,3.5,num=5))},
	{'name':'_yield','bins':list(np.linspace(0,1,num=2))},
	#{'name':'_leptonMVATOP_min','bins':list(np.linspace(-1,1,num=41))},
	#{'name':'_leptonMVAttH_min','bins':list(np.linspace(-1,1,num=41))},
	{'name':'_leptonPtLeading','bins':list(np.linspace(10,150,num=21))},
	{'name':'_leptonPtSubLeading','bins':list(np.linspace(10,150,num=21))},
	{'name':'_leptonPtTrailing','bins':list(np.linspace(10,150,num=21))}
    ]
    # name of tree to read for each input file:
    args['treename'] = 'blackJackAndHookersTree'
    # normalization:
    args['normalization'] = 1
    # (normalization parameter: 0 = no normalization, weights, xsection and lumi set to 1.)
    # (             1 = normalize using weights, xsection and lumi.)
    # (             2 = same as before but apply normalization to data afterwards.)
    # luminosity in inverse picobarns
    args['lumi'] = 35900.
    args['doextraselection'] = False
    args['usedata'] = True # whether to include datapoints
    args['npfromdata'] = False # whether to use fosideband files for nonprompt background

    ### Overwrite using cmd args
    if(len(sys.argv)==13):
	args = input_from_cmd(sys.argv)
    elif(not len(sys.argv)==1):
	print('### ERROR ###: wrong number of command line args: '+str(len(sys.argv)))
	sys.exit()

    ### Make all paths global
    mcrootdir = os.path.abspath(args['mcrootdir'])
    mcsamplelist = os.path.abspath(args['mcsamplelist'])
    datarootdir = os.path.abspath(args['datarootdir'])
    datasamplelist = os.path.abspath(args['datasamplelist'])
    histfile = os.path.abspath(args['histfile'])

    ### Overwrite output file
    if os.path.exists(histfile): os.system('rm '+histfile)
    histdir = histfile[:histfile.rfind('/')]
    if not os.path.exists(histdir): os.makedirs(histdir)

    ### Configure input files
    mcin = []
    datain = []
    npfromdatain = []
    pcorrectionin = []
    if not args['npfromdata']:
	mcin = getinputfiles(mcrootdir+'_3tight_flat',mcsamplelist)
	datain = getinputfiles(datarootdir+'_3tight_flat',datasamplelist)
	mcin = [f for f in mcin if not 'ZG' in f['sample_name']]
    else:
	mcin = getinputfiles(mcrootdir+'_3prompt_flat',mcsamplelist)
	datain = getinputfiles(datarootdir+'_3tight_flat',datasamplelist)
	npfromdatain = getinputfiles(datarootdir+'_fakerate_flat',datasamplelist)
	pcorrectionin += getinputfiles(mcrootdir+'_fakerate_flat',mcsamplelist)
	mcin = removenonpromptmc(mcin)
    if not args['usedata']: datain = datain[1:2] # speed-up if no data is used

    print('=== MC files ===')
    for s in mcin: print(s['sample_name'])
    print('=== data files ===')
    for s in datain: print(s['sample_name'])
    print('=== nonprompt data files ===')
    for s in npfromdatain: print(s['sample_name'])
    print('=== prompt correction files ===')
    for s in pcorrectionin: print(s['sample_name'])

    ### Fill histograms
    mchistlist = initializehistograms(mcin,'mc',args['variables'])
    datahistlist = initializehistograms(datain,'data',args['variables'])
    npfromdatahistlist = initializehistograms(npfromdatain,'npdata',args['variables'])
    pcorrectionhistlist = initializehistograms(pcorrectionin,'pcorrection',args['variables'])
    print('adding simulation files...')
    for i in range(len(mcin)):
        print('file '+str(i+1)+' of '+str(len(mcin)))
	print(mcin[i]['sample_name'])
        addinputfile(mchistlist,mcin,args['treename'],i,args['variables'],
			True,False,args['normalization'],args['lumi'],[],
			args['doextraselection'])
    print('adding data files...')
    evttags = []
    for i in range(len(datain)):
	print('file '+str(i+1)+' of '+str(len(datain)))
	evttags = addinputfile(datahistlist,datain,args['treename'],i,args['variables'],
				False,True,args['normalization'],args['lumi'],evttags,
				args['doextraselection'])
    if args['npfromdata']:
	evttags = []
	print('adding non-prompt data files...')
	for i in range(len(npfromdatain)):
	    print('file '+str(i+1)+' of '+str(len(npfromdatain)))
	    evttags = addinputfile(npfromdatahistlist,npfromdatain,args['treename'],i,args['variables'],
				    True,True,args['normalization'],args['lumi'],evttags,
				    args['doextraselection'])
	print('adding prompt correction files...')
	for i in range(len(pcorrectionin)):
	    print('file '+str(i+1)+' of '+str(len(pcorrectionin)))
	    addinputfile(pcorrectionhistlist,pcorrectionin,args['treename'],i,args['variables'],
				    True,False,args['normalization'],args['lumi'],evttags,
				    args['doextraselection'])

    ### Merge histograms with same process name
    mergemchistograms(mchistlist)
    mergedatahistograms(datahistlist,'data')
    if args['npfromdata']:
	mergedatahistograms(npfromdatahistlist,'nonprompt')
	mergedatahistograms(pcorrectionhistlist,'pcorrection')
	for k in range(len(npfromdatahistlist)):
	    npfromdatahistlist[k].Add(pcorrectionhistlist[k])
    # put data to 0 if not using it
    if not args['usedata']:
	print('resetting data to 0...')
	for hist in datahistlist:
	    hist.Reset()

    ### Normalize if needed
    if(args['normalization']==2):
        normalizemctodata(mchistlist,datahistlist,args['variables'])

    ### Write histograms to file
    savehistograms(mchistlist,datahistlist,npfromdatahistlist,histfile,args['normalization'],args['lumi']) 
