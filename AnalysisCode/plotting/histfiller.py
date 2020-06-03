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

def initializehistograms(mcin,datain,npdatain,variables):
    # initialize histograms for all variables and files
    # (2D lists: dimension 1: variables, dimension 2: files)
    mchistlist = []
    datahistlist = []
    npdatahistlist = []
    for k,vardict in enumerate(variables):
        bins = vardict['bins']
        varname = vardict['name']
        nbins = len(bins)-1
        bins = array('f',bins)
        mchistlist.append([])
        datahistlist.append([])
        for j in range(len(mcin)):
            mchistlist[k].append(ROOT.TH1F("mc_"+varname+"_"+str(j),
                                 mcin[j]['process_name']+"/"+mcin[j]['sample_name'],nbins,bins))
            mchistlist[k][j].SetDirectory(0)
        for j in range(len(datain)):
            datahistlist[k].append(ROOT.TH1F("data_"+varname+"_"+str(j),
                                datain[j]['process_name']+"/"+datain[j]['sample_name'],nbins,bins))
            datahistlist[k][j].SetDirectory(0)
	if len(npdatain)>0:
	    npdatahistlist.append([])
	    for j in range(len(npdatain)):
		npdatahistlist[k].append(ROOT.TH1F("npdata_"+varname+"_"+str(j),
                                datain[j]['process_name']+"/"+datain[j]['sample_name'],nbins,bins))
		npdatahistlist[k][j].SetDirectory(0)
    return (mchistlist,datahistlist,npdatahistlist)

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
    for j in range(int(tree.GetEntries()/1.)):
        if(j%5000==0):
            percent = j*100/tree.GetEntries()
            sys.stdout.write("\r"+str(percent)+'%')
            sys.stdout.flush()
        tree.GetEntry(j)
	if doextraselection:
	    print('here')
	    if not passextraselection(tree): continue
	normweight = 1.
        if(usenormweight and not normalization==0):
	    normweight = getattr(tree,'_normweight')
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
	    # default filling method (deprecated, removed definition of parameters from func):
            #histlist[k][index].Fill(varvalue,weight*lumi*xsection/sumweights)
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
    return getattr(tree,'_eventBDT')>0

def input_from_cmd(arglist):
    # read command line arguments and return a dict.
    # the order of the arguments is fixed and no checking is done, but this can be generalized
    if not len(sys.argv)==11:
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
    return res

if __name__ == '__main__':
    # create histogram objects for an arbitrary number of variables
    # using a collection of root files in a single sample list, directory and tree.
  
    args = {}
    ### Configure input parameters (hard-coded)
    # folder to read mc root files from
    args['mcrootdir'] = '/user/llambrec/Files/tthid_tthmva_reduced/signalregion/2017MC_flat'
    # folder to read data root files from
    #args['datarootdir'] = '/user/llambrec/Files/tzqid/wzcontrolregion/2018data_flat'
    args['datarootdir'] = args['mcrootdir']
    # samplelist for simulation with process names and cross sections
    args['mcsamplelist'] = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2017_MC.txt'
    #args['mcsamplelist'] = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples/samplelist_tzq_2016_MC.txt'
    # samplelist for data with dataset names
    #args['datasamplelist'] = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2018_data.txt' 
    args['datasamplelist'] = args['mcsamplelist']
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
	#{'name':'_eventBDT','bins':list(np.linspace(-1,1,num=21))},
	{'name':'_nMuons','bins':list(np.linspace(-0.5,3.5,num=5))},
	{'name':'_nElectrons','bins':list(np.linspace(-0.5,3.5,num=5))},
	#{'name':'_leptonMVATOP_min','bins':list(np.linspace(-1,1,num=41))},
	#{'name':'_leptonMVAttH_min','bins':list(np.linspace(-1,1,num=41))}
    ]
    # name of tree to read for each input file:
    args['treename'] = 'treeCat1'
    # normalization:
    args['normalization'] = 1
    # (normalization parameter: 0 = no normalization, weights, xsection and lumi set to 1.)
    # (             1 = normalize using weights, xsection and lumi.)
    # (             2 = same as before but apply normalization to data afterwards.)
    # luminosity in inverse picobarns
    args['lumi'] = 41500.
    args['doextraselection'] = False

    ### Overwrite using cmd args
    if(len(sys.argv)==11):
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
    mcin = getinputfiles(mcrootdir,mcsamplelist)
    datain = getinputfiles(datarootdir,datasamplelist)
    #datain = datain[1:2] # TEMP FOR SPEED-UP WHEN NO DATA IS BEING USED
    npdatain = []
    npfromdata = False # later add as cmdline arg
    if npfromdata:
	npdatain = getinputfiles(os.path.join(datarootdir,'nonprompt_background'),
				    datasamplelist)
	mcin = removenonpromptmc(mcin)

    print('=== MC files ===')
    for s in mcin: print(s['sample_name'])
    print('=== data files ===')
    for s in datain: print(s['sample_name'])
    print('=== nonprompt data files ===')
    for s in npdatain: print(s['sample_name'])

    ### Fill histograms
    mchistlist,datahistlist,npdatahistlist = initializehistograms(mcin,datain,npdatain,args['variables'])
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
    if npfromdata:
	evttags = []
	print('adding non-prompt data files...')
	# note: 
	for i in range(len(npdatain)):
	    print('file '+str(i+1)+' of '+str(len(npdatain)))
	    evttags = addinputfile(npdatahistlist,npdatain,args['treename'],i,args['variables'],
				    True,True,args['normalization'],args['lumi'],evttags,
				    args['doextraselection'])

    ### Merge histograms with same process name
    mergemchistograms(mchistlist)
    mergedatahistograms(datahistlist,'data')
    mergedatahistograms(npdatahistlist,'nonprompt')
    # manually put data to zero for now
    #for hist in datahistlist:
    #	hist.Reset()

    ### Normalize if needed
    if(args['normalization']==2):
        normalizemctodata(mchistlist,datahistlist,variables)

    ### Write histograms to file
    savehistograms(mchistlist,datahistlist,npdatahistlist,histfile,args['normalization'],args['lumi']) 
