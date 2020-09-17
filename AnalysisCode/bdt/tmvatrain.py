############################################################
# A script to train a tZq signal BDT with the TMVA package #
############################################################
import ROOT
import sys
import os
import json
#import tmvautils as u

def pathtosidebandfile(filename,sidebanddir):
    ### return path to file in sideband directory corresponding to filename, '' if none
    filename = filename.split('/')[-1].replace('.root','')
    sfiles = [f for f in os.listdir(sidebanddir) if f[-5:]=='.root']
    for f in sfiles:
	if filename in f: return os.path.join(sidebanddir,f)
    return ''

def getscalefactor(filelist,treenamelist,scaledentries):
    ### return factor to scale entries in trees so that total nentries corressponds to scaledentries
    nentries = 0
    for fpath,treename in zip(filelist,treenamelist):
	f = ROOT.TFile.Open(fpath)
	tree = f.Get(treename)
	nentries += tree.GetEntries()
	f.Close()
    return float(scaledentries)/nentries

def getmvatreesfromdirs(indirs,treenames,sigtag,sidebanddirs=[]):
    ### Add signal and background to DataLoader
    # sidebanddirs must have same lenth as indirs and  determines which sideband folder to include
    # the sideband folder must contain files with the same name as regular inputs (extensions allowed)
    # use '' to not include a sideband for a specific indir
    sigchains = [] # remark: one chain per tree will be used, in order to apply per-file weights;
    bckchains = [] # not sure if a list of trees (without chain) would work as well
    bckchains_onlytrain = []
    bckchains_onlytest = []
    nsig = 0
    nbck = 0
    for indir in indirs:
	filelist = [f for f in os.listdir(indir) if f[-5:]=='.root']
	for filename in filelist:
	    # use only training files:
	    #if not '_train' in filename: continue
	    # check whether file has sideband equivalent
	    sfile = ''
	    if(len(sidebanddirs)==len(indirs) and sidebanddirs[indirs.index(indir)]!=''):
		sfile = pathtosidebandfile(filename,sidebanddirs[indirs.index(indir)])
	    fullpath = os.path.join(indir,filename)
	    # call loader on this root file
	    temp = getmvatrees(fullpath,treenames,sigtag,sidebandrootfilename=sfile)
	    sigchains += temp[0]
	    bckchains += temp[1]
	    bckchains_onlytrain += temp[2]
	    bckchains_onlytest += temp[3]
	    nsig += temp[4]
	    nbck += temp[5]
    print('Total amount of signal entries: '+str(nsig))
    print('Total amound of background entries: '+str(nbck))
    return (sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,nsig,nbck)

