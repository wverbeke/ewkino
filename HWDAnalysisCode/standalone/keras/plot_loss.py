#########################################
# plot the loss from a training process #
#########################################

import sys
import os
import pickle as pkl
sys.path.append('../tools')
import optiontools as opt
sys.path.append('../plotting')
import plotloss as pl

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputfile', vtype='path') )
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
    options.append( opt.Option('sort', vtype='bool', default=False) )
    options.append( opt.Option('metric', default='val_loss') )
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
    labels = []
    # set input files in case a file is given
    if( options.inputfile is not None): 
	infiles = [optons.inputfile]
	label = [options.metric]
    # set input files in case a directory is given
    elif( options.inputdir is not None ):
	jobdirs = [f for f in os.listdir(options.inputdir) if 'job' in f]
	for jobdir in jobdirs:
	    infile = os.path.join(options.inputdir,jobdir,'history.pkl')
	    if not os.path.exists(infile):
		print('WARNING: found directory {} in {},'.format(jobdir, options.inputdir)
			+' but it does not seem to contain a "history.pkl" file')
		continue
	    infiles.append(infile)
	    labels.append('{} for job {}'.format(options.metric,jobdir.replace('job','')))
    
    # load the losses
    losses = []
    for infile in infiles:
	with open(infile,'r') as f:
	    history = pkl.load(f)
	if options.metric in history:
	    loss = history[options.metric]
	else: raise Exception('ERROR: metric {} not in file {}'.format(options.metric,infile))
	if options.sort: loss.sort(reverse=True)
	losses.append( loss )

    # hard coded arguments
    if len(labels)>5: labels = None
    extratext = 'CMS Preliminary\n\n'
    if len(infiles)>1:
	extratext += 'Results for hyperparameter optimization\n'
	extratext += 'of a neural network with keras\n'
	if options.sort: extratext += '(sorted by {})'.format(options.metric)
    else:
	extratext += 'Results for neural network training\n'
        if options.sort: extratext += '(sorted by {})'.format(options.metric)
    extratext_coords = (0.1, 0.95)
 
    # make the figure
    (fig,ax) = pl.plotloss( losses, labellist=labels,
		colorsort=True,
		title=None,
                xlims=None, xaxtitle='iteration',
                yaxlog=False, yaxtitle=options.metric )
    ax.text(extratext_coords[0], extratext_coords[1], extratext,
            horizontalalignment='left', verticalalignment='top', transform=ax.transAxes) 
    fig.savefig(options.outputfile)
