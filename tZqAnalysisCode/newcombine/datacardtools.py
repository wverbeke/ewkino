############################################################
# tools for automatically generating datacards for combine #
############################################################

import os
import sys
import re
import ROOT

### classes to store all info for processes relevant for making datacards ###

class ProcessInfo:
    ### dict-like data structure storing information for a single process
    
    def __init__( self, name, pid, pyield=-1, hname='', systematics={} ):
	### initializer
	# input arguments:
	# - name: string representing the name of this process
	# - pid: integer representing the id number for combine
	# - pyield: float representing the yield for this process
	# - hname: alternative name for the process, 
	#	    referring to how it is named in a histogram file
	#	    (default: equal to name)
	# - systematics: dict mapping systematics names to impacts (float or '-')
	# todo: implement proper checking of arguments
	self.name = name
	self.pid = pid
	self.pyield = pyield
	self.hname = hname
	if hname=='': self.hname = name
	self.systematics = systematics

    def addsys( self, sysname, mag ):
	### add a systematic to this process with given impact strength (float or '-')
	# (note: to update already present systematic, use enablesys)
	if sysname in self.systematics.keys():
	    raise Exception('ERROR in ProcessInfo.addsys: '
		    +'systematic {} already exists for process {}'.format(sysname,self.name))
	self.systematics[sysname] = mag

    def enablesys( self, sysname, mag ):
	### enable a systematic for this process with given impact strength (float or '-')
	# (note: to add a new systematic, use addsys)
        if sysname not in self.systematics.keys():
            raise Exception('ERROR in ProcessInfo.enablesys: '
		    +'systematic {} not found for process {}'.format(sysname,self.name))
        self.systematics[sysname] = mag

    def disablesys( self , sysname ):
	### disable a given systematic for a given set of processes
        self.enablesys( sysname, '-' )

    def changename( self, newname ):
	### change name of this ProcessInfo
	# note: names of systematics that may contain the process name,
	#       as well as hname, are unaffected!
	self.name = newname

    def __str__( self ):
	### get printable string for this ProcessInfo
	res = 'ProcessInfo:\n'
	res += '  process: {}, pid: {}, yield: {}\n'.format(self.name,self.pid,self.pyield)
        res += '  systematics\n'
        for s.v in self.systematics.items(): res += '    {}: {}\n'.format(s,v)
	return res

