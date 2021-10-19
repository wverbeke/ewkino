#############################################
# run a goodness-of-fit test on a workspace #
#############################################

import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import ROOT
import combinetools as cbt
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

def integral( binvalues, binedges, lowerbound=None, upperbound=None ):
    ### compute the integral of a histogram
    # check dimensions
    if len(binedges)!=len(binvalues)+1:
	raise Exception('ERROR in integral: binvalues and binedges are incompatible')
    # set bounds if not given
    if lowerbound is None: lowerbound = binedges[0]-1
    if upperbound is None: upperbound = binedges[-1]+1
    # loop over bins and calculate integral
    integral = 0
    for binnb in range(len(binvalues)):
	if binedges[binnb]<lowerbound: continue
	if binedges[binnb+1]>upperbound: continue
	binwidth = binedges[binnb+1]-binedges[binnb]
	integral += binvalues[binnb]*binwidth
    return integral

def makepvalplot( binvalues, binedges, tdata, pval=None, title=None, 
		    yaxtitle='probability density', xaxtitle='test statistic', 
		    extrainfos=[],
		    figname='goodnessoffit.png' ):
    ### make a plot of the distribution of test statistics for toys and the test statistic in data
    fig,ax = plt.subplots()
    ymax = np.max(binvalues)
    binwidths = binedges[1:] - binedges[:-1]
    ax.bar( binedges[:-1], binvalues, width=binwidths, color='c', label='toys' )
    ax.axvline( x=tdata, color='red', label='data' )
    ax.set_ylim((0,ymax*1.2))
    ax.set_xlabel(xaxtitle)
    ax.set_ylabel(yaxtitle)
    ax.legend(loc='upper right')
    if title is not None: ax.set_title(title)
    if pval is not None: 
	ax.text(0.65,0.7,'p-value: {}'.format(pval),transform=ax.transAxes)
    for i,info in enumerate(extrainfos):
	ax.text(0.65,0.65-i*0.05,info,transform=ax.transAxes)
    fig.savefig(figname)

if __name__=='__main__':

    # parse command line arguments
    if len(sys.argv)==2:
	datacardpath = os.path.abspath(sys.argv[1])
    # other arguments
    runlocal = False
    runqsub = True
    runcondor = False
    rungofcommands = True
    runplot = False # always locally, no job submission
    ntoys = 500 # ignored if rungofcommands is False

    # further initializations
    (datacarddir,card) = os.path.split(datacardpath)
    name = card.replace('.txt','')

    if rungofcommands:
	# execute goodness-of-fit commands
	commands = cbt.get_gof_commands( datacarddir, card, ntoys=ntoys )
	if( runcondor ):
                ct.submitCommandsAsCondorJob( 'cjob_goodnessoffit', commands )
        else:
            script_name = 'qsub_runcombine.sh'
            with open( script_name, 'w' ) as script:
                initializeJobScript( script )
                for c in commands: script.write(c+'\n')
            if runlocal:
                os.system('bash {}'.format(script_name))
            elif runqsub:
                submitQsubJob( script_name )

    if runplot:

	# some intializations
	nametotitle = ({
			'dc_combined_2016':'test statistic distribution for 2016 fit',
			'dc_combined_2017':'test statistic distribution for 2017 fit',
			'dc_combined_2018':'test statistic distribution for 2018 fit',
			})
	# read output test statistic for data
	datafile = os.path.join(datacarddir,
		    'higgsCombine{}.GoodnessOfFit.mH120.root'.format(name))
	f = ROOT.TFile.Open(datafile)
	tree = f.Get('limit')
	tree.GetEntry(0)
	tdata = getattr(tree,'limit')
	print('found data test statistic: {}'.format(tdata))

	# read output test statistics for toys
	toyfile = os.path.join(datacarddir,
		    'higgsCombine{}.GoodnessOfFit.mH120.123456.root'.format(name))
	f = ROOT.TFile.Open(toyfile)
	tree = f.Get('limit')
	ttoys = []
	for i in range(tree.GetEntries()):
	    tree.GetEntry(i)
	    ttoys.append(getattr(tree,'limit'))
	print('found {} toy test statistic'.format(len(ttoys)))

	# compute p-value
	(thist,binedges) = np.histogram( ttoys, bins=30, density=True )
	pval = integral( thist, binedges, lowerbound=tdata )
	print('calculated p value: {}'.format(pval))
	extrainfos = []
	extrainfos.append('algo: {}'.format('saturated model')) # possible to extract this?
	extrainfos.append('number of toys: {}'.format(1000))
	makepvalplot( thist, binedges, tdata, pval=pval, title=nametotitle[name],
			extrainfos=extrainfos )

