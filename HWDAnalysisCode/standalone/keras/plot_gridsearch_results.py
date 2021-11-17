##############################################
# plot the results from a gridsearch process #
##############################################

import sys
import os
import pickle as pkl
import matplotlib.pyplot as plt
sys.path.append('../tools')
import optiontools as opt

def plotres( reslist, 
		title=None,
                xlims=None, xaxtitle='iteration',
                ylims=None, yaxlog=False, yaxtitle='loss',
		markersize=10 ): 
    fig,ax = plt.subplots()
    xax = list(range(len(reslist)))
    ax.scatter(xax, reslist, s=markersize, c=reslist, cmap='jet', edgecolors='none')
    if ylims is not None: ax.set_ylim(ylims)  
    if yaxlog: ax.set_yscale('log')
    if xlims is not None: ax.set_xlim(xlims)
    if title is not None: ax.set_title(title)
    if xaxtitle is not None: ax.set_xlabel(xaxtitle)
    if yaxtitle is not None: ax.set_ylabel(yaxtitle)
    return (fig,ax)


if __name__=='__main__':

    options = []
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
    options.append( opt.Option('sort', vtype='bool', default=False) )
    options.append( opt.Option('metric', default='val_loss') )
    options.append( opt.Option('yaxlog', vtype='bool', default=False) )
    options.append( opt.Option('markersize', vtype='int', default=10) )
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
    if options.sort: losses.sort(reverse=True)

    # hard coded arguments
    ylims = None
    if not options.yaxlog: ylims = (0., max(losses)*1.5)
    extratext = 'CMS Preliminary\n\n'
    extratext += 'Results for hyperparameter optimization\n'
    extratext += 'of a neural network with keras\n'
    if options.sort: extratext += '(sorted by {})'.format(options.metric)
    extratext_coords = (0.05, 0.95)

    # make the figure
    (fig,ax) = plotres( losses,
		title=None,
                xlims=(-1, len(losses)), xaxtitle='iteration',
                ylims=ylims, yaxlog=options.yaxlog, yaxtitle=options.metric,
		markersize=options.markersize )
    ax.text(extratext_coords[0], extratext_coords[1], extratext,
	    horizontalalignment='left', verticalalignment='top', transform=ax.transAxes)
    fig.savefig(options.outputfile)
