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
    samples_to_use = []
    cross_sections = []
    with open(samplelist) as f:
        for line in f:
            if(line[0] != '#' and len(line)>1):
                line = line.split(' ')
                samples_to_use.append(line[0].rstrip('\n'))
                if len(line)>1:
                    cross_sections.append(float(line[1].rstrip('\n')))
                else:
                    cross_sections.append(0.)
    # find available samples in input directory and compare with sample list
    samples_available = glob.glob(rootdir+'/*.root')
    for sample,xsection in zip(samples_to_use,cross_sections):
        filename = os.path.join(rootdir,sample+appendix+'.root')
        if not os.path.exists(filename):
            print('### WARNING ###: this file was requested but does not seem to exist:')
            print('                 '+filename)
            continue
        if isdata:
            infiles.append('file':filename,'label':r'data')
        else:
            infiles.append({'file':filename,'label':r'simulation','xsection':xsection})
        if filename in samples_available:
            samples_available.remove(filename)
    if len(samples_available)>0:
        print('### WARNING ###: these files are present in input directory but not used:')
        for filename in samples_available:
            print('                 '+filename)
    return infiles

def initializehistograms(mcin,datain,varnames):
    # initialize histograms for all variables and files
    # (2D lists: dimension 1: variables, dimension 2: files)
    mchistlist = []
    datahistlist = []
    for varname in varnames:
        bins = varnames[varname]
        nbins = len(bins)-1
        bins = array('f',bins)
        mchistlist.append([])
        datahistlist.append([])
        for j in range(len(mcin)):
            mchistlist[i].append(ROOT.TH1F("mc_"+varname+'_'+str(j),mcin[j]['label'],nbins,bins))
            mchistlist[i][j].SetDirectory(0)
        for j in range(len(datain)):
            datahistlist[i].append(ROOT.TH1F("data_"+varname+')'+str(j),datain[j]['label'],nbins,bins))
            datahistlist[i][j].SetDirectory(0)
    return (mchistlist,datahistlist)

def addinputfile(histlist,filelist,index,varnames,isdata,normalization,lumi):
    f = ROOT.TFile.Open(filelist[index]['file'])
    sumweights=1; xsection=1; lumi=1
    if(not isdata and not normalization==0):
        sumweights = f.Get("hCounter").GetSumOfWeights()
        xsection = filelist[index]['xsection']
    tree = f.Get(treename)
    # (for testing purposes: use only a fracion of the data !!!)
    for j in range(int(tree.GetEntries()/1.)):
        if(j%5000==0):
            percent = j*100/tree.GetEntries()
            sys.stdout.write("\r"+str(percent)+'%')
            sys.stdout.flush()
        tree.GetEntry(j)
        weight = 1.
        if(not isdata and not normalization==0):
            weight = getattr(tree,'_weight')
        for k,varname in enumerate(varnames):
            varvalue = getattr(tree,varname)
            bins = varnames[varname]
            if(varvalue<bins[0] or varvalue>bins[-1]): continue
            hist[k][index].Fill(varvalue,weight*lumi*xsection/sumweights)
    f.Close()
    sys.stdout.write("\r"+'100%'+"\n")
    sys.stdout.flush()

if __name__ == '__main__':
  
    ### Configure input parameters (hard-coded)
    # folder to read root files from
    rootdir = '/user/llambrec/Files/signalregion'
    # samplelist for simulation with process names and cross sections
    mcsamplelist = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2016_MC.txt'
    # samplelist for data with dataset names
    datasamplelist = '/user/llambrec/ewkino/AnalysisCode/samplelists/samplelist_tzq_2016_MC.txt' # use MC for test
    # folder to store histograms in:
    histdir = '/user/llambrec/ewkino/AnalysisCode/plotting/histograms'
    # dict of variables to plot with corresponding bins:
    varnames = {
        '_Mjj_max':list(np.linspace(0,1200,num=20))
        }
    # name of tree to read for each input file:
    treename = 'treeCat1'
    # normalization:
    normalization = 2
    # (normalization parameter: 0 = no normalization, weights, xsection and lumi set to 1.)
    # (             1 = normalize using weights, xsection and lumi.)
    # (             2 = same as before but apply normalization to data afterwards.)
    # luminosity in inverse picobarns
    lumi = 35900.

    ### Configure input files
    mcin = getMCin(rootdir,mcsamplelist,False)
    print(mcin)
    datain = getinputfiles(rootdir,datasamplelist,True)

    ### Fill histograms
    mchistlist,datahistlist = initializehistograms(mcin,datain,varnames)
    print('adding simulation files...')
    for i in range(len(mcin)):
        print('file '+str(i+1)+' of '+str(len(mcin)))
        addinputfile(mchistlist,mcin,i,varnames,False,normalization,lumi)
    print('adding data files...')
    for i in range(len(datain)):
        print('file '+str(i+1)+' of '+str(len(datain)))
    addinputfile(datahistlist,datain,i,varnames,True,normalization,lumi)

    
