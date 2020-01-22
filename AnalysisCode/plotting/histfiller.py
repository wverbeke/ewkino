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
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist

def getinputfiles(rootdir,samplelist,isdata=True):
    ### get list of input files in rootdir whose names (and cross-sections) are in samplelist
    infiles = []
    # first define some kind of appendix to have the correct filename; possibly to be replaced by cleaner method
    appendix = ''
    if '2016_MC' in samplelist: appendix = '_Summer16'
    if '2017_MC' in samplelist: appendix = '_Fall17'
    if '2018_MC' in samplelist: appendix = '_Autumn18'
    if '2016_data' in samplelist: appendix = '_Run2016' # (?)
    if '2017_data' in samplelist: appendix = '_Run2017' # (?)
    if '2018_data' in samplelist: appendix = '_Run2018' # (?)
    # read process names and cross sections from the sample list
    samples_dict = readsamplelist(samplelist)
    # find available samples in input directory and compare with sample list
    samples_available = glob.glob(rootdir+'/*.root')
    for sample in samples_dict:
        filename = os.path.join(rootdir,sample['sample_name']+appendix+'.root')
        if not os.path.exists(filename):
            print('### WARNING ###: this file was requested but does not seem to exist:')
            print('                 '+filename)
            continue
        if isdata:
            infiles.append({'file':filename,'sample_name':sample['sample_name']+appendix,
                            'process_name':sample['process_name']})
        else:
            infiles.append({'file':filename,'sample_name':sample['sample_name']+appendix,
                            'process_name':sample['process_name'],'xsection':sample['cross_section']})
        if filename in samples_available:
            samples_available.remove(filename)
    if len(samples_available)>0:
        print('### WARNING ###: these files are present in input directory but not used:')
        for filename in samples_available:
            print('                 '+filename)
    return infiles

def initializehistograms(mcin,datain,variables):
    # initialize histograms for all variables and files
    # (2D lists: dimension 1: variables, dimension 2: files)
    mchistlist = []
    datahistlist = []
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
    return (mchistlist,datahistlist)

def addinputfile(histlist,filelist,index,variables,isdata,normalization,lumi):
    f = ROOT.TFile.Open(filelist[index]['file'])
    if(not isdata and not normalization==0):
        sumweights = f.Get("blackJackAndHookers/hCounter").GetSumOfWeights()
        xsection = filelist[index]['xsection']
    else:
        sumweights=1; xsection=1; lumi=1
    tree = f.Get("blackJackAndHookers/"+treename)
    for j in range(int(tree.GetEntries()/1.)):
        if(j%5000==0):
            percent = j*100/tree.GetEntries()
            sys.stdout.write("\r"+str(percent)+'%')
            sys.stdout.flush()
        tree.GetEntry(j)
        weight = 1.
        if(not isdata and not normalization==0):
            weight = getattr(tree,'_weight')
        for k,vardict in enumerate(variables):
            varname = vardict['name']
            varvalue = getattr(tree,varname)
            bins = vardict['bins']
            if(varvalue<bins[0] or varvalue>bins[-1]): continue
            histlist[k][index].Fill(varvalue,weight*lumi*xsection/sumweights)
    f.Close()
    sys.stdout.write("\r"+'100%'+"\n")
    sys.stdout.flush()

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

def mergedatahistograms(datahistlist):
    # merge all data histograms
    print('merging data histograms...')
    for k in range(len(datahistlist)):
        firsthist = datahistlist[k][0]
        newhist = firsthist.Clone()
        for hist in datahistlist[k][1:]:
            newhist.Add(hist)
        newhist.SetTitle('data')
        newhist.SetName(firsthist.GetName()[:firsthist.GetName().rfind("_")+1]+str(0))
        datahistlist[k] = newhist

def savehistograms(mchistlist,datahistlist,histfile,normalization,lumi):
    print('writing histograms to file...')
    f = ROOT.TFile.Open(histfile,"recreate")
    for histset in mchistlist:
        for hist in histset:
            hist.Write(hist.GetName())
    for hist in datahistlist:
        hist.Write(hist.GetName())
    normalization_st = ROOT.TVectorD(1); normalization_st[0] = normalization
    normalization_st.Write("normalization")
    lumi_st = ROOT.TVectorD(1)
    lumi_st[0] = lumi
    lumi_st.Write("lumi")
    f.Close()

if __name__ == '__main__':
    # create histogram objects for an arbitrary number of variables
    # using a collection of root files in a single sample list, directory and tree.
  
    ### Configure input parameters (hard-coded)
    # folder to read mc root files from
    mcrootdir = '/user/llambrec/Files/test'
    # folder to read data root files from
    datarootdir = '/user/llambrec/Files/test'
    # samplelist for simulation with process names and cross sections
    mcsamplelist = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2016_MC.txt'
    # samplelist for data with dataset names
    datasamplelist = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2016_MC.txt' 
    # (use MC for testing, data histogram will be reset later on)
    # file to store histograms in:
    histfile = '/user/llambrec/ewkino/AnalysisCode/plotting/histograms/test.root'
    # list of dicts of variables to plot with corresponding bins:
    variables = [
        {'name':'_Mjj_max','bins':list(np.linspace(0,1200,num=21))}
    ]
    # name of tree to read for each input file:
    treename = 'treeCat1'
    # normalization:
    normalization = 1
    # (normalization parameter: 0 = no normalization, weights, xsection and lumi set to 1.)
    # (             1 = normalize using weights, xsection and lumi.)
    # (             2 = same as before but apply normalization to data afterwards.)
    # luminosity in inverse picobarns
    lumi = 35900.

    ### Make all paths global
    mcrootdir = os.path.abspath(mcrootdir)
    datarootdir = os.path.abspath(datarootdir)
    mcsamplelist = os.path.abspath(mcsamplelist)
    datasamplelist = os.path.abspath(datasamplelist)
    histfile = os.path.abspath(histfile)

    ### Overwrite output file
    if os.path.exists(histfile): os.system('rm '+histfile)
    histdir = histfile[:histfile.rfind('/')]
    if not os.path.exists(histdir): os.makedirs(histdir)

    ### Configure input files
    mcin = getinputfiles(mcrootdir,mcsamplelist,False)
    #print(mcin)
    datain = getinputfiles(datarootdir,datasamplelist,True)

    ### Fill histograms
    mchistlist,datahistlist = initializehistograms(mcin,datain,variables)
    print('adding simulation files...')
    for i in range(len(mcin)):
        print('file '+str(i+1)+' of '+str(len(mcin)))
        addinputfile(mchistlist,mcin,i,variables,False,normalization,lumi)
    print('adding data files...')
    for i in range(len(datain)):
        print('file '+str(i+1)+' of '+str(len(datain)))
        addinputfile(datahistlist,datain,i,variables,True,normalization,lumi)

    ### Merge histograms with same process name
    mergemchistograms(mchistlist)
    mergedatahistograms(datahistlist)
    # reset data hist for testing mc only
    for hist in datahistlist:
        hist.Reset()

    ### Normalize if needed
    if(normalization==2):
        normalizemctodata(mchistlist,datahistlist,variables)

    ### Write histograms to file
    savehistograms(mchistlist,datahistlist,histfile,normalization,lumi) 
