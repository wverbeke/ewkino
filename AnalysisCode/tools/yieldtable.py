#############################################
# get event yields from trees or histograms #
#############################################
import ROOT
import os
import sys
import smalltools as tls
sys.path.append(os.path.abspath('../samplelists'))
from extendsamplelist import extendsamplelist

##### help functions for tree mode #####

def mcfileyield(rootfile,treename,lumi=0.,xsec=0.,hcountername='blackJackAndHookers/hCounter'):
    ### determine event yield for a single root tree with given parameters
    # input args:
    # - a path to a single root file
    # - the name of the tree within that root file
    # - luminosity and cross section (not strictly needed if tree contains fully normalized weights)
    # - name of hCounter object within the root file
    f = ROOT.TFile.Open(rootfile)
    res = {'nEntries':0.,'nEvents':0.,'sumWeights':0.,'hCounter':0.,'nWeightedEvents':0.}
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
    res['nEntries']=tree.GetEntries(); res['nEvents']=sumweights*norm; 
    res['sumWeights']=sumweights; res['hCounter']=hcount
    try: testnw = getattr(tree,'_normweight')
    except: print('### WARNING ###: tree does not seem to have normalized weights'); return res
    h2 = ROOT.TH1F("h2","h2",1,1,2)
    h2.StatOverflows(ROOT.kTRUE) # use overflow bins for getmean and getentries!
    tree.Draw("_normweight>>h2", "", "goff")
    snw = h2.GetMean()*h2.GetEntries()
    res['nWeightedEvents']=snw
    f.Close()
    return res

def datafileyield(rootfile,treename,evttags=[]):
    ### determine event yield for a single root tree with given parameters
    # only events with run/ls/nb combination not in evttags are counted 
    # (use evttags=-1 to count all entries)
    # input arguments:
    # - a path to a single root file
    # - the name of the tree within that root file
    # - list of event tags to exclude
    f = ROOT.TFile.Open(rootfile)
    res = {'nEntries':0.,'evtTags':evttags}
    # check if tree exists
    try: tree = f.Get(treename); tree.GetEntry(0)
    except: print('### ERROR ###: tree not found.'); return res
    # allow fast bypass:
    if evttags == -1:
	res['nEntries'] = tree.GetEntries()
	return res
    # else loop over entries
    for entry in range(tree.GetEntries()):
	tree.GetEntry(entry)
	evtid = str(getattr(tree,'_runNb'))
	evtid += '/'+str(getattr(tree,'_lumiBlock'))
	evtid += '/'+str(getattr(tree,'_eventNb'))
	if evtid not in evttags:
	    res['nEntries'] += 1
	    res['evtTags'].append(evtid)
    f.Close()
    return res

def samplelistyield(filedictlist,treename,year,lumi,dtype,doprint=True):
    ### determine the yield of all samples corresponding to a give sample list and directory
    # input arguments:
    # - a list of file dictionaries, which is of the same format as the output of extendsamplelist
    # - the name of the tree within each file
    # - year and luminosity
    # - data type ('data' or 'MC')
    # output object:
    #   a copy of filedictlist which has been extended with an additional entry in the dicts
    #   each dict now contains an entry 'yield' which is itself a dict as given by mcfileyield or datafileyield
    evttags = []
    for filedict in filedictlist:
        sname = filedict['sample_name']
	dct = {}
        if dtype=='MC':
            dct = mcfileyield(filedict['file'],treename,lumi=lumi,xsec=filedict['cross_section'])
        elif dtype=='data':
            dct = datafileyield(filedict['file'],treename,evttags)
	    evttags = dct['evtTags']
	else:
	    print('dtype not recognized: "{}"'.format(dtype))
	filedict['yield'] = dct
    if doprint:
	print('--- Event yield table for '+year+' '+dtype+' '+treename+' ---')
	formatsamplelistyield(filedictlist,dtype)
    return filedictlist