class ProcessInfoCollection:
    ### collection of ProcessInfo's with addtional info common to all of them

    def __init__( self ):
	### empty initializer
	self.pinfos = {}
	self.plist = []
	self.slist = []
	self.minpid = None
	self.maxpid = None

    def nprocesses( self ):
	### get current number of processes
	return len(self.plist)

    def addprocess( self, processinfo ):
	### add a process
	# note that the slist and already present processes will be updated
	# so each ProcessInfo in the collection contains the same systematics
	# input arguments:
	# - processinfo: object of type ProcessInfo
	if not isinstance(processinfo,ProcessInfo):
	    raise Exception('ERROR in ProcessInfoCollection.addprocess: type {} for argument'
			    +' is invalid'.format(type(processinfo)))
	if processinfo.name in self.plist:
	    raise Exception('ERROR in ProcessInfoCollection.addprocess: a process with name {}'
			    +' already exists within this collection'.format(processinfo.name))
	for p in self.plist:
	    if self.pinfos[p].pid == processinfo.pid:
		raise Exception('ERROR in ProcessInfoCollection.addprocess: a process with pid {}'
				+' already exists within this collection'.format(processinfo.pid))
	# add the systematics of this new process to all existing processes
	for sys in processinfo.systematics.keys():
	    if not sys in self.slist:
		self.slist.append(sys)
		for p in self.pinfos.values(): p.addsys( sys, '-' )
	# add the existing systematics to the new process
	for sys in self.slist:
	    if not sys in processinfo.systematics.keys():
		processinfo.addsys( sys, '-' )
	# add the new process
	self.pinfos[processinfo.name] = processinfo
	self.plist.append(processinfo.name)
	if( self.maxpid is None or processinfo.pid > self.maxpid ): self.maxpid = processinfo.pid
	if( self.minpid is None or processinfo.pid < self.minpid ): self.minpid = processinfo.pid
	# re-sort lists
	self.sort()

    def sort( self ):
	### (re-) sort both the systematic list (alphabetically) and process list (by ID)
	self.slist = sorted(self.slist)
        newplist = []
        for i in range(self.minpid,self.maxpid+1):
            for p in self.plist:
                if self.pinfos[p].pid==i: newplist.append(p)
        self.plist = newplist

    def addnormsys( self, sysname, impacts ):
	### add a normalization uncertainty
	# input arguments:
	# - sysname: name of systematic uncertainty
	# - impacts: dict mapping process names to impacts (either float or equivalent string, or '-')
	#   - note: the keys of impacts must correspond exactly to this ProcessInfoCollection, 
	#     for safety of accidentally using wrong process names and hence lacking uncertainties
	if sorted(self.plist)!=sorted(impacts.keys()):
	    raise Exception('ERROR in ProcessInfoCollection.addnormsys: processes in info struct '
			    +'and impacts do not agree; '
			    + 'found\n{}\nand\n{}'.format(sorted(self.plist),sorted(impacts.keys())))
	if sysname in self.slist:
	    print('WARNING in ProcessInfoCollection.addnormsys: overwriting existing uncertainty...')
	self.slist.append( sysname )
	for p in self.plist:
	    self.pinfos[p].addsys( sysname, impacts[p] )

    def enablesys( self, sysname, processes, mag):
	### enable a given systematic for a given set of processes with given magnitude
	if sysname not in self.slist:
                raise Exception('ERROR in ProcessInfoCollection.enablesys: '
				+'systematic {} not found'.format(sysname))
	for p in processes:
	    if p not in self.plist:
		raise Exception('ERROR in ProcessInfoCollection.enablesys: '
				+'process {} not found'.format(p))
	    self.pinfos[p].enablesys( sysname, mag )

    def disablesys( self, sysname, processes ):
	### disable a given systematic for a given set of processes
	self.enablesys( sysname, processes, '-' )

    def changename( self, oldpname, newpname ):
	### change the name of a process from oldpname to newpname
	# note: all systematic uncertainties associated to this process,
	# as well as the 'hname' (to find it in the histogram file) are unaffexted!
	if not oldpname in self.plist:
	    raise Exception('ERROR in ProcessInfoCollection.changename: '
			    +'old name {} not in process info'.format(oldpname))
	self.pinfos[newpname] = self.pinfos.pop(oldpname)
	self.plist.append( newpname )
	self.plist.remove( oldpname )
	self.sort()

    def makebkg( self, pname ):
	### turn a given process from signal (id<=0) to background (id>0)
	if not pname in self.plist:
	    raise Exception('ERROR in ProcessInfoCollection.makebkg: '
			    +'process {} not in process info'.format(pname))
	if self.pinfos[pname].pid>0:
	    print('WARNING in ProcessInfoCollection.makebkg: '
		    +'process {} is already a background process'.format(pname))
	    return
	# redefine id's of unmodified processes
	for p in self.plist:
	    # add 1 to all backgrounds
	    if self.pinfos[p].pid>0:
		self.pinfos[p].pid += 1
	    # add 1 to all signals with pid < original pid of given process
	    if self.pinfos[p].pid<self.pinfos[pname].pid:
		self.pinfos[p].pid += 1
	# redefine given process
	self.pinfos[pname].pid = 1
	self.maxpid += 1
	self.minpid += 1
	self.sort()

    def __str__( self ):
	### get a printable string of this collection
	res = 'ProcessInfoCollection with {} processes and {} systematics\n'.format(
		len(self.plist),len(self.slist))
	for n,p in self.pinfos.items():
	    res += '  process: {}, pid: {}, yield: {}\n'.format(n,p.pid,p.pyield)
	    for s in self.slist:
		res += '    {}: {}\n'.format(s,p.systematics[s])
	return res

