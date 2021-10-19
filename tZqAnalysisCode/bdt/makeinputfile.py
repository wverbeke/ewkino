#######################################################################################
# intermediary script to generate the necessary input for Willem's deep learning repo #
#######################################################################################

import os
import sys
import ROOT
from tmvatrain import getmvatreesfromdirs

def mergetrees(treelist,outputtreename):
    # merge all trees in treelist into one single outputtree
    treeTlist = ROOT.TList()
    for tree in treelist:
	tree.SetName(outputtreename)
	treeTlist.Add(tree)
    outputtree = ROOT.TTree.MergeTrees(treeTlist)
    outputtree.SetDirectory(0)
    outputtree.SetName(outputtreename)
    outputtree.SetTitle(outputtreename)
    return outputtree

def mergetreesinfolders(outfilename,indirs,treenames,sigtag):
    # get lists of TChains containging all files in all input directories
    print('collecting input root files...')
    (sigchains,bckchains,bckchains_onlytrain,bckchains_onlytest,_,_) = getmvatreesfromdirs(indirs,treenames,sigtag)
    # note: not yet implemented how to include onlytrain or onlytest,
    #       nor the 'weight' attribute of the chain dicts...
    # merge all chains into one tree and delete original chains 
    # (to avoid implicit writing to file)
    print('merging all root trees...')
    sigtree = mergetrees([chain['chain'] for chain in sigchains],'sigtree')
    del sigchains
    bcktree = mergetrees([chain['chain'] for chain in bckchains],'bcktree')
    del bckchains
    del bckchains_onlytrain
    del bckchains_onlytest
    # create output file and write trees
    print('writing merged trees to output file...')
    if '.' in outfilename: outfilename = outfilename.rplit(1)[-1]
    outfilename = outfilename + '.root'
    outfile = ROOT.TFile(outfilename,'recreate')
    sigtree.Write()
    bcktree.Write()
    outfile.Close()
    print('done')

def makeconfigurationfile(filename,path_to_root_file,variables):
    print('writing configuration file...')
    if '.' in filename: filename = filename.rplit(1)[-1]
    filename = filename + '.py'
    if '.' in path_to_root_file: path_to_root_file = path_to_root_file.rplit(1)[-1]
    path_to_root_file = path_to_root_file + '.root'
    with open(filename,'w') as cfile:
	cfile.write('#name of input root file, relative to the directory of this script\n')
	cfile.write('root_file_name = \''+path_to_root_file+'\'\n\n')
	cfile.write('#names of trees that contain signal and background events\n') 
	cfile.write('signal_tree_name = \'sigtree\'\n')
	cfile.write('background_tree_name = \'bcktree\'\n\n')
	cfile.write('#list of variables to be used in training (corresponding to tree branches)\n')
	cfile.write('list_of_branches = [')
	for variable in variables: cfile.write('\''+variable+'\', ')
	cfile.write(']\n\n')
	cfile.write('#branch that indicates the event weights\n')
	cfile.write('weight_branch = \'_normweight\'\n\n')
	cfile.write('#use only positive weights in training or not\n')
	cfile.write('only_positive_weights = True\n\n')
	cfile.write('#validation and test fractions\n')
	cfile.write('validation_fraction = 0.2\n')
	cfile.write('test_fraction = 0.2\n\n')
	cfile.write('#number of threads to use when training\n')
	cfile.write('number_of_threads = 1\n\n')
	cfile.write('#use BDT or NN\n')
	cfile.write('mode = \'BDT\'\n\n')
	cfile.write('#use genetic algorithm or grid-scan for optimization\n')
	cfile.write('use_genetic_algorithm = False\n')
	cfile.write('high_memory = False\n\n')
	cfile.write('#set stop conditions for genetic algorithm\n')
	cfile.write('max_generations = 100\n')
	cfile.write('min_generations = 5\n')
	cfile.write('min_average_improvement = 0.05\n\n')
	cfile.write('if use_genetic_algorithm:\n')
	cfile.write('	population_size = 100\n')
	cfile.write('	#ranges of parameters for the genetic algorithm to scan\n')
	cfile.write('	if mode==\'BDT\':\n')
	cfile.write('	    parameter_ranges = {\n')
	cfile.write('		\'number_of_trees\' : list( range(100, 10000) ),\n')
	cfile.write('		\'learning_rate\' : (0.001, 1),\n')
	cfile.write('		\'max_depth\' : list( range(2, 10) ),\n')
	cfile.write('		\'min_child_weight\' : (1, 20),\n')
	cfile.write('		\'subsample\' : (0.1, 1),\n')
	cfile.write('		\'colsample_bytree\' : (0.5, 1),\n')
	cfile.write('		\'gamma\' : (0, 1),\n')
	cfile.write('		\'alpha\' : (0, 1)\n')
	cfile.write('		}\n\n')
	cfile.write('	elif mode==\'NN\':\n')
        cfile.write('	    parameter_ranges = {\n')
        cfile.write('		\'number_of_hidden_layers\' : list(range(1, 10) ),\n')
        cfile.write('		\'units_per_layer\' : list( range(16, 1024) ),\n')
        cfile.write('		\'optimizer\' : [\'RMSprop\', \'Adagrad\', \'Adadelta\', \'Adam\', \'Adamax\', \'Nadam\'],\n')
        cfile.write('		\'learning_rate\' : (0.01, 1),\n')
        cfile.write('		\'learning_rate_decay\' : (0.9, 1),\n')
        cfile.write('		\'dropout_first\' : (False, True),\n')
        cfile.write('		\'dropout_all\' : (False, True),\n')
        cfile.write('		\'dropout_rate\' : (0, 0.5),\n')
        cfile.write('		\'number_of_epochs\' : [20],\n')
        cfile.write('		\'batch_size\' : list( range( 32, 256 ) )\n')
	cfile.write('		}\n\n')
	cfile.write('	else:\n')
	cfile.write('	    print(\'### WARNING ###: mode \'+mode+\' not recognized, will crash...\')\n\n')
	cfile.write('else:\n')
	cfile.write('	if mode==\'BDT\':\n')
	cfile.write('	    parameter_values = {\n')
	cfile.write('		\'number_of_trees\' : [10, 20, 30],\n')
	cfile.write('		\'learning_rate\' : [0.05, 0.1, 0.2],\n')
	cfile.write('		\'max_depth\' : [3, 4],\n')
	cfile.write('		\'min_child_weight\' : [5],\n')
	cfile.write('		\'subsample\' : [1],\n')
	cfile.write('		\'colsample_bytree\' : [1],\n')
	cfile.write('		\'gamma\' : [0],\n')
	cfile.write('		\'alpha\' : [0]\n')
	cfile.write('		}\n\n')
	cfile.write('	elif mode==\'NN\':\n')
        cfile.write('	    parameter_values = {\n')
        cfile.write('		\'number_of_hidden_layers\' : [1, 2, 3, 4, 5],\n')
        cfile.write('		\'units_per_layer\' : [8, 16, 32, 64],\n')
        cfile.write('		\'optimizer\' : [\'Nadam\'],\n')
        cfile.write('		\'learning_rate\' : [0.1, 1, 0.01],\n')
	cfile.write('		\'learning_rate_decay\' : [1, 0.99, 0.95],\n')
	cfile.write('		\'dropout_first\' : [False, True],\n')
        cfile.write('		\'dropout_all\' : [False, True],\n')
        cfile.write('		\'dropout_rate\' : [0.3],\n')
        cfile.write('		\'number_of_epochs\' : [100],\n')
        cfile.write('		\'batch_size\' : [128]\n')
	cfile.write('		}\n\n')
	cfile.write('	else:\n')
	cfile.write('	    print(\'### WARNING ###: mode \'+mode+\' not recognized, will crash...\')\n\n')
	print('done')

