########################################################
# plot the loss of an iterative optimization procedure #
########################################################

import sys
import matplotlib as mpl
import matplotlib.pyplot as plt

def plotloss( losses, labellist=None, 
		colorlist=None, colorsort=False,
                title=None,
                xlims=None, xaxtitle='iteration',
                yaxlog=False, yaxtitle='loss'):
    ### plot one or multiple arrays of loss values
    fig,ax = plt.subplots()
    dolegend = True
    if labellist is None:
        dolegend = False
        labellist = ['dummy']*len(losses)
    if colorlist is None:
	# get a colormap with the right amount of colors
        colormap = mpl.cm.get_cmap('jet', len(losses))
        colorlist = [mpl.colors.rgb2hex(colormap(i)) for i in range(colormap.N)]
	if colorsort:
	    # re-index the losses to sort according to last loss value
	    lastloss = [l[-1] for l in losses]
	    sorted_inds = [i[0] for i in sorted(enumerate(lastloss), key=lambda x:x[1])]
	    losses = [losses[i] for i in sorted_inds]
	    # revert losses and colorlist to plot lowest loss last
	    losses = losses[::-1]
	    colorlist = colorlist[::-1]
    for loss, label, color in zip(losses, labellist, colorlist):
        ax.plot(loss, color=color,
                linewidth=2, label=label)
    if dolegend: ax.legend()
    if yaxlog: ax.set_yscale('log')
    if xlims is not None: ax.set_xlim(xlims)
    if title is not None: ax.set_title(title)
    if xaxtitle is not None: ax.set_xlabel(xaxtitle)
    if yaxtitle is not None: ax.set_ylabel(yaxtitle)
    #plt.show(block=False)
    return (fig,ax)
