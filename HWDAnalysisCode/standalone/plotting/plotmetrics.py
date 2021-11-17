#############################################################################
# plot some metrics of a classifier algorithm with continuous output scores #
#############################################################################

import sys
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt

def plot_stosqrtn( wprange, stosqrtn, label=None, color=None,
		    sig_eff=None, sig_label=None, sig_color=None,
		    bck_eff=None, bck_label=None, bck_color=None,
		    title=None,
		    xaxtitle='working point',
		    yaxlog=False, ymaxfactor=1.3, yaxtitle='metric' ):
    ### plot a metric based on signal and background efficiencies.
    # originally meant for S/sqrt(S+B), but other similar metrics can be plotted as well.
    # along with the metric, the actual signal and background efficiencies can be plotted as well.
    fig,ax = plt.subplots()
    ax2 = ax.twinx()
    # parse arguments
    if label is None: label = ''
    if sig_label is None: sig_label = ''
    if bck_label is None: bck_label = ''
    if color is None: color = 'blue'
    if sig_color is None: sig_color = 'forestgreen'
    if bck_color is None: bck_color = 'firebrick'
    # make the plots
    ax.plot( wprange, stosqrtn, label=label, color=color, linewidth=3 )
    if sig_eff is not None: ax2.plot( wprange, sig_eff, label=sig_label, 
					color=sig_color, linewidth=2 )
    if bck_eff is not None: ax2.plot( wprange, bck_eff, label=bck_label, 
					color=bck_color, linewidth=2 )
    # draw a dashed line at unit efficiency
    ax2.plot( [wprange[0],wprange[1]], [1.,1.], color='black', linestyle='dashed')
    ax2.grid()
    # set the legends
    ax.legend(loc='upper left')
    ax2.legend(loc='upper right')
    # axis properties for first axes
    if yaxlog: ax.set_yscale('log')
    ymin,ymax = ax.get_ylim()
    ax.set_ylim( (ymin, ymax*ymaxfactor) )
    if title is not None: ax.set_title(title)
    if xaxtitle is not None: ax.set_xlabel(xaxtitle)
    if yaxtitle is not None: ax.set_ylabel(yaxtitle, color=color)
    # axis properties for second axes
    ax2.set_ylabel('efficiency')
    ymin,ymax = ax2.get_ylim()
    ax2.set_ylim( (ymin, ymax*ymaxfactor) )
    return (fig,ax)

def plot_roc( sig_eff, bck_eff, label=None, color=None,
		bck_label='background efficiency', bck_log=False,
		bck_lims=(0.,1.),
		sig_label='signal efficiency', sig_log=False,
		sig_lims=(0.,1.),
		plotstyle='scatter' ):
    ### plot a roc curve
    fig,ax = plt.subplots()
    if label is None: label = ''
    if color is None: color = 'blue'
    if plotstyle=='scatter':
	ax.scatter(bck_eff, sig_eff, label=label, color=color)
    elif plotstyle=='plot':
	ax.plot(bck_eff, sig_eff, label=label, color=color, linewidth=2)
    ax.legend(loc='lower right')
    ax.set_xlabel(bck_label)
    ax.set_ylabel(sig_label)
    if bck_log: ax.set_xscale('log')
    if sig_log: ax.set_yscale('log')
    # set x axis limits
    if bck_lims=='auto': 
	bck_lims = ( np.amin(np.where(bck_eff>0.,bck_eff,1.))/2., 1. )
    ax.set_xlim(bck_lims)
    # set y axis limits
    if sig_lims=='auto':
	sig_min = np.amin(np.where((sig_eff>0.) & (bck_eff>0.),sig_eff,1.))
	sig_min = 2*sig_min-1.
	sig_max = 1+(1-sig_min)/5
	sig_lims = (sig_min,sig_max)
    ax.grid()
    return (fig,ax)
