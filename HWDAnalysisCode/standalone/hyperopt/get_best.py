######################################################
# get the best configuration from a hyperopt process #
######################################################

import sys
import os
import pickle as pkl
import run_hyperopt as hp
sys.path.append('../')
import optiontools as opt

def get_best_indices(trials, nbest=1):
    losses = trials.losses()
    if nbest>len(losses): nbest = len(losses)
    sorted_indices = sorted(range(len(losses)), key=lambda k: losses[k])
    return sorted_indices[:nbest]

def get_best_attachments(trials, nbest=1, attachments=None):
    # old method where info is stored as 'attachment' in the trials object;
    # need to specify the names of each attachment explicitly...
    best_info = []
    losses = trials.losses()
    ids = get_best_indices(trials, nbest=nbest)
    for idx in ids:
	this_info = {}
        this_info['loss'] = losses[idx]
        attachmentsobj = trials.trial_attachments(trials.trials[idx])
	for attachment in attachments:
	    this_info[attachment] = attachmentsobj[attachment]
        this_info['config'] = {}
        for name,val in trials.trials[idx]['misc']['vals'].items():
            this_info['config'][name] = val
	best_info.append(this_info)
    return best_info

def get_best_info(trials, nbest=1):
    # new method where info is stored in a normal dict 'extra_info' in the trials object;
    # allows to simply loop over all keys present in the dict
    best_info = []
    losses = trials.losses()
    ids = get_best_indices(trials, nbest=nbest)
    for idx in ids:
        this_info = {}
        this_info['loss'] = losses[idx]
        infodict = trials.trials[idx]['result']['extra_info']
        for key,val in infodict.items():
            this_info[key] = val
        this_info['config'] = {}
        for name,val in trials.trials[idx]['misc']['vals'].items():
            this_info['config'][name] = val
        best_info.append(this_info)
    return best_info
    
if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('nbest', vtype='int', default=1) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # other options (hard-coded for now)
    attachments = ['nsignal','nbackground']

    # parse options
    if( options.inputfile is not None and options.inputdir is not None ):
	raise Exception('ERROR: inputfile and inputdir cannot both be specified.')

    # case 1: print results for one input file
    if( options.inputfile is not None ):    

	with open(options.inputfile,'r') as f:
	    trials = pkl.load(f)
	#info = get_best_attachments(trials, nbest = options.nbest, attachments=attachments)
	info = get_best_info(trials, nbest = options.nbest)
	for idx in range(len(info)):
	    print('--- configuration {} ---'.format(idx))
	    print('loss: {}'.format(info[idx]['loss']))
	    for key,val in info[idx].items(): 
		if key=='loss': continue   # already printed
		if key=='config': continue # will be printed later
		print('{}: {}'.format(key, val))
	    print('config:')
	    for name,val in info[idx]['config'].items(): print('  - {}: {}'.format(name,val))

    # case 2: compare best result of multiple files
    if( options.inputdir is not None ):	
	infiles = []
        jobdirs = [f for f in os.listdir(options.inputdir) if 'job' in f]
        for jobdir in jobdirs:
            infile = os.path.join(options.inputdir,jobdir,'trials.pkl')
            if not os.path.exists(infile):
                print('WARNING: found directory {} in {},'.format(jobdir, inputdir)
                        +' but it does not seem to contain a "trials.pkl" file')
                continue
            infiles.append(infile)

	for infile in infiles:
	    with open(infile,'r') as f:
		trials = pkl.load(f)
	    #info = get_best_attachments(trials, nbest=1, attachments=attachments)
	    info = get_best_info(trials, nbest = options.nbest)
	    for idx in range(len(info)):
		print('--- configuration {} ---'.format(infile))
		print('loss: {}'.format(info[idx]['loss']))
		for key,val in info[idx].items(): 
		    if key=='loss': continue   # already printed
		    if key=='config': continue # don't print this
		    print('{}: {}'.format(key, val))
