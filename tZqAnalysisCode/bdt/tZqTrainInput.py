#name of input root file, relative to the directory of this script
root_file_name = 'tZqTrainInput.root'

#names of trees that contain signal and background events
signal_tree_name = 'sigtree'
background_tree_name = 'bcktree'

#list of variables to be used in training (corresponding to tree branches)
list_of_branches = ['_abs_eta_recoil', '_Mjj_max', '_lW_asymmetry', '_deepCSV_max', '_lT', '_MT', '_dPhill_max', '_pTjj_max', '_dRlb_min', '_HT', '_dRlWrecoil', '_dRlWbtagged', '_M3l', '_abs_eta_max', '_nJets', '_nBJets', ]

#branch that indicates the event weights
weight_branch = '_normweight'

#use only positive weights in training or not
only_positive_weights = True

#validation and test fractions
validation_fraction = 0.2
test_fraction = 0.2

#number of threads to use when training
number_of_threads = 1

#use BDT or NN
mode = 'BDT'

#use genetic algorithm or grid-scan for optimization
use_genetic_algorithm = False
high_memory = False

#set stop conditions for genetic algorithm
max_generations = 100
min_generations = 5
min_average_improvement = 0.05

if use_genetic_algorithm:
	population_size = 100
	#ranges of parameters for the genetic algorithm to scan
	if mode=='BDT':
	    parameter_ranges = {
		'number_of_trees' : list( range(100, 10000) ),
		'learning_rate' : (0.001, 1),
		'max_depth' : list( range(2, 10) ),
		'min_child_weight' : (1, 20),
		'subsample' : (0.1, 1),
		'colsample_bytree' : (0.5, 1),
		'gamma' : (0, 1),
		'alpha' : (0, 1)
		}

	elif mode=='NN':
	    parameter_ranges = {
		'number_of_hidden_layers' : list(range(1, 10) ),
		'units_per_layer' : list( range(16, 1024) ),
		'optimizer' : ['RMSprop', 'Adagrad', 'Adadelta', 'Adam', 'Adamax', 'Nadam'],
		'learning_rate' : (0.01, 1),
		'learning_rate_decay' : (0.9, 1),
		'dropout_first' : (False, True),
		'dropout_all' : (False, True),
		'dropout_rate' : (0, 0.5),
		'number_of_epochs' : [20],
		'batch_size' : list( range( 32, 256 ) )
		}

	else:
	    print('### WARNING ###: mode '+mode+' not recognized, will crash...')

else:
	if mode=='BDT':
	    parameter_values = {
		'number_of_trees' : [10, 20, 30],
		'learning_rate' : [0.05, 0.1, 0.2],
		'max_depth' : [3, 4],
		'min_child_weight' : [5],
		'subsample' : [1],
		'colsample_bytree' : [1],
		'gamma' : [0],
		'alpha' : [0]
		}

	elif mode=='NN':
	    parameter_values = {
		'number_of_hidden_layers' : [1, 2, 3, 4, 5],
		'units_per_layer' : [8, 16, 32, 64],
		'optimizer' : ['Nadam'],
		'learning_rate' : [0.1, 1, 0.01],
		'learning_rate_decay' : [1, 0.99, 0.95],
		'dropout_first' : [False, True],
		'dropout_all' : [False, True],
		'dropout_rate' : [0.3],
		'number_of_epochs' : [100],
		'batch_size' : [128]
		}

	else:
	    print('### WARNING ###: mode '+mode+' not recognized, will crash...')

