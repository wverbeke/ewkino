###########################################################
# get event yields of all root trees in a given directory #
###########################################################
# command line args: <directory> <treename>

import ROOT
import os
import sys
import smalltools as tls
sys.path.append(os.path.abspath('../samplelists'))
from extendsamplelist import extendsamplelist

def mcfileyield(rootfile,treename,lumi,xsec,hcountername='blackJackAndHookers/hCounter'):
    ### determine event yield for a single root tree with given parameters
    f = ROOT.TFile.Open(rootfile)
    res = {'nentries':0.,'nevents':0.,'sumweights':0.,'hcounter':0.,'snw':0.}
    # check if tree exists
    try: tree = f.Get(treename); tree.GetEntry(0)
    except: print('### ERROR ###: tree not found.'); return res
    # get sum of weights and norm factor
    try: testweight = getattr(tree,'_weight')
    except: print('### ERROR ###: tree supposed to be MC but has no "_weight"'); return res
    h = ROOT.TH1F("h","h",1,1,2)
    h.StatOverflows(ROOT.kTRUE) # use overflow bins for getmean and getentries!
    tree.Draw("_weight>>h", "", "goff")
    sumweights = h.GetMean()*h.GetEntries()
    try: hcount = f.Get(hcountername).GetSumOfWeights()
    except: print('### ERROR ###: tree supposed to be MC but has no hCounter'); return res
    norm = lumi*xsec/hcount
    res['nentries']=tree.GetEntries(); res['nevents']=sumweights*norm; 
    res['sumweights']=sumweights; res['hcounter']=hcount
    try: testnw = getattr(tree,'_normweight')
    except: print('### WARNING ###: tree does not seem to have normalized weights'); return res
    h2 = ROOT.TH1F("h2","h2",1,1,2)
    h2.StatOverflows(ROOT.kTRUE) # use overflow bins for getmean and getentries!
    tree.Draw("_normweight>>h2", "", "goff")
    snw = h2.GetMean()*h2.GetEntries()
    res['snw']=snw
    return res

def datafileyield(rootfile,treename,evttags=[]):
    ### determine event yield for a single root tree with given parameters
    ### Only events with run/ls/nb combination not in evttags are counted (use evttags=-1 to count all entries)
    f = ROOT.TFile.Open(rootfile)
    # check if tree exists
    try: tree = f.Get(treename); tree.GetEntry(0)
    except: print('### ERROR ###: tree not found.'); return (0,evttags)
    # allow fast bypass:
    if evttags == -1: return tree.GetEntries()
    # else loop over entries
    nentries = 0
    for entry in range(tree.GetEntries()):
	tree.GetEntry(entry)
	evtid = str(getattr(tree,'_runNb'))
	evtid += '/'+str(getattr(tree,'_lumiBlock'))
	evtid += '/'+str(getattr(tree,'_eventNb'))
	if evtid not in evttags:
	    nentries += 1
	    evttags.append(evtid)
    return (nentries,evttags)

def formatoutput(filedictlist,treename,year,lumi,dtype):
    # filedictlist is supposed to be in the same format as the output of extendsamplelist
    wcol1 = '40'
    wcol2 = '15'
    wcol3 = '10'
    wcol4 = '15'
    wcol5 = '15'
    wcol6 = '15'
    wcol7 = '25'
    print('--- Event yield table for '+year+' '+dtype+' '+treename+' ---')
    info = str('{:<'+wcol1+'}{:<'+wcol2+'}').format('Sample','Cross-section')
    info += str('{:<'+wcol3+'}{:<'+wcol4+'}').format('Entries','Sum of weights')
    info += str('{:<'+wcol5+'}{:<'+wcol6+'}').format('hCounter','nEvents')
    info += str('{:<'+wcol7+'}').format('Sum of normed weights')
    width = len(info)
    print('_'*width)
    print(info)
    print('-'*width)
    evttags = []
    nmctot = 0.
    ndatatot = 0
    for filedict in filedictlist:
	sname = filedict['sample_name']
	if len(sname)>int(wcol1)-1: sname = sname[:int(wcol1)-5]
	info = str('{:<'+wcol1+'}').format(sname)
	info += str('{:<'+wcol2+'}').format('{0:.5E}'.format(filedict['cross_section']))
	if dtype=='MC':
	    dct = mcfileyield(filedict['file'],treename,lumi,filedict['cross_section'])
	    #info += str('{:<'+wcol3+'}').format('{0:.5E}'.format(dct['nentries']))
	    info += str('{:<'+wcol3+'}').format(dct['nentries'])
	    info += str('{:<'+wcol4+'}').format('{0:.3E}'.format(dct['sumweights']))
	    info += str('{:<'+wcol5+'}').format('{0:.3E}'.format(dct['hcounter']))
	    info += str('{:<'+wcol6+'}').format('{0:.3E}'.format(dct['nevents']))
	    info += str('{:<'+wcol7+'}').format('{0:.3E}'.format(dct['snw']))
	    nmctot += dct['snw']
	else:
	    nevents,evttags = datafileyield(filedict['file'],treename,evttags)
	    info += str('{:<'+wcol3+'}').format(nevents)
	    ndatatot += nevents
	print(info)
    print('-'*width)
    info = str('{:<'+wcol1+'}').format('Total:')
    if dtype=='MC':
	info += str('{:<'+wcol2+'}{:<'+wcol3+'}{:<'+wcol4+'}{:<'+wcol5+'}').format('','','','')
	info += str('{:<'+wcol6+'}').format('{0:.3E}'.format(nmctot))
    else:
	info += str('{:<'+wcol2+'}').format('')
	info += str('{:<'+wcol3+'}').format(ndatatot)
    print(info)
    print('_'*width)

if __name__=="__main__":
    if not len(sys.argv) == 4:
	print('### ERROR ###: wrong number of command line arguments.')
	print('Use like this: python yieldtable.py < samplelist > < directory > < category >')
	sys.exit()
    treename = 'blackJackAndHookers/'+sys.argv[3]
    (year,lumi) = tls.year_and_lumi_from_samplelist(sys.argv[1])
    dtype = tls.data_type_from_samplelist(sys.argv[1])
    flist = extendsamplelist(sys.argv[1],sys.argv[2])
    formatoutput(flist,treename,year,lumi,dtype)
