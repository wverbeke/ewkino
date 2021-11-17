#####################################
# run hyperopt for cut optimization #
#####################################

import ROOT
import sys
import os
import pickle as pkl
from hyperopt import hp, fmin, tpe, STATUS_OK, Trials
from functools import partial
sys.path.append('../tools')
import optiontools as opt
import metrictools as mt
import logtools as lt

### define help functions ###

def pass_cuts( cuts, tree, evtidx, instidx=None ):

    tree.GetEntry(evtidx)
    for cutname, cutvalue in cuts.items():
	# parse cut name
	ismax = (cutname[-4:]=='_max')
	ismin = (cutname[-4:]=='_min')
	if( not (ismax or ismin) ): 
	    raise Exception('ERROR: cut {} is neither min nor max.'.format(cutname))
	varname = cutname[:-4]
	# get the variable value
	varvalue = 0.
	try:
	    varvalue = getattr(tree, varname)
	    if instidx is not None: varvalue = varvalue[instidx]
	except:
	    raise Exception('ERROR: could not get value {} from tree.'.format(varname))
	# perform the cut
	if( ismax and varvalue>cutvalue): return False
	if( ismin and varvalue<cutvalue): return False
    return True

def calculate_loss( tree, cuts, lossfunction='negstob',
			    nvar=None, signalvar='_isSignal', nprocess=-1,
			    iteration=None, maxiterations=None ):
    ### calculate signal significance for a given configuration of cuts
    # input arguments:
    # - lossfunction: name of a member function of a MetricCalculator object,
    #                 to be used as loss definition
    # - nvar: name of the variable defining the size of the branches per event 
    #         (use None for event-level variables)
    #         (e.g. '_nDs' for the current application)
    # - signalvar: name of the variable defining if an instance is signal
    #              (e.g. '_DHasFastGenMatch' for the current application)
    # - nprocess: number of events to process
    # - iteration: list with one element, i.e. number of current iteration 
    #              (only used for printing progress)
    # - maxiterations: maximum number of iterations
    #                  (only used for printing progress)

    # print progress
    print('now processing iteration {} of {}'.format(iteration[0],maxiterations))
    iteration[0] += 1

    # initializations
    nsignal_tot = 0
    nsignal_pass = 0
    nbackground_tot = 0
    nbackground_pass = 0
    nevents = tree.GetEntries()
    if( nprocess<0 or nprocess>nevents ): nprocess = nevents
    
    # loop over events and instances
    for evtidx in range(nprocess):
	tree.GetEntry(evtidx)
	# case 1: one instance per entry
	if nvar is None:
	    if getattr(tree, signalvar): nsignal_tot += 1
            else: nbackground_tot += 1
	    if pass_cuts( cuts, tree, evtidx ): 
		if getattr(tree, signalvar): nsignal_pass += 1
		else: nbackground_pass += 1
	# case 2: multiple instances per entry
	else:
	    ninstances = getattr(tree, nvar)
	    for instidx in range(ninstances):
		if getattr(tree, signalvar)[instidx]: nsignal_tot += 1
                else: nbackground_tot += 1
		if pass_cuts( cuts, tree, evtidx, instidx=instidx ): 
		    if getattr(tree, signalvar)[instidx]: nsignal_pass += 1
		    else: nbackground_pass += 1
    if( nsignal_pass==0 or nbackground_pass==0 ): 
	loss=0
	lossfunction = 'none'
    else:
	metcalc = mt.MetricCalculator( nsignal_tot, nsignal_pass, 
					nbackground_tot, nbackground_pass )
	loss = getattr(metcalc, lossfunction)()
    extra_info = {'nsignal_tot': nsignal_tot,
		  'nsignal_pass': nsignal_pass,
		  'nbackground_tot': nbackground_tot,
		  'nbackground_pass': nbackground_pass,
		  'lossfunction': lossfunction}
    return {'loss':loss, 'status':STATUS_OK, 
	    'extra_info': extra_info}


if __name__=='__main__':

    # read options
    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('treename', default='blackJackAndHookers/blackJackAndHookersTree') )
    options.append( opt.Option('nvar', vtype='str') )
    options.append( opt.Option('signalvar', vtype='str') )
    options.append( opt.Option('nprocess', vtype='int', default=-1) )
    options.append( opt.Option('gridfile', vtype='path') )
    options.append( opt.Option('nevals', vtype='int') )
    options.append( opt.Option('nstartup', vtype='int', default=1) )
    options.append( opt.Option('lossfunction') )
    options.append( opt.Option('outputfile', vtype='path') )
    options.append( opt.Option('logfile', vtype='path') )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )

    # set logfile first and redirect all output if needed
    if options.logfile is not None:
	if( os.path.exists(options.logfile) ):
	    os.system('rm {}'.format(options.logfile))
	log = lt.StdOutRedirector(options.logfile)
	sys.stdout = log
	sys.stderr = log
    print('### starting ###\n')
    print('Found following configuration:')
    print(options)
    print('')

    # parse options
    if( options.nevals is None ): raise Exception('ERROR: please set "nevals" argument.')
    try:
	testmetriccalc = mt.MetricCalculator(10,9,10,1)
	testmetric = getattr(testmetriccalc, options.lossfunction)
    except:
    	raise Exception('ERROR: please provide a valid loss function.')

    # get the input tree
    fin = ROOT.TFile.Open(options.inputfile)
    tree = fin.Get(options.treename)
    
    # parse arguments (continued)
    tree.GetEntry(0)
    try: testval = getattr(tree, options.signalvar)
    except: raise Exception('ERROR: please provide a valid signal variable.')
    try: testval = getattr(tree, options.nvar)
    except: raise Exception('ERROR: please provide a valid instance counting variable.')    

    # get the grid
    with open(options.gridfile,'r') as f:
	obj = pkl.load(f)
	grid = obj['grid']
	gridstr = obj['description']
    print('Found following grid:')
    print(gridstr)
    print('')

    # run hyperopt
    trials = Trials()
    iteration = [1]
    best = fmin( fn=partial(calculate_loss, tree, 
			    lossfunction=options.lossfunction,
			    nvar=options.nvar, signalvar=options.signalvar, 
			    nprocess=options.nprocess,
			    iteration=iteration, maxiterations=options.nevals),
		space=grid,
		algo=partial(tpe.suggest, n_startup_jobs=options.nstartup),
		max_evals=options.nevals,
		trials=trials)

    # write search results to output file
    if options.outputfile is not None:
	print('Writing results to {}'.format(options.outputfile))
	with open(options.outputfile,'w') as f:
	    pkl.dump(trials,f)
    print('### done ###')