def formatsamplelistyield(filedictlist,dtype):
    ### print the output of samplelistyield to screen
    wcol1 = '40'
    wcol2 = '15'
    wcol3 = '10'
    wcol4 = '15'
    wcol5 = '15'
    wcol6 = '15'
    wcol7 = '25'
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
    nmctotweighted = 0.
    ndatatot = 0
    for filedict in filedictlist:
	sname = filedict['sample_name']
	if len(sname)>int(wcol1)-1: sname = sname[:int(wcol1)-5]
	info = str('{:<'+wcol1+'}').format(sname)
	info += str('{:<'+wcol2+'}').format('{0:.5E}'.format(filedict['cross_section']))
	if dtype=='MC':
	    info += str('{:<'+wcol3+'}').format(filedict['yield']['nEntries'])
	    info += str('{:<'+wcol4+'}').format('{0:.3E}'.format(filedict['yield']['sumWeights']))
	    info += str('{:<'+wcol5+'}').format('{0:.3E}'.format(filedict['yield']['hCounter']))
	    info += str('{:<'+wcol6+'}').format('{0:.3E}'.format(filedict['yield']['nEvents']))
	    info += str('{:<'+wcol7+'}').format('{0:.3E}'.format(filedict['yield']['nWeightedEvents']))
	    nmctot += filedict['yield']['nEvents']
	    nmctotweighted += filedict['yield']['nWeightedEvents']
	elif dtype=='data':
	    info += str('{:<'+wcol3+'}').format(filedict['yield']['nEntries'])
	    ndatatot += filedict['yield']['nEntries']
	print(info)
    print('-'*width)
    info = str('{:<'+wcol1+'}').format('Total:')
    if dtype=='MC':
	info += str('{:<'+wcol2+'}{:<'+wcol3+'}{:<'+wcol4+'}{:<'+wcol5+'}').format('','','','')
	info += str('{:<'+wcol6+'}').format('{0:.3E}'.format(nmctot))
	info += str('{:<'+wcol7+'}').format('{0:.3E}'.format(nmctotweighted))
    else:
	info += str('{:<'+wcol2+'}').format('')
	info += str('{:<'+wcol3+'}').format(ndatatot)
    print(info)
    print('_'*width)


##### helper functions for histogram mode #####

def histogramyield(hist):
    ### determine the yield of a single histogram
    # input arguments:
    # - a TH1 object
    return {'name':hist.GetName(),'yield':hist.Integral()}

def histogramfileyield(histfile,mustcontain=[],doprint=True):
    ### determine the yield of all histograms in histfile
    # if mustcontain is not empty, histogram names are required to contain all elements in it
    histlist = tls.loadallhistograms(histfile,mustcontain)
    yieldlist = []
    for hist in histlist:
        yieldlist.append(histogramyield(hist))
    if doprint:
	print('--- Event yield table for '+histfile+' '+str(mustcontain)+' ---')
	formathistogramfileyield(yieldlist)
    return yieldlist

def formathistogramfileyield(yieldlist):
    ### print the output of histogramfileyield to screen
    wcol1 = '40'
    wcol2 = '25'
    info = str('{:<'+wcol1+'}{:<'+wcol2+'}').format('Name','Yield (integral)')
    width = len(info)
    print('_'*width)
    print(info)
    print('-'*width)
    nmctot = 0.
    ndatatot = 0
    for yielddict in yieldlist:
        name = yielddict['name']
        if len(name)>int(wcol1)-1: name = name[:int(wcol1)-5]
        info = str('{:<'+wcol1+'}').format(name)
        info += str('{:<'+wcol2+'}').format('{0:.5E}'.format(yielddict['yield']))
        print(info)
    print('-'*width)
    info = str('{:<'+wcol1+'}').format('Total:')
    #if dtype=='MC':
    #    info += str('{:<'+wcol2+'}{:<'+wcol3+'}{:<'+wcol4+'}{:<'+wcol5+'}').format('','','','')
    #    info += str('{:<'+wcol6+'}').format('{0:.3E}'.format(nmctot))
    #    info += str('{:<'+wcol7+'}').format('{0:.3E}'.format(nmctotweighted))
    #else:
    #    info += str('{:<'+wcol2+'}').format('')
    #    info += str('{:<'+wcol3+'}').format(ndatatot)
    print(info)
    print('_'*width)