def getmvatrees(rootfilename,treenames,sigtag,sidebandrootfilename=''):

    # intitalizations of return objects    
    sigchains = []
    bckchains = []
    bckchains_onlytrain = []
    bckchains_onlytest = []
    nsig = 0
    nbck = 0

    # open root file
    filename = rootfilename.split('/')[-1]
    temp = ROOT.TFile.Open(rootfilename)
    for treename in treenames:
	# for trees containing signal events, add to list of signal chains
	if sigtag in filename:
	    try:
		sigchains.append({'chain':ROOT.TChain(treename),'weight':1.0})
		sigchains[-1]['chain'].Add(rootfilename+'/'+treename)
		nsig += temp.Get(treename).GetEntries()
		print('Found signal file: '+filename+' ('+treename+')')
	    except:
		print('An error occurred for '+filename+' ('+treename+')')
	# for trees containing background events, add to list of background chains
	else:
	    # skip DY:
	    #if 'DYJets' in filename: continue
	    #try:
		nbckf = temp.Get(treename).GetEntries()
                if nbckf==0: continue
		if len(sidebandrootfilename)==0:
		    # add regular file with original weight
                    bckchains.append({'chain':ROOT.TChain(treename),'weight':1.0})
                    bckchains[-1]['chain'].Add(rootfilename+'/'+treename)
		    print('Found background file: '+filename+' ('+treename+')')
		else:
		    # method 1:
		    # add regular and sideband file with proper weight
		    weight = getscalefactor([rootfilename,sidebandrootfilename],
					    [treename,treename],nbckf)
                    bckchains.append({'chain':ROOT.TChain(treename),'weight':weight})
                    bckchains[-1]['chain'].Add(rootfilename+'/'+treename)
                    print('Found background file: '+filename+' ('+treename+') with weight '
                            +str(weight))
		    if weight>1.: # special catch for no events in sideband file
			bckchains.append({'chain':ROOT.TChain(treename),'weight':weight})
			bckchains[-1]['chain'].Add(sidebandrootfilename+'/'+treename)
			print('Found sideband background file: '+sidebandrootfilename.split('/')[-1]
				+' ('+treename+') with weight '+str(weight))
		    # method 2:
		    '''# add regular and sideband file to training with proper weight
		    weight = getscalefactor([sfile],[treename],nbckf)
		    #bckchains_onlytrain.append({'chain':ROOT.TChain(treename),'weight':weight})
		    #bckchains_onlytrain[-1]['chain'].Add(rootfilename+'/'+treename)
		    #print('Found background file: '+filename+' ('+treename+') with weight '
		    	    +str(weight))
		    bckchains_onlytrain.append({'chain':ROOT.TChain(treename),'weight':weight})
		    bckchains_onlytrain[-1]['chain'].Add(sfile+'/'+treename)
		    print('Found sideband background file: '+sfile.split('/')[-1]
			    +' ('+treename+') with weight '+str(weight))
		    # add regular file to testing with original weight
		    bckchains_onlytest.append({'chain':ROOT.TChain(treename),'weight':1.0})
		    bckchains_onlytest[-1]['chain'].Add(rootfilename+'/'+treename)'''
		nbck += nbckf
	    #except:
		#print('An error occurred for '+filename+' ('+treename+')')
    # return all chains and number of signal and background events from this file
    return (sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,nsig,nbck)

def tloader_fromchains(sigchains, bckchains, bckchains_onlytrain, bckchains_onlytest,
			variables,tloader_options, outfilename):
    # intialize a TMVA.Dataloader using the output from getmvatreesfrom dirs directly
    tloader = ROOT.TMVA.DataLoader(outfilename.replace('.root','_data'))
    for sigchain in sigchains: tloader.AddSignalTree(sigchain['chain'],sigchain['weight'])
    for bckchain in bckchains: tloader.AddBackgroundTree(bckchain['chain'],bckchain['weight'])
    for bckchain in bckchains_onlytrain: tloader.AddBackgroundTree(bckchain['chain'],bckchain['weight'],
                                                                    ROOT.TMVA.Types.kTraining)
    for bckchain in bckchains_onlytest: tloader.AddBackgroundTree(bckchain['chain'],bckchain['weight'],
                                                                    ROOT.TMVA.Types.kTesting)
    tloader.SetSignalWeightExpression("_normweight")
    tloader.SetBackgroundWeightExpression("_normweight")

    ### Define train and test set
    for var in variables:
        tloader.AddVariable(var,'F')
    cuts = ROOT.TCut("_normweight>0")
    cutb = ROOT.TCut("_normweight>0")
    #cuts = ROOT.TCut("")
    #cutb = ROOT.TCut("")
    tloader.PrepareTrainingAndTestTree(cuts,cutb,tloader_options)
    return tloader

