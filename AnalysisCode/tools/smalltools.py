########################################
# some small functions for general use #
########################################
import ROOT
import numpy as np
from array import array

def year_and_lumi_from_samplelist(samplelist_name):
    # return year (string) and luminosity (float) associated to a sample list
    # WARNING: depends on naming convention of sample lists.
    slname = samplelist_name
    if '/' in slname: slname = slname[slname.rfind('/'):]
    year = ''
    lumi = -1.
    if '2016' in slname: year = '2016'; lumi = 35900
    elif '2017' in slname: year = '2017'; lumi = 41500
    elif '2018' in slname: year = '2018'; lumi = 59700
    else:
	print('### WARNING ###: year not recognized from samplelist '+str(slname))
    return (year,lumi)

def data_type_from_samplelist(samplelist_name):
    # determine whether samplelist contains data or MC files
    # WARNING: depends on naming convention of sample lists.
    slname = samplelist_name
    if '/' in slname: slname = slname[slname.rfind('/'):]
    dtype = ''
    if 'MC' in slname: dtype = 'MC'
    elif 'data' in slname: dtype = 'data'
    else:
	print('### WARNING ###: data type not recognized from samplelist '+str(slname))
    return dtype

def isdata_from_filepath(filepath):
    # determine whether a file represents data
    if('2016data' in filepath or '2017data' in filepath or '2018data' in filepath): return True
    return False
    
def year_from_filepath(filepath):
    # determine the year based on the path to the file
    if('Summer16' in filepath): return '2016'
    elif('Fall17' in filepath): return '2017'
    elif('Autumn' in filepath): return '2018'
    else: print('### WARNING ###: cannot determine year from path to file: '+filepath)
    return None

def subselect_inputfiles(inputfiles,selection_type):
    # remove elements from inputfiles if not needed for a given selection_type
    # inputfiles is a list of dicts, e.g. given as output by extendsamplelist
    # inputfiles is assumed to consist of only data or only simulation, no mixture.
    isdata = inputfiles[0]['process_name']=='data'
    if selection_type=='3tight':
	# remove ZG as it is fully included in DY
        return [f for f in inputfiles if not 'ZGToLLG' in f['file']]
    if selection_type=='3prompt':
	# only prompt simulation needed here
	if isdata:
	    print('### WARNING ###: trying to run 3prompt selection on data, which does not make sense.')
	    print('                 run 3tight selection instead, which is equivalent for data.')
	    return None
	return [f for f in inputfiles if f['process_name']!='nonprompt']
    if selection_type=='fakerate':
	# data and prompt simulation needed in this case
	return [f for f in inputfiles if f['process_name']!='nonprompt']
    if selection_type=='2tight':
	# only nonprompt simulation needed here
	if isdata:
	    print('### WARNING ###: trying to run 2tight selection on data...')
	    print('                 run fakerate selection instead.')
	    return None
	return [f for f in inputfiles if f['process_name']=='nonprompt']
    print('### ERROR ###: selection_type not recognized: '+selection_type)
    return None

def loadallhistograms(histfile,mustcontain=[]):
    ### read a root file containing histograms and load all histograms to a list
    # if mustcontain is not empty, histogram names are required to contain all elements in it.
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        # check if histogram is readable
        try:
            nentries = hist.GetEntries() # maybe replace by more histogram-specific function
	    nbins = hist.GetNbinsX()
	    hist.SetDirectory(0)
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
	    continue
	keep = True
	if len(mustcontain)>0:
	    for tag in mustcontain:
		if not tag in hist.GetName(): keep = False; break
	if not keep: continue
        # add hist to dict
        histlist.append(hist)
    f.Close()
    return histlist

def subselect(stringlist,tagstodiscard=[],tagstokeep=[]):
    ### generic function to subselect strings from a list of strings
    sellist = []
    for s in stringlist:
        # if tags to keep are specified, check if any is there, else discard
        if(len(tagstokeep)>0):
            keep = False
            for t in tagstokeep:
                if t in s: keep = True; break
            if not keep: continue
        # for all tags to discard, check if none is there, else discard
        keep = True
        for t in tagstodiscard:
            if t in s: keep = False; break
        if not keep: continue
        sellist.append(s)
    return sellist

def tgraphtohist( graph ):
    
    # get list of x values and sort them
    xvals = []
    for i in range(graph.GetN()): xvals.append(graph.GetX()[i])
    xvals = np.array(xvals)
    sortedindices = np.argsort(xvals)
    # make bins
    bins = []
    for i in sortedindices: bins.append(graph.GetX()[i]-graph.GetErrorXlow(i))
    bins.append(graph.GetX()[i]+graph.GetErrorXhigh(i))
    # make histogram
    hist = ROOT.TH1D("","",len(bins)-1,array('f',bins))
    # set bin contents
    for i in range(1,hist.GetNbinsX()+1):
	bincontent = graph.GetY()[sortedindices[i-1]]
	binerror = max(graph.GetErrorYlow(sortedindices[i-1]),
			graph.GetErrorYhigh(sortedindices[i-1]))
	hist.SetBinContent(i,bincontent)
	hist.SetBinError(i,binerror)
    hist.SetName(graph.GetName())
    hist.SetTitle(graph.GetTitle())
    return hist

### test section ###
if __name__ == "__main__":
    testname = 'samplelist_tzq_2016_MC'
    print(year_and_lumi_from_samplelist(testname))
    print(data_type_from_samplelist(testname))
