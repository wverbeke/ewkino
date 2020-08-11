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

def gettrees(indirs,treenames,sigtag,sidebanddirs=[]):
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
	    fullpath = os.path.join(indir,filename)
	    temp = ROOT.TFile.Open(fullpath)
	    for treename in treenames:
		if sigtag in filename:
		    try:
			sigchains.append({'chain':ROOT.TChain(treename),'weight':1.0})
			sigchains[-1]['chain'].Add(fullpath+'/'+treename)
			nsig += temp.Get(treename).GetEntries()
			print('Found signal file: '+filename+' ('+treename+')')
		    except:
			print('An error occurred for '+filename+' ('+treename+')')
		else:
		    # skip DY:
		    #if 'DYJets' in filename: continue
		    #try:
			nbckf = temp.Get(treename).GetEntries()
                        if nbckf==0: continue
			if(len(sidebanddirs)==0 or len(sidebanddirs[indirs.index(indir)])==0):
			    # add regular file with original weight
                            bckchains.append({'chain':ROOT.TChain(treename),'weight':1.0})
                            bckchains[-1]['chain'].Add(fullpath+'/'+treename)
			    print('Found background file: '+filename+' ('+treename+')')
			else:
			    # check whether file has sideband equivalent
			    sfile = pathtosidebandfile(filename,sidebanddirs[indirs.index(indir)])
			    if len(sfile)==0:
				# add regular file with original weight
                                bckchains.append({'chain':ROOT.TChain(treename),'weight':1.0})
                                bckchains[-1]['chain'].Add(fullpath+'/'+treename)
				print('Found background file: '+filename+' ('+treename+')')
			    else:
				# method 1:
				# add regular and sideband file with proper weight
				weight = getscalefactor([fullpath,sfile],[treename,treename],nbckf)
                                bckchains.append({'chain':ROOT.TChain(treename),'weight':weight})
                                bckchains[-1]['chain'].Add(fullpath+'/'+treename)
                                print('Found background file: '+filename+' ('+treename+') with weight '
                                       +str(weight))
                                bckchains.append({'chain':ROOT.TChain(treename),'weight':weight})
                                bckchains[-1]['chain'].Add(sfile+'/'+treename)
                                print('Found sideband background file: '+sfile.split('/')[-1]
                                        +' ('+treename+') with weight '+str(weight))
				# method 2:
				'''# add regular and sideband file to training with proper weight
				weight = getscalefactor([sfile],[treename],nbckf)
				#bckchains_onlytrain.append({'chain':ROOT.TChain(treename),'weight':weight})
				#bckchains_onlytrain[-1]['chain'].Add(fullpath+'/'+treename)
				#print('Found background file: '+filename+' ('+treename+') with weight '
				#	+str(weight))
				bckchains_onlytrain.append({'chain':ROOT.TChain(treename),'weight':weight})
				bckchains_onlytrain[-1]['chain'].Add(sfile+'/'+treename)
				print('Found sideband background file: '+sfile.split('/')[-1]
					+' ('+treename+') with weight '+str(weight))
				# add regular file to testing with original weight
				bckchains_onlytest.append({'chain':ROOT.TChain(treename),'weight':1.0})
				bckchains_onlytest[-1]['chain'].Add(fullpath+'/'+treename)'''
			nbck += nbckf
		    #except:
			#print('An error occurred for '+filename+' ('+treename+')')
    print('Found '+str(nsig)+' signal events and '+str(nbck)+' background events.')
    return (sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest)

def tmvatrain(sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,
		variables,tloader_options,tfactory_options):
    ### Initialize Factory and DataLoader
    outfile = ROOT.TFile('out.root',"recreate")
    tfactory = ROOT.TMVA.Factory('tmvatrain',outfile,"")
    tloader = ROOT.TMVA.DataLoader('outdata')
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

    ### Book BDT method
    print(tfactory_options)
    tfactory.BookMethod(tloader,ROOT.TMVA.Types.kBDT,"BDT",tfactory_options)

    ### Train and test method
    tfactory.TrainAllMethods()
    tfactory.TestAllMethods()
    tfactory.EvaluateAllMethods()
    roc = outfile.Get("outdata/Method_BDT/BDT/MVA_BDT_rejBvsS")
    aucroc = roc.Integral(1,roc.GetNbinsX(),"width")
    outfile.Close()
    return aucroc

def argsfromcmd(args):
    ### Check input args from command line
    argkeys = []
    rqargkeys = (['indirs','treenames','sigtag',
		    'varlist','lopts','fopts'])
    for arg in args:
	(key,value) = tuple(arg.split('=',1))
	if key=='indirs': 
	    indirs=[os.path.abspath(str(x)) for x in json.loads(value)]; 
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
    return (indirs,treenames,sigtag,varlist,lopts,fopts)

if __name__=="__main__":
    
    if len(sys.argv)>1:
	args = sys.argv[1:]
	(indirs,treenames,sigtag,varlist,lopts,fopts) = argsfromcmd(args)
    else:
	### Set list of input files
	indirs = [os.path.abspath('/user/llambrec/Files/tthid/signalregion/2016MC_flat')]
	indirs.append(os.path.abspath('/user/llambrec/Files/tthid/signalregion/2017MC_flat'))
	indirs.append(os.path.abspath('/user/llambrec/Files/tthid/signalregion/2018MC_flat'))
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
	fopts += ":NTrees=100:BoostType=Grad" # options for ensemble
	fopts += ":MinNodeSize=1%:MaxDepth=4:nCuts=200" # options for single tree
	fopts += ":UseBaggedGrad=True:BaggedSampleFraction=1."
	fopts += ":Shrinkage=0.05"
	#fopts += ":IgnoreNegWeightsInTraining=True"
	### variables
	varlist = (['_abs_eta_recoil','_Mjj_max','_lW_asymmetry',
		'_deepCSV_max','_lT','_MT','_dPhill_max',
		'_pTjj_max','_dRlb_min','_HT','_dRlWrecoil','_dRlWbtagged',
		'_M3l','_abs_eta_max'])
	varlist += (['_nJets','_nBJets']) # parametrized learning
    ### Function call
    (sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest) = gettrees(indirs,treenames,sigtag,sidebanddirs=sidebanddirs)
    aucroc = tmvatrain(sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,
			varlist,lopts,fopts)
    print('---AUC (ROC)---')
    print(str(aucroc))
