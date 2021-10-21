#########################################
# plot the loss from a hyperopt process #
#########################################

import sys
import os
import pickle as pkl
import run_hyperopt as hp
sys.path.append('../')
import optiontools as opt

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
    options.append( opt.Option('sort', vtype='bool', default=False) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # parse arguments
    if( options.inputfile is not None and options.inputdir is not None ):
	raise Exception('ERROR: specify inputfile or inputdir, not both')

    infiles = []
    # set input files in case a file is given
    if( options.inputfile is not None): infiles = [optons.inputfile]
    # set input files in case a directory is given
    elif( options.inputdir is not None ):
	jobdirs = [f for f in os.listdir(options.inputdir) if 'job' in f]
	for jobdir in jobdirs:
	    infile = os.path.join(options.inputdir,jobdir,'trials.pkl')
	    if not os.path.exists(infile):
		print('WARNING: found directory {} in {},'.format(jobdir, inputdir)
			+' but it does not seem to contain a "trials.pkl" file')
		continue
	    infiles.append(infile)
    
    # load the losses
    losses = []
    for infile in infiles:
	with open(infile,'r') as f:
	    trials = pkl.load(f)
	loss = trials.losses()
	if options.sort: loss.sort(reverse=True)
	losses.append( loss )

    # make the figure
    (fig,ax) = hp.loss_plot( losses, title=None,
                xlims=None, xaxtitle='iteration',
                yaxlog=False, yaxtitle='loss' )
    fig.savefig(options.outputfile)
