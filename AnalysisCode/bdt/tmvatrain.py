############################################################
# A script to train a tZq signal BDT with the TMVA package #
############################################################
import ROOT
import sys
import os
import json
#import tmvautils as u

def tmvatrain(indirs,treenames,sigtag,variables,tloader_options,tfactory_options):
    ### Initialize Factory and DataLoader
    outfile = ROOT.TFile('out.root',"recreate")
    tfactory = ROOT.TMVA.Factory('tmvatrain',outfile,"")
    tloader = ROOT.TMVA.DataLoader('outdata')

    ### Add signal and background to DataLoader
    sigchains = [] # separate chains per treename seem to be necessary...
    bckchains = []
    nsig = 0
    nbck = 0
    for treename in treenames:
	sigchains.append(ROOT.TChain(treename))
	bckchains.append(ROOT.TChain(treename))
    for indir in indirs:
	filelist = [f for f in os.listdir(indir) if f[-5:]=='.root']
	for filename in filelist:
	    fullpath = os.path.join(indir,filename)
	    temp = ROOT.TFile.Open(fullpath)
	    for i,treename in enumerate(treenames):
		if sigtag in filename:
		    try:
			sigchains[i].Add(fullpath+'/'+treename)
			nsig += temp.Get(treename).GetEntries()
			print('Found signal file: '+filename+' ('+treename+')')
		    except:
			print('An error occurred for '+filename+' ('+treename+')')
		else:
		    #if 'DYJets' in filename: continue
		    try:
			nbckf = temp.Get(treename).GetEntries()
			if nbckf==0: continue
			bckchains[i].Add(fullpath+'/'+treename)
			nbck += nbckf
			print('Found background file: '+filename+' ('+treename+')')
		    except:
			print('An error occurred for '+filename+' ('+treename+')')
    print('Found '+str(nsig)+' signal events and '+str(nbck)+' background events.')
    for sigchain in sigchains: tloader.AddSignalTree(sigchain,1.0)
    for bckchain in bckchains: tloader.AddBackgroundTree(bckchain,1.0)
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
	indirs = [os.path.abspath('/user/llambrec/Files/tzqid/signalregion_wp0p6/2016MC_flat')]
	indirs.append(os.path.abspath('/user/llambrec/Files/tzqid/signalregion_wp0p6/2017MC_flat'))
	indirs.append(os.path.abspath('/user/llambrec/Files/tzqid/signalregion_wp0p6/2018MC_flat'))
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
    aucroc = tmvatrain(indirs,treenames,sigtag,
			varlist,lopts,fopts)
    print('---AUC (ROC)---')
    print(str(aucroc))