def tloader_fromfile(inputfile,sigtreename,bcktreename,variables,tloader_options,outfilename):
    # initialize a TMVA.DataLoader using the output from makeinputfile.py,
    # to compare more directly with the BDT results from Willem's deepLearning repository
    tloader = ROOT.TMVA.DataLoader(outfilename.replace('.root','_data'))
    infile = ROOT.TFile.Open(inputfile)
    sigtree = infile.Get(sigtreename)
    sigtree.SetDirectory(0)
    nsig = sigtree.GetEntries()
    bcktree = infile.Get(bcktreename)
    bcktree.SetDirectory(0)
    nbck = bcktree.GetEntries()
    infile.Close()
    print('input file contains {} signal and {} background entries'.format(nsig,nbck))
    tloader.AddSignalTree(sigtree)
    tloader.AddBackgroundTree(bcktree)
    tloader.SetSignalWeightExpression("_normweight")
    tloader.SetBackgroundWeightExpression("_normweight")

    ### Define train and test set
    for var in variables:
        tloader.AddVariable(var,'F')
    cuts = ROOT.TCut("_normweight>0")
    cutb = ROOT.TCut("_normweight>0")
    #cuts = ROOT.TCut("")
    #cutb = ROOT.TCut("")
    tloader.PrepareTrainingAndTestTree(cuts,cutb,tloader_options)
    return tloader

def tmvatrain(tloader,tfactory_options,outfilename):
    # initialize a TMVA.Factory and train the BDT
    
    outfile = ROOT.TFile(outfilename,"recreate")
    tfactory = ROOT.TMVA.Factory('tmvatrain',outfile,"")
    # book BDT method
    tfactory.BookMethod(tloader,ROOT.TMVA.Types.kBDT,"BDT",tfactory_options)
    # train and test method
    tfactory.TrainAllMethods()
    tfactory.TestAllMethods()
    tfactory.EvaluateAllMethods()
    roc = outfile.Get(outfilename.replace('.root','_data')+"/Method_BDT/BDT/MVA_BDT_rejBvsS")
    aucroc = roc.Integral(1,roc.GetNbinsX(),"width")
    outfile.Close()
    return aucroc

def argsfromcmd(args):
    ### Check input args from command line
    argkeys = []
    rqargkeys = (['combine','indirs','sidebanddirs','treenames','sigtag',
		    'varlist','lopts','fopts'])
    for arg in args:
	(key,value) = tuple(arg.split('=',1))
	if key=='combine': combine=value; argkeys.append(key)
	elif key=='indirs': 
	    indirs=[os.path.abspath(str(x)) for x in json.loads(value)]
	    argkeys.append(key)
	    years = []
	    for indir in indirs:
		years.append(indir[indir.find('201'):indir.find('201')+4])
	elif key=='sidebanddirs':
	    sidebanddirs=[os.path.abspath(str(x)) for x in json.loads(value)]
	    argkeys.append(key)
	elif key=='treenames': 
	    treenames=[str(x) for x in json.loads(value)]; 
	    argkeys.append(key)
	elif key=='sigtag': sigtag=value; argkeys.append(key)
	elif key=='varlist': 
	    varlist=[str(x) for x in json.loads(value)]; 
	    argkeys.append(key)
	elif key=='lopts': lopts=value; argkeys.append(key)
	elif key=='fopts': fopts=value; argkeys.append(key)
	else:
	    print('### WARNING ###: argument not recognized: '+str(arg))
	    print('                 skipping it...')
    missargkeys = []
    for argkey in rqargkeys:
	if not argkey in argkeys:
	    missargkeys.append(argkey)
    if len(missargkeys)>0:
	print('### ERROR ###: following required arguments are missing:')
	print(missargkeys)
	sys.exit()
    return (combine,years,indirs,sidebanddirs,treenames,sigtag,varlist,lopts,fopts)