### constructor of ProcessInfoCollection from a file containing histograms ###

def readhistfile(histfile,variable,signals,
		includesystematics=[],excludesystematics=[],datatag='data' ):
    ### read a ROOT file containing histograms and make a ProcessInfoCollection
    # note: this concerns a definition of processes and shape systematics;
    # to add normalization uncertainties (not stored as root histograms), 
    # use ProcessInfoCollection.addnormsys
    # input arguments:
    # - histfile is the path to a root file containing all histograms
    #   the histograms are assumed to be named process_variable_systematic
    #   (with 'nominal' as systematic for the nominal histogram)
    #	(all tags, i.e. process, variable and systematic, are allowed to contain underscores)
    # - variable is the name of the variable for which to extract the histograms
    # - signals is a list of process names that identify signal processes (opposed to background)
    #   (signals are given an 'index' <= 0 to make combine define them as signal)
    # - includesystematics: list of systematics to include (default: all in file)
    # - excludesystematics: list of systematics to exclude (default: none)
    # - datatag is the process name of data histograms; they are fully ignored by this function
    # output object: a ProcessInfoCollection object 

    # initialization
    f = ROOT.TFile.Open(histfile)
    pinfo = {} # final output dict containing info for all processes
    plist = [] # list of process names
    slist = [] # list of systematics
    bkgcounter = 1 # id counter for backgrounds
    sigcounter = 0 # id counter for signals
    keylist = f.GetListOfKeys()
    # loop over all histograms in the file and construct the individual processes
    for key in keylist:
        hist = f.Get(key.GetName())
	histname = hist.GetName()
	# select only histograms of the requested variable
	if not variable in histname: continue
	# do not consider data
	process = histname.split(variable)[0].rstrip('_')
	if datatag in process: continue
	# if the proces is not in the list and dict of processes, add it
	if not process in plist:
	    plist.append(process)
	    idnumber = bkgcounter
	    if process in signals: 
		idnumber = sigcounter
		sigcounter -= 1
	    else: bkgcounter += 1
	    pinfo[process] = ProcessInfo( process, idnumber, 0., hname=process, systematics={} )
	# determine what systematic the current histogram belongs to
	systematic = histname.split(variable)[-1].strip('_')
	if(systematic[-2:]=='Up'): systematic = systematic[:-2]
	elif(systematic[-4:]=='Down'): continue
	# (consider only up as only the name of the systematic is needed
	#  possible extension later: check explicitly if both up and down are present)
	# in case of nominal: determine the yield and fill that info in pinfo
	if systematic == 'nominal': 
	    pinfo[process].pyield += hist.Integral()
	    continue
	# check whether to consider this systematic
	if( len(includesystematics)>0 and (systematic not in includesystematics) ): continue
	if( len(excludesystematics)>0 and (systematic in excludesystematics) ): continue
	# if so, set systematic impacts
	if not systematic in slist: slist.append(systematic)
        if not systematic in pinfo[process].systematics:
            pinfo[process].addsys(systematic,1)
    # add all processes to a collection
    if len(plist)==0:
	print('WARNING in datacardtools.py/readhistfile: returning an empty ProcessInfoCollection;'
		+' check if the file contains the right histograms and if they are read correctly.')
    if len(slist)==0:
	print('WARNING in datacardtools.py/readhistfile: returning a ProcessInfoCollection'
		+' with no systematics;'
                +' check if the file contains the right histograms and if they are read correctly.')
    PIC = ProcessInfoCollection()
    for p in pinfo.values():
	PIC.addprocess(p)
    return PIC

### functions for writing the datacard info to a txt file in correct format ###