if __name__=="__main__":

    ### set output file name (dont put extension; .root and .py will be added automatically)
    outfilename = 'tZqTrainInput'
    ### set directory where to put the output
    destiny = os.path.abspath('.')
    ### set input parameters
    indirs = []
    indirs.append(os.path.abspath('/user/llambrec/Files/tzqid_new/2016MC/signalregion_3tight_flat'))
    indirs.append(os.path.abspath('/user/llambrec/Files/tzqid_new/2017MC/signalregion_3tight_flat'))
    indirs.append(os.path.abspath('/user/llambrec/Files/tzqid_new/2018MC/signalregion_3tight_flat'))
    sidebanddirs = []
    #sidebanddirs = [f.replace('_flat','_2tight_flat') for f in indirs]
    treenames = ["blackJackAndHookers/treeCat1"]
    treenames.append("blackJackAndHookers/treeCat2")
    treenames.append("blackJackAndHookers/treeCat3")
    sigtag = 'tZq'
    variables = (['_abs_eta_recoil','_Mjj_max','_lW_asymmetry',
                '_deepCSV_max','_lT','_MT','_dPhill_max',
                '_pTjj_max','_dRlb_min','_HT','_dRlWrecoil','_dRlWbtagged',
                '_M3l','_abs_eta_max'])
    variables += (['_nJets','_nBJets']) # parametrized learning
    ### merge trees
    mergetreesinfolders(outfilename,indirs,treenames,sigtag)
    ### write configuration file
    makeconfigurationfile(outfilename,outfilename,variables)
    ### move files to destiny
    if not os.path.exists(destiny):
	print('### WARNING ###: destiny folder '+destiny+' does not seem to exist;')
	print('files are stored in current working directory.')
	sys.exit()
    if( destiny=='.' or destiny=='' or os.path.abspath(destiny)==os.path.abspath(os.getcwd()) ):
	print('files are stored in current working directory.')
        sys.exit()
    if os.path.exists(outfilename+'.root'):
	print('copying root file to destiny folder '+destiny)
	try:
	    os.system('cp '+outfilename+'.root '+destiny)
	    print('done')
	except:
	    print('### WARNING ###: something seems to have gone wrong')
    if os.path.exists(outfilename+'.py'):
	print('copying root file to destiny folder '+destiny)
        try:
	    os.system('cp '+outfilename+'.py '+destiny)
	    print('done')
	except:
	    print('### WARNING ###: something seems to have gone wrong')