if __name__=="__main__":
    
    if len(sys.argv)>1:
	args = sys.argv[1:]
	(combine,years,indirs,sidebanddirs,treenames,sigtag,varlist,lopts,fopts) = argsfromcmd(args)
    else:
	### Set global switches
	combine = 'years' # choose from 'none', 'years', 'regions', 'all'
	### Set list of input files
	years = []
	years.append('2016')
	years.append('2017')
	years.append('2018')
	indirs = []
	for year in years: 
	    indirs.append('/user/llambrec/Files/tzqid/'+year+'MC/signalregion_3tight_flat')
	sidebanddirs = []
	#sidebanddirs = [f.replace('_flat','_2tight_flat') for f in indirs]
	treenames = ["blackJackAndHookers/treeCat1"]
	treenames.append("blackJackAndHookers/treeCat2")
	treenames.append("blackJackAndHookers/treeCat3")
	sigtag = 'tZq'
	### Data loading options
	lopts = "SplitMode=Random:NormMode=None"
	### BDT options
	fopts = "!H:!V" # help and verbosity level
	fopts += ":NTrees=1000:BoostType=Grad" # options for ensemble
	fopts += ":MinNodeSize=5%:MaxDepth=4:nCuts=200" # options for single tree
	fopts += ":UseBaggedGrad=True:BaggedSampleFraction=0.5"
	fopts += ":Shrinkage=0.05"
	#fopts += ":IgnoreNegWeightsInTraining=True"
	### variables
	varlist = (['_abs_eta_recoil','_Mjj_max','_lW_asymmetry',
		'_deepCSV_max','_lT','_MT','_dPhill_max',
		'_pTjj_max','_dRlb_min','_HT','_dRlWrecoil','_dRlWbtagged',
		'_M3l','_abs_eta_max'])
	varlist += (['_nJets','_nBJets']) # parametrized learning
    ### Function call
    if(combine=='all'):
	outfilename = 'out_all.root'
	# train a BDT on all years and regions together
	# option 1: use output from getmvatreesfromdirs directly to add to the TMVA.DataLoader
	(sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,_,_) = getmvatreesfromdirs(
	   indirs,treenames,sigtag,sidebanddirs=sidebanddirs)
	tloader = tloader_fromchains(sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,
					varlist,lopts,outfilename)
	# option 2: use input file created indirectly using makeinputfile.py
	#tloader = tloader_fromfile('tZqTrainInput.root','sigtree','bcktree',varlist,lopts)
	# now train the BDT
	aucroc = tmvatrain(tloader,fopts,outfilename)
	print('results for '+outfilename)
	print('---AUC (ROC)---')
	print(str(aucroc))
    elif(combine=='none'):
	if '_nJets' in varlist: varlist.remove('_nJets')
	if '_nBJets' in varlist: varlist.remove('_nBJets')
	for indir,year in zip(indirs,years):
	    for treename in treenames:
		outfilename = 'out_'+year+'_'+treename.split('/')[-1]+'.root'
		(sigcs,bckcs,bckcs_train,bckcs_test,_,_) = getmvatreesfromdirs([indir],[treename],
							    sigtag,sidebanddirs=sidebanddirs)
		tloader = tloader_fromchains(sigcs,bckcs,bckcs_train,bckcs_test,
					    varlist,lopts,outfilename)
		aucroc = tmvatrain(tloader,fopts,outfilename)
		print('results for '+outfilename)
		print('---AUC (ROC)---')
		print(str(aucroc))
    elif(combine=='years'):
	if '_nJets' in varlist: varlist.remove('_nJets')
        if '_nBJets' in varlist: varlist.remove('_nBJets')
        for treename in treenames:
	    outfilename = 'out_'+treename.split('/')[-1]+'.root'
	    (sigcs,bckcs,bckcs_train,bckcs_test,_,_) = getmvatreesfromdirs(indirs,[treename],
							sigtag,sidebanddirs=sidebanddirs)
	    tloader = tloader_fromchains(sigcs,bckcs,bckcs_train,bckcs_test,
					varlist,lopts,outfilename)
	    aucroc = tmvatrain(tloader,fopts,outfilename)
	    print('results for '+outfilename)
	    print('---AUC (ROC)---')
	    print(str(aucroc))
    elif(combine=='regions'):
        for indir,year in zip(indirs,years):
            outfilename = 'out_'+year+'.root'
            (sigcs,bckcs,bckcs_train,bckcs_test,_,_) = getmvatreesfromdirs([indir],treenames,
							sigtag,sidebanddirs=sidebanddirs)
            tloader = tloader_fromchains(sigcs,bckcs,bckcs_train,bckcs_test,
					varlist,lopts,outfilename)
            aucroc = tmvatrain(tloader,fopts,outfilename)
	    print('results for '+outfilename)
            print('---AUC (ROC)---')
            print(str(aucroc))
    else:
	print('not yet implemented')
