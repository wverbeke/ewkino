##############################
# submitter for run_hyperopt #
##############################

import ROOT
import sys
import os
import pickle as pkl
sys.path.append('../tools')
import optiontools as opt
import qsubTools as qt 

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('treename', default='blackJackAndHookers/blackJackAndHookersTree') )
    options.append( opt.Option('nprocess', vtype='int', default=-1) )
    options.append( opt.Option('gridfile', vtype='path') )
    options.append( opt.Option('nevals', vtype='int') )
    options.append( opt.Option('nstartup', vtype='int', default=1) )
    options.append( opt.Option('outputdir', vtype='path') )
    options.append( opt.Option('njobs', vtype='int', default=1) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # parse options
    if not os.path.exists(options.inputfile): 
	raise Exception('ERROR: input file does not exist.')
    try:
	fin = ROOT.TFile.Open(options.inputfile)
	tree = fin.Get(options.treename)
	nentries = tree.GetEntries()
	fin.Close()
    except:
	raise Exception('ERROR: could not access input tree')
    try:
	with open(options.gridfile,'r') as f:
	    grid = pkl.load(f)['grid']
    except:
	raise Exception('ERROR: could not load the grid')
    if( options.nevals is None ): 
	raise Exception('ERROR: please set "nevals" argument.')
    if os.path.exists(options.outputdir):
	print('WARNING: output directory already exists: {}'.format(options.outputdir))
	print('overwrite? (y/n)')
	go = raw_input()
	if not go=='y':
	    sys.exit()
	os.system('rm -r {}'.format(options.outputdir))
    os.makedirs(options.outputdir)

    # options hardcoded for now
    nvar = '_nDs'
    signalvar = '_DHasFastGenMatch'
    lossfunction = 'negstosqrtn'

    # loop over number of jobs:
    for jobn in range(options.njobs):
	
	thisjobdir = os.path.join(options.outputdir,'job{}'.format(jobn))
	os.makedirs(thisjobdir)
	thisjoboutfile = os.path.join(thisjobdir,'trials.pkl')
	thisjoblogfile = os.path.join(thisjobdir,'log.txt')

	cmd = 'python run_hyperopt.py'
	cmd += ' --inputfile {}'.format(options.inputfile)
	cmd += ' --treename {}'.format(options.treename)
	cmd += ' --nvar {}'.format(nvar)
	cmd += ' --signalvar {}'.format(signalvar)
	cmd += ' --nprocess {}'.format(options.nprocess)
	cmd += ' --gridfile {}'.format(options.gridfile)
	cmd += ' --nevals {}'.format(options.nevals)
	cmd += ' --nstartup {}'.format(options.nstartup)
	cmd += ' --lossfunction {}'.format(lossfunction)
	cmd += ' --outputfile {}'.format(thisjoboutfile)
	cmd += ' --logfile {}'.format(thisjoblogfile)

	cmds = []
	cmds.append('source ~/venv_hyperopt/bin/activate')
	cmds.append(cmd)

	# print each command before executing it
	newcmds = []
	for cmd in cmds:
	    newcmds.append("echo '[cmd]: {}'".format(cmd))
	    newcmds.append(cmd)
	cmds = newcmds

	# submit job
	script_name = 'qsub_run_hyperopt.sh'
	qt.submitCommandsAsQsubJob( cmds, script_name )		