def makecolumn(channel,process,pid,pyield,systematicsdict,systematicslist):
    ### get list of strings representing one column in the datacard.
    # input arguments:
    # - channel: name of the channel (= first row) (called 'bin' in combine)
    # - process: name of the process (= second row)
    # - pid and pyield: id number and expected yield of this process
    # - systematicsdict: dict mapping systematics for this process to either '1' or '-'
    # - systematicslist: list of systematics
    #   - note: all columns (also the first one listing the names of the systematics)
    #     must be made using the same systematicslist, also same order,
    #     else the marks ('1' or '-') will not be in the correct row
    #   - note: all names in systematicslist are expected to be in systematicsdict,
    #     either with '1' (str or float) or '-' as value
    #   - this is for shape systematics only, for flat ones, use makerow instead of makecolumn
    # output:
    # - list of strings (in correct order) corresponding to the rows of the column
    column = []
    column.append(str(channel))
    column.append(str(process))
    column.append(str(pid))
    column.append(str(pyield))
    for systematic in systematicslist:
	column.append(str(systematicsdict[systematic]))
    return column

def makerateparamrow( process ):
    ### return a row for a datacard adding a rateParam for the given process
    # note: maybe extend functionality later, for now just adding a line
    #       rate_<process> rateParam * <process> 1.0 [0.0,3.0]
    # warning: it should be checked in the calling function if process is a valid name!
    rpstr = 'rate_{} rateParam * {} 1.0 [0.0,3.0]'.format(process,process)
    return rpstr

def getseparator(endline=True):
    ### return a string usable as horizontal separator in the card
    # optionally include a newline char at the end
    sep = '-'*20
    if endline: sep += '\n'
    return sep

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

