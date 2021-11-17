#################################################################
# run plot_evaluation in a loop for the best gridsearch results #
#################################################################

import sys
import os
import pickle as pkl
import matplotlib.pyplot as plt
sys.path.append('../tools')
import optiontools as opt

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('evaldata', vtype='path') )
    options.append( opt.Option('metric', default='val_loss') )
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

    # find input files
    infiles = []
    jobdirs = [f for f in os.listdir(options.inputdir) if 'job' in f]
    for jobdir in jobdirs:
	infile = os.path.join(options.inputdir,jobdir,'history.pkl')
	if not os.path.exists(infile):
	    print('WARNING: found directory {} in {},'.format(jobdir, options.inputdir)
		    +' but it does not seem to contain a "history.pkl" file')
	    continue
	infiles.append(infile)
    
    # load the results
    losses = []
    for infile in infiles:
	with open(infile,'r') as f:
	    history = pkl.load(f)
	if options.metric in history:
	    losses.append( history[options.metric][-1] )
	else: raise Exception('ERROR: metric {} not in file {}'.format(options.metric,infile))

    # sort the losses and corresponding job directories
    sorted_inds = [i[0] for i in sorted(enumerate(losses), key=lambda x:x[1])]
    losses = [losses[i] for i in sorted_inds]
    infiles = [infiles[i] for i in sorted_inds]
    if options.nbest<len(losses):
	losses = losses[:options.nbest]
	infiles = infiles[:options.nbest]

    # run plot_evaluation
    for i,infile in enumerate(infiles):
	dirname = os.path.dirname(infile)
	print('running on file {} of {}'.format(i+1, len(infiles)))
	print('({})'.format(dirname))
	model = os.path.join(dirname,'model')
	outputfile = os.path.join(dirname,'evaluation')
	cmd = 'python plot_evaluation.py'
	cmd += ' --model {}'.format(model)
	cmd += ' --evaldata {}'.format(options.evaldata)
	cmd += ' --outputfile {}'.format(outputfile)
	os.system(cmd)
