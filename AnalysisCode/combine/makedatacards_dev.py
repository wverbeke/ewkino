##################################################################
# attempt at automatically generating datacards for combine tool #
##################################################################
import os
import sys
import ROOT

def readhistfile(histfile,variable,doprint=True):
    ### extract unique processes and systematics from an input root file
    ### containing the shapes, as will be used by combine to do the fit.
    ### note that this function depends on the naming convention of the histograms.
    ### output is a dict of dicts matching processes to idnumbers, yields and systematic impacts.
    ### e.g. {'tZq':{'id':0,'yield':30.2,'systematics':{'JEC':1}}, 'WZ': ... }

    # open root file and read processes with systematics
    f = ROOT.TFile.Open(histfile)
    pinfo = {}
    plist = []
    slist = []
    counter = 1
    keylist = f.GetListOfKeys()
    # temporary: define a limited list of systematics to ignore
    excludesystematics = []
    # loop over all histograms in the file
    for key in keylist:
        hist = f.Get(key.GetName())
	histname = hist.GetName()
	# select only histograms of the requested variable
	if not variable in histname: continue
	# do not consider data
	process = histname.split(variable)[0].rstrip('_')
	if 'data' in process: continue
	# if the proces is not in the list and dict of processes, add it
	if not process in plist:
	    plist.append(process)
	    idnumber = counter
	    if process=='tZq': idnumber = 0
	    else: counter += 1
	    pinfo[process] = {'id':idnumber,'yield':0.,'systematics':{}}
	# determine what systematic the current histogram belongs to
	systematic = histname.split(variable)[-1].strip('_')
	if(systematic[-2:]=='Up'): systematic = systematic[:-2]
	elif(systematic[-4:]=='Down'): systematic = systematic[:-4]
	# in case of nominal: determine the yield and fill that info in pinfo
	if systematic == 'nominal': 
	    pinfo[process]['yield'] += hist.Integral()
	    continue
	if systematic in excludesystematics: continue
	# else fill systematic impacts
	if not systematic in slist: slist.append(systematic)
        if not systematic in pinfo[process]['systematics'].keys():
            pinfo[process]['systematics'][systematic] = 1
    slist = sorted(slist)
    # loop over all processes and fill the systematics that are not present with '-'
    for process in pinfo.keys():
	for systematic in slist:
	    if systematic not in pinfo[process]['systematics'].keys():
		pinfo[process]['systematics'][systematic] = '-'

    # print info if requested
    if not doprint: return (pinfo,slist)
    print('found '+str(len(pinfo))+' processes:')
    for p in pinfo:
	print('  '+p+' (ID: '+str(pinfo[p]['id'])+', yield: '+str(pinfo[p]['yield'])+')')
	for s in pinfo[p]['systematics'].keys():
	    print('    '+s+': '+str(pinfo[p]['systematics'][s]))
    return (pinfo,slist)

def getseparator():
    return '-'*20 + '\n'

def makecolumn(channel,processinfo,process,systematicslist):
    ### get list of strings representing one column in the datacard.
    ### note that all processes in processinfo are assumed to have all systematics in systematicslist
    ### (either a number or '-')
    column = []
    column.append(channel)
    column.append(process)
    column.append(str(processinfo[process]['id']))
    column.append(str(processinfo[process]['yield']))
    for systematic in systematicslist:
	column.append(str(processinfo[process]['systematics'][systematic]))
    return column

def makerow(systematic,stype,processinfo,processimpacts):
    ### make one row to append to a datacard.
    ### inputs are for example 'lumi', 'lnN', readhistfile(...)[0], {'tZq':1.02}
    row = [systematic,stype]
    for p in processinfo.keys():
	if p in processimpacts.keys():
	    row.append(str(processimpacts[p]))
	else:
	    row.append('-')
    return row

def makealigned(stringlist):
    ### append spaces to all strings in stringlist until they have the same length
    # get maximum length of strings in list
    maxlen = 0
    for s in stringlist: 
	if len(s)>maxlen: maxlen=len(s)
    maxlen += 2
    # replace each string by string of fixed length
    for i,s in enumerate(stringlist):
	stringlist[i] = str('{:<'+str(maxlen)+'}').format(s)