def formatlatextable(table,rows=[],columns=[]):
    ### format a latex table
    # (not yet tested!)
    # input args:
    # - table is a dict of dicts, of which each element represents a row
    #   dict keys are assumed to be the same for each dict, they represent the columns
    # - rows and columns are lists of strings, optional to specify the order of the labels
    rowkeys = rows
    if len(rows)==0: rowkeys = sorted(table.keys())
    colkeys = columns
    if len(columns)==0: colkeys = sorted(table[rowkeys[0]].keys())
    t = r'\begin{table}[h]'+'\n'
    t += '\t'+r'\begin{center}'+'\n'
    t += '\t\t'+r'\caption{}'+'\n'
    t += '\t\t'+r'\label{}'+'\n'
    t += '\t\t'+r'\begin{tabular}{|l|'
    for s in ['c|']*len(colkeys): t += s
    t += '}\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t\t'+r'process '
    for s in colkeys: t += ' & '+s
    t += r' \\'+'\n'
    t += '\t\t\t'+r'\hline \hline'+'\n'
    for rowkey in rowkeys:
	t += '\t\t\t'+rowkey
	for colkey in colkeys:
	    t += ' & '+str(table[rowkey][colkey])
	t += r' \\'+'\n'
    t += '\t\t\t'+r'\hline'+'\n'
    t += '\t\t'+r'\end{tabular}'+'\n'
    t += '\t'+r'\end{center}'+'\n'
    t += r'\end{table}'
    return t

if __name__=="__main__":

    if( len(sys.argv)<2 or sys.argv[1].split('=')[0]!='mode' ):
	print('### ERROR ###: wrong command line args.')
	print('Use like this: python yieldtable.py mode=< mode > < additional args >')
	sys.exit()

    mode = sys.argv[1].split('=')[1]

    if mode=='tree':
	print('mode = tree, required args: <filename> <treename> <dtype>')
	rootfile = sys.argv[2]
	treename = sys.argv[3]
	dtype = sys.argv[4]
	yielddict = {}
	if dtype=='MC': yielddict = mcfileyield(rootfile,treename)
	elif dtype=='data': yielddict = datafileyield(rootfile,treename,[])
	totdict = {}
	totdict['sample_name'] = rootfile.split('/')[-1].replace('.root','')
	totdict['cross_section'] = -1
	totdict['yield'] = yielddict
	formatsamplelistyield([totdict],dtype)

    elif mode=='samplelist':
	print('mode = samplelist, required args: <samplelist> <directory> <treename>')	
	(year,lumi) = tls.year_and_lumi_from_samplelist(sys.argv[2])
	dtype = tls.data_type_from_samplelist(sys.argv[2])
	flist = extendsamplelist(sys.argv[2],sys.argv[3])
	treename = sys.argv[4]
	samplelistyield(flist,treename,year,lumi,dtype,doprint=True)

    elif mode=='histogram':
	print('mode = histogram, required args: <histogramfile>')
	print('                  optional args: <tags of histogram names to consider>')
	histfile = sys.argv[2]
	mustcontain = []
	for tag in sys.argv[3:]:
	    mustcontain.append(tag)
	histogramfileyield(histfile,mustcontain=mustcontain)

    elif mode=='histogramSR':
	print('mode = histogramSR, required args: <topdir> <npmode>')
	# note: depends on naming convention of the files and folders!
	topdir = os.path.abspath(sys.argv[2])
	npmode = sys.argv[3]
	mustcontain = ['nominal','yield']
	for yeardir in [year+'combined' for year in ['2016','2017','2018']]:
	    signalregions = ['signalregion_'+str(i) for i in [1,2,3]]
	    table = {}
	    for signalregiondir in signalregions:
		totdir = os.path.join(topdir,yeardir,signalregiondir,npmode,'combined.root')
		yieldlist = histogramfileyield(totdir,mustcontain=mustcontain)
		yielddict = {}
		totmc = 0.
		for el in yieldlist:
		    name = el['name'].split('_')[0]
		    yielddict[name] = '{:.2f}'.format(el['yield'])
		    if not name=='data': totmc += el['yield']
		yielddict['total simulation'] = '{:.2f}'.format(totmc)
		table[signalregiondir] = yielddict
	    rows = table.keys()
	    columns = table[rows[0]].keys()
	    # bring tZq to front
	    columns.insert(0, columns.pop(columns.index('tZq')))
	    # bring total simulation to front
            columns.insert(0, columns.pop(columns.index('total simulation')))
	    # bring data to front
            columns.insert(0, columns.pop(columns.index('data')))
	    print(formatlatextable(table,rows=rows,columns=columns))