def writedatacard(datacarddir,channelname,processinfo,
		  histfile,variable,datatag='data',
		  shapesyslist=[],lnNsyslist=[],
		  rateparamlist=[],ratio=[],
		  automcstats=10,
		  writeobs=True,
		  autof=False):
    ### write a datacard corresponding to a single channel ('bin' in combine terminology)
    # input arguments:
    # - datacarddir: directory where the card and histograms will go
    # - channelname: name of the channel/bin
    # - processinfo: object of type ProcessInfoCollection
    # - histfile: path to root file containing the histograms (naming convention: see readhistfile)
    #   note: the file will be copied to the datacard directory
    # - variable: name of the variable for which the histograms will be used
    # - datatag: process name of observed data (how it is named in the root histograms)
    # - shapesyslist: list of shape systematics to consider (default: none)
    #	(must be a subset of the ones included in processinfo)
    # - lnNsyslist: list of normalization systematics (type lnN) to consider (default: none)
    #   (must be a subset of the ones included in processinfo)
    # - rateparamlist: a list of process names for which to add a rate param
    #   note: maybe extend functionality later, for now just adding a line
    #         rate_<process> rateParam * <process> 1.0 [0.0,3.0]
    # - ratio: list of 2 process names, a rate param will be added linking both
    #   (so the signal strength for the first of both corresponds to the ratio)
    #   note: the first element in ratio must be the (unique) signal process
    #   note: can also contain only 1 element with a wildcard in it,
    #         in this case the rateParam will be added if there is a matching process, else ignored
    # - automcstats: threshold for combine's autoMCStats functionality
    # - writeobs: whether to write for each process the observed yield
    #   (if False, simply write -1 to extract the yield at runtime,
    #    can be useful but not very clean when recycling datacards for different variables
    #	 with slightly different yields (depending on the binning) 
    #    without remaking the processinfo)
    # - autof: boolean whether to overwrite existing card without explicitly asking

    # make path to datacard
    datacardname = 'datacard_'+channelname+'.txt'
    datacardpath = os.path.join(datacarddir,datacardname) 
    # check if datacard file already exists
    if( os.path.exists(datacardpath) and not autof ):
	print('WARNING in writedatacard: requested file already exists. Overwrite? (y/n)')
	go = raw_input()
	if not go=='y': return
    # copy root file to location
    newhistname = 'histograms_'+channelname+'.root'
    newhistpath = os.path.join(datacarddir,newhistname)
    if not os.path.exists(histfile):
	raise Exception('ERROR in writedatacard: input histogram file {} '
			+'does not seem to exist'.format(histfile))
    os.system('cp '+histfile+' '+newhistpath)
    # open (recreate) datacard file
    datacard = open(datacardpath,'w')
    # write nchannels, nprocesses and nparameters
    datacard.write('imax\t1'+'\n') # only 1 channel, combine later
    datacard.write('jmax\t'+str(processinfo.nprocesses()-1)+'\n')
    datacard.write('kmax\t'+'*\n')
    datacard.write(getseparator())
    # write file info
    for p in processinfo.plist:
	datacard.write('shapes '+p+' '+channelname+' '+newhistname
			+' '+processinfo.pinfos[p].hname+'_'+variable+'_nominal'
			+' '+processinfo.pinfos[p].hname+'_'+variable+'_$SYSTEMATIC\n')
    datacard.write('shapes data_obs '+channelname+' '+newhistname
		    +' '+datatag+'_'+variable+'_nominal\n')
    datacard.write(getseparator())
    # write bin info
    datacard.write('bin\t\t'+channelname+'\n')
    datacard.write('observation\t-1\n')
    datacard.write(getseparator())
    # make first and second column
    c1 = ['bin','process','process','rate']
    c2 = ['','','','']
    for systematic in shapesyslist:
        c1.append(systematic)
        c2.append('shape')
    for systematic in lnNsyslist:
	c1.append(systematic)
	c2.append('lnN')
    # make rest of the columns
    columns = [c1,c2]
    for p in processinfo.plist:
	pyield = processinfo.pinfos[p].pyield
	if not writeobs: pyield = -1
        pcolumn = makecolumn(channelname,p,processinfo.pinfos[p].pid,
		    pyield,processinfo.pinfos[p].systematics,
		    shapesyslist+lnNsyslist)
        columns.append(pcolumn)
    # format the columns
    for c in columns: 
	makealigned(c)
    # write all info row by row
    nrows = len(columns[0])
    for row in range(nrows):
        for col in range(len(columns)):
            datacard.write(columns[col][row]+' ')
        datacard.write('\n')
        if(row==3): datacard.write(getseparator())
	if(row==3+len(shapesyslist)): datacard.write(getseparator())
    datacard.write(getseparator())
    # add rate parameters
    if len(rateparamlist)>0:
	for p in rateparamlist:
	    if p not in processinfo.plist:
		raise Exception('ERROR in writedatacard: rateParam requested for '
				+'{}, but not in list of processes'.format(p))
	    datacard.write( makerateparamrow(p)+'\n' )
	datacard.write(getseparator())
    # add rate parameters for ratio measurement
    if len(ratio)>0:
	if len(ratio)==1:
	    if '*' not in ratio[0]:
		raise Exception('ERROR in writedatacard: list of ratio measurement has length 1,'
				' which is only supported if the process name contains a wildcard.')
	    hasmatch = False
	    for p in processinfo.plist:
		if re.match(ratio[0].replace('*','.*'),p): hasmatch = True
	    if hasmatch:
		datacard.write( 'ratio_scale rateParam * '+ratio[0]+' 1.0\n' )
		datacard.write(getseparator())
	elif len(ratio)==2:
	    if not ratio[0] in processinfo.plist:
		raise Exception('ERROR in writedatacard: process '+ratio[0]+' in list for ratio'
			    +' but not in list of recognized processes')
	    if not ratio[1] in processinfo.plist:
		raise Exception('ERROR in writedatacard: process '+ratio[1]+' in list for ratio'
                            +' but not in list of recognized processes')
	    if not processinfo.pinfos[ratio[0]].pid <= 0:
		raise Exception('ERROR in writedatacard: process '+ratio[0]+' is numerator for ratio'
			    +' but not defined as signal')
	    datacard.write( 'ratio_scale rateParam * '+ratio[0]+' 1.0\n' )
	    datacard.write( 'ratio_scale rateParam * '+ratio[1]+' 1.0\n' )
	    datacard.write(getseparator())
	else:
	    raise Exception('ERROR in writedatacard: list for ratio measurement has unexpected '
                            +' length: {}'.format(len(ratio)))
	    
    # manage statistical uncertainties
    datacard.write(channelname+' autoMCStats '+str(automcstats))
    # close datacard
    datacard.close()