if __name__=="__main__":

    if len(sys.argv)<3:
	print('### ERROR ###: need different number of command line args:')
	print('               <input directory> and <datacard directory>')
	sys.exit()
    topdir = sys.argv[1]
    datacarddir = sys.argv[2]
    channelsuffix = ''
    topchargesuffix = ''
    # note: current implementation of channels and top/antitop:
    # - for channels nothing in particular;
    #   only refering to specific histogram files for the signal region;
    #	for the rest the datacard has the same syntax as for the unchanneled fit;
    #   separte channels will not be combined back into a common fit.
    # - for top charge: 
    #   additionally to specific histogram files,
    #	process name is modified since tZq_top and tZq_antitop will be seen as separate signals
    #   (e.g. to compute the ratio of both)
    #   note: modify the names only in the signal regions, since the control regions are not split
    #         this implies another tZq process will be present in the fit... not yet ideal
    #	      maybe remove tZq from control regions? but not everywhere fully negligible...
    #	      probably no need to remove, just consider it as a 'background' in the CR

    for arg in sys.argv[3:]:
	[key,val] = arg.split('=',1)
	if key=='channel': channelsuffix = val
	if key=='topcharge': topchargesuffix = val

    npfromdata = True
    cnames = []
    cnames.append({'region':'signalregion_cat1'+channelsuffix+topchargesuffix,'var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat2'+channelsuffix+topchargesuffix,'var':'_eventBDT'})
    #cnames.append({'region':'signalregion_cat3'+channelsuffix+topchargesuffix,'var':'_eventBDT'})
    cnames.append({'region':'wzcontrolregion','var':'_MT'})
    #cnames.append({'region':'zzcontrolregion','var':'_MT'})
    #cnames.append({'region':'zgcontrolregion','var':'_nJets'})
    #cnames.append({'region':'ttzcontrolregion','var':'_nBJets'})
    #cnames.append({'region':'signalsideband_noossf_cat1','var':'_yield'})
    #cnames.append({'region':'signalsideband_noz_cat1','var':'_yield'})
    years = ['2016']
    # make dict to read channels
    channels = []
    suffix = 'npfromdata' if npfromdata else 'npfromsim'
    for c in cnames:
	region = c['region']
	for year in years:
	    channels.append( {'name':region+'_'+year,
		    'path':os.path.join(topdir,year+'combined',region,suffix,'combined.root'),
		    'var':c['var'] })

    if os.path.exists(datacarddir):
	#print('### WARNING ###: directory already exists. Clean it? (y/n)')
	#go = raw_input()
	#if not go=='y':
	#    sys.exit()
	os.system('rm -r '+datacarddir)
    os.makedirs(datacarddir)

    for channel in channels:
	year = channel['name'][-4:]
	# define variable to use
	variable = channel['var']
	# set lumi uncertainty
	unc_lumi = {}
	if year=='2016': unc_lumi = {'lumi':1.009, # correlated
				     'lumi_2016': 1.022, # uncorrelated
				     'lumi_20162017': 1.008} # partially correlated
	elif year=='2017': unc_lumi = {'lumi':1.008,
				       'lumi_2017':1.02,
				       'lumi_20161017':1.006,
				       'lumi_20172018':1.004}
	elif year=='2018': unc_lumi = {'lumi':1.02,
				       'lumi_2018':1.015,
				       'lumi_20172018':1.003}
	# set trigger uncertainty
	unc_trigger = {'trigger_'+year:1.02} # preliminary dummy value
					     # uncorrelated
	# set individual normalization uncertainties
	unc_norm = {'norm_WZ':1.1,'norm_ZZH':1.1,'norm_tbartZ':1.15,'norm_Xgamma':1.1,'norm_nonprompt':1.3}
	# check if histogram file exists
	if not os.path.exists(channel['path']):
	    print('### WARNING ###: '+channel['path']+' not found, skipping it.')
	    raise Exception('see warning above, break off for safety of not missing something')
	print('making datacard for '+channel['path']+'...')
	# copy root file to location
	os.system('cp '+channel['path']+' '+datacarddir+'/histograms_'+channel['name']+'.root')
	# get necessary info
	(processinfo,systematiclist) = readhistfile(channel['path'],variable,doprint=True)
	# (processinfo is a dict mapping process names to a dict of info)
	# redefine signal
	# in case of top, modify tZq to tZq_top 
	if( topchargesuffix!='' and 'signalregion' in channel['name'] ): 
	    processinfo['tZq'+topchargesuffix] = processinfo['tZq']
	    processinfo.pop('tZq')
	    # also change systematics
	    # DOES NOT WORK YET AS HISTOGRAM NAMES DO NOT CONTAIN TOPCHARGESUFFIX
	    # also probably unwanted behaviour, tZq systematics should be correlated between
	    # tZq_top and tZq_antitop (?)
	    #systematics_copy = list(systematiclist)
	    #for sys in systematics_copy:
	    #	if 'tZq' in sys:
	    #	    newsys = sys.replace('tZq','tZq'+topchargesuffix)
	    #	    systematiclist.append(newsys)
	    #	    systematiclist.remove(sys)
	    #	    for p in processinfo.keys():
	    #		processinfo[p]['systematics'][newsys] = processinfo[p]['systematics'].pop(sys)
	    #systematiclist = sorted(systematiclist)
	# open (recreate) datacard file
	datacard = open(os.path.join(datacarddir,'datacard_'+channel['name']+'.txt'),'w')
	# write nchannels, nprocesses and nparameters
	datacard.write('imax\t1'+'\n') # write 1 for now, combine channels later
	datacard.write('jmax\t'+str(len(processinfo)-1)+'\n')
	datacard.write('kmax\t'+'*\n')
	datacard.write(getseparator())
	# write file info (WARNING: depends on convention how to rename the copied root files, see above)
	datacard.write('shapes * '+channel['name']+' histograms_'+channel['name']+'.root $PROCESS_'+variable+'_nominal $PROCESS_'+variable+'_$SYSTEMATIC\n')
	if topchargesuffix != '': 
	    datacard.write('shapes tZq'+topchargesuffix+' '+channel['name']+' histograms_'+channel['name']+'.root tZq_'+variable+'_nominal tZq_'+variable+'_$SYSTEMATIC\n')
	datacard.write('shapes data_obs '+channel['name']+' histograms_'+channel['name']+'.root data_'+variable+'_nominal\n')
	datacard.write(getseparator())
	# write bin info
	datacard.write('bin\t\t'+channel['name']+'\n')
	datacard.write('observation\t-1\n')
	datacard.write(getseparator())
	# make first and second column
	c1 = ['bin','process','process','rate']
	c2 = ['','','','']
	for systematic in systematiclist:
	    c1.append(systematic)
	    c2.append('shape')
	# make rest of the columns
	columns = [c1,c2]
	for process in processinfo:
	    pcolumn = makecolumn(channel['name'],processinfo,process,systematiclist)
	    columns.append(pcolumn)
	# make rows
	rows = []
	# lumi uncertainty:
	for lumisource in unc_lumi.keys():
	    pdict = {}
	    for p in processinfo.keys(): 
		if p =='nonprompt': continue # estimated from data so no lumi unc.
		pdict[p] = unc_lumi[lumisource]
	    rows.append(makerow(lumisource,'lnN',processinfo,pdict))
	# trigger uncertainty:
	for triggersource in unc_trigger.keys():
	    pdict = {}
            for p in processinfo.keys(): 
		if p=='nonprompt': continue # estimated from data so no trigger unc.
		pdict[p] = unc_lumi[lumisource]
	    rows.append(makerow(triggersource,'lnN',processinfo,pdict))
	# norm uncertainties
	for normsource in unc_norm.keys():
	    rows.append(makerow(normsource,'lnN',processinfo,{normsource.split('_')[-1]:unc_norm[normsource]}))
	# add rows to columns
	for row in rows:
	    for rowel,c in zip(row,columns):
		c.append(rowel)
	# align columns
	for c in columns: makealigned(c)
	# write all info row by row
	nrows = len(columns[0])
	for row in range(nrows):
	    for col in range(len(columns)):
		datacard.write(columns[col][row]+' ')
	    datacard.write('\n')
	    if(row==3): datacard.write(getseparator())
	datacard.write(getseparator())
	# manage statistical uncertainties
	threshold = 10
	datacard.write(channel['name']+' autoMCStats '+str(threshold))
    
	# close datacard
	datacard.close()
