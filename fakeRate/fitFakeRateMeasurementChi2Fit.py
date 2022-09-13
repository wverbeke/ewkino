###############################################################################
# use perform simple chi2 template fits to the data in order to get fake rate #
###############################################################################
# experimental stage!
# more simple alternative to fiFakeRateMeasurementTemplateFit 
# (which uses combine for a full maximum likelihood fit)

import sys
import os
import ROOT
import numpy as np
from copy import copy
from array import array
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
# import local tools
sys.path.append('../Tools/python')
import histtools as ht
sys.path.append('../plotting/python')
import histplotter as hp
import hist2dplotter as h2dp
sys.path.append('python')
import fakeRateMeasurementTools as frt


def calcchi2(obs, exp, std):
    ### calculate chi2 test statistic between two numpy arrays
    # input arguments:
    # - obs, exp and std: 1D numpy arrays with observed values, 
    #   expected values, and error estimates respectively;
    #   must all have the same length.
    # note: the return type is a tuple of the form (chi2, npoints),
    #       where npoints is the length of obs/exp minus the number of points 
    #       with zero error, as these points are not taken into account
    #       in the calculation.
    valid_inds = np.nonzero(std)[0]
    npoints = len(valid_inds)
    obs = obs[valid_inds]
    exp = exp[valid_inds]
    std = std[valid_inds]
    chi2 = np.sum(np.power(np.divide(obs-exp,std),2))
    return (chi2, npoints)


def chi2fit(obs, sig, sig_std, bkg=None, bkg_std=None, 
	    poirange=(0,3), nsteps=301, figname=None):
    ### perform a chi2 fit for a single signal strength POI
    # input arguments:
    # - obs: array of observed data
    # - sig: array of signal
    # - sig_std: array of (expected) error on signal
    # - bkg: array of background (defaults to zero-array)
    # - bkg_std: array of (expected) error on background (defaults to zero-array)
    # - poirange: scanning range for signal strength
    # - nsteps: scanning step for signal strength
    # - figname: name of figure to make (default: no plotting)
    poivals = np.linspace(poirange[0], poirange[1], num=nsteps)
    chi2vals = np.zeros(nsteps)
    if bkg is None: bkg = np.zeros(len(sig))
    if bkg_std is None: bkg_std = np.zeros(len(sig))
    # determine effective number of degrees of freedom (assume independent of poi)
    exp = bkg+sig
    std = np.sqrt(obs + np.power(sig_std,2) + np.power(bkg_std,2))
    ndof = calcchi2(obs, exp, std)[1] - 1
    # loop over allowed values of poi and calculate chi2
    for i in range(nsteps):
	# recalculate expectation
	exp = bkg + poivals[i]*sig
	# recalculate total error
	std = np.sqrt(obs + np.power(poivals[i]*sig_std,2) + np.power(bkg_std,2))
	# calculate chi2
	chi2vals[i] = calcchi2(obs, exp, std)[0]
    # find minimum
    argmin = np.argmin(chi2vals)
    minchi2 = chi2vals[argmin]
    bestpoi = poivals[argmin]
    # also find minimum in log scale (only for plotting)
    minlogchi2 = np.log(minchi2)
    # also find minimum for normalized chi2 (only for goodness-of-fit estimate)
    minnormchi2 = minchi2/ndof
    # check if minimum is pathologic
    if( argmin==0 or argmin==nsteps-1 ):
	print('WARNING in chi2fit: minimum chi2 value is at the edge of scanning range.')
    # determine errors by crossing with minchi2+1 contour
    stdinds = np.nonzero(chi2vals<=minchi2+1)[0]
    uperror = poivals[stdinds[-1]]-bestpoi
    downerror = bestpoi-poivals[stdinds[0]]
    # make a plot
    if figname is not None:
	fig,ax = plt.subplots()
	#ylims = (minchi2/2, minchi2*5) # for log scale
	ylims = (minchi2-0.5, minchi2+7.5) # for lin scale
	#xlims = (poivals[0], poivals[-1]) # full range
	xlims = (max(bestpoi-5*downerror,poivals[0]),min(bestpoi+5*uperror,poivals[-1])) # subrange
	ytextoffset = (ylims[1]-ylims[0])*0.03
	xtextoffset = (xlims[1]-xlims[0])*0.03
	# basic plot
	ax.plot(poivals, chi2vals, color='b', label='$\chi^2$ test statistic')
	# horizontal lines and labels
	ax.hlines(minchi2, xlims[0], xlims[1], colors='g')
	ax.text(xlims[0]+xtextoffset, minchi2, 'minimum', color='g')
	ax.hlines(minchi2+1, xlims[0], xlims[1], colors='r')
	ax.text(xlims[0]+xtextoffset, minchi2+1, '$\pm 1\sigma$', color='r')
	ax.hlines(minchi2+4, xlims[0], xlims[1], colors='k')
	ax.text(xlims[0]+xtextoffset, minchi2+4, '$\pm 2\sigma$')
	# vertical line and patch
	area = mpl.patches.Rectangle( (bestpoi-downerror,ylims[0]), 
		downerror+uperror, ylims[1]-ylims[0], color='r', alpha=0.2 )
	ax.add_patch(area)
	ax.vlines(bestpoi, ylims[0], ylims[1], colors='g', linestyles='dashed')
	# other properties
	ax.set_ylabel('$\chi^2$')
	#ax.set_yscale('log')
	ax.set_ylim(ylims)
	ax.set_xlabel('Signal strength')
	ax.set_xlim(xlims)
	# write fit result
	poitext = 'Fit result: r = {:.2f} + {:.2f} - {:.2f}'.format(
		    bestpoi, uperror, downerror)
	txt = ax.text(0.05, 0.95, poitext,
		horizontalalignment='left', verticalalignment='top',
		transform=ax.transAxes)
	txt.set_bbox(dict(facecolor='white', 
                          alpha=0.8, 
                          edgecolor='black'))
	# write goodness-of-fit
	goftext = 'Goodness of fit:\n'
	goftext += '$\chi^2$/ndof = {:.2f}\n'.format(minnormchi2)
	goftext += 'ndof = {}'.format(ndof)
	txt = ax.text(0.05, 0.85, goftext,
                horizontalalignment='left', verticalalignment='top',
		transform=ax.transAxes)
	txt.set_bbox(dict(facecolor='white',             
                          alpha=0.8,             
                          edgecolor='black'))
	ax.legend()
	figname = os.path.splitext(figname)[0]+'.png'
	fig.savefig(figname)
    return (bestpoi, uperror, downerror)
    
    
def readroothist(hist):
    ### read a root histogram into a numpy array
    nbins = hist.GetNbinsX()
    val = np.zeros(nbins)
    err = np.zeros(nbins)
    for i in range(1,nbins+1):
	val[i-1] = hist.GetBinContent(i)
	err[i-1] = hist.GetBinError(i)
    return (val,err)
    

if __name__=='__main__':
   
    sys.stderr.write('###starting###\n')
 
    if not len(sys.argv)==5:
	print('### ERROR ###: wrong number of command-line arguments')
	print('               use: python fitTemplates.py <var> <flavour> <year> <frmapdir>')
	sys.exit()
    
    # initialize arguments and global parameters
    var = sys.argv[1]
    flavour = sys.argv[2]
    year = sys.argv[3]
    frmapdir = sys.argv[4]
    instancename = '{}_{}_{}'.format(flavour, year, var)
    inputfile = os.path.abspath('fakeRateMeasurement_data_'+instancename+'_histograms.root')

    # check if input file exists
    if not os.path.exists(inputfile):
	raise Exception('ERROR: required input file {} does not exist.'.format(inputfile))
    # make a working directory to store intermediate results
    workingdir = 'fakeRateChi2Fits_{}'.format(instancename)
    if not os.path.exists(workingdir):
	os.makedirs(workingdir)

    # read pt and eta bins and ranges
    basehistname = 'data_numerator_'+var+'_'+year+'_'+flavour
    (ptbins,etabins) = frt.readptetabins(inputfile,basehistname)
    ptrange = copy(ptbins); ptrange.append(100)
    etarange = copy(etabins); etarange.append(2.5)
    if(flavour=='muon'): etarange[-1]=2.4

    # make 2D maps to hold number of nonprompt leptons in num and denom
    numyieldmap = ROOT.TH2D( "fake-rate", "fake-rate; p_{T} (GeV); |#eta|", 
		len(ptrange)-1, array('f',ptrange), 
		len(etarange)-1, array('f',etarange))
    numyieldmap.SetDirectory(0)
    numyieldmap.Sumw2()
    denomyieldmap = ROOT.TH2D( "", "", 
		len(ptrange)-1, array('f',ptrange), 
		len(etarange)-1, array('f',etarange))
    denomyieldmap.SetDirectory(0)
    denomyieldmap.Sumw2()
    # group the nummap and denommap in a dict for easier access
    npyieldmaps = {'denominator':denomyieldmap,'numerator':numyieldmap}

    # make 2D maps to hold the measured signal strengths in num and denom
    numrmap = ROOT.TH2D( "signal strength", "signal strength; p_{T} (GeV); |#eta|",
                len(ptrange)-1, array('f',ptrange),
                len(etarange)-1, array('f',etarange))
    numrmap.SetDirectory(0)
    numrmap.Sumw2()
    denomrmap = ROOT.TH2D( "signal strength", "signal strength; p_{T} (GeV); |#eta|",
                len(ptrange)-1, array('f',ptrange),
                len(etarange)-1, array('f',etarange))
    denomrmap.SetDirectory(0)
    denomrmap.Sumw2()
    # group the nummap and denommap in a dict for easier access
    rmaps = {'denominator':denomrmap,'numerator':numrmap}

    # move to workingdir
    cwd = os.getcwd()
    os.chdir(workingdir)

    # loop over pt and eta bins
    print('running on file {}'.format(inputfile))
    print('start looping over pt and eta bins...')
    for ptbinnb,ptbin in enumerate(ptbins):
	for etabinnb,etabin in enumerate(etabins):
	    #if( ptbinnb!=2 or etabinnb!=0 ): continue # for testing on small number of bins
	    ptbinstr = str(ptbin).replace('.','p')
	    etabinstr = str(etabin).replace('.','p')
	    print('   bin pt {}, eta {}'.format(ptbinstr,etabinstr))
	    for ftype in ['denominator','numerator']:
	
		### get the correct histograms and make a prefit plot
		thisbin = ftype+'_'+var+'_'+year+'_'+flavour+'_pT_'+ptbinstr+'_eta_'+etabinstr
		# get histograms
		histograms = frt.loadselectedhistograms(inputfile,
                                ftype, var, year, flavour, ptbin, etabin)
                datahist = histograms['datahist']
		prompthists = histograms['prompthists']
		nonprompthists = histograms['nonprompthists']	
		# add all prompt processes into one histogram
		prompthist = prompthists[0].Clone()
		prompthist.Reset()
		for hist in prompthists: prompthist.Add( hist )
		prompthist.SetName('total_prompt_'+thisbin)
		prompthist.SetTitle('Prompt')
		# do the same for nonprompt processes
		nonprompthist = nonprompthists[0].Clone()
		nonprompthist.Reset()
		for hist in nonprompthists: nonprompthist.Add( hist )
		nonprompthist.SetName('total_nonprompt_'+thisbin)
		nonprompthist.SetTitle('Nonprompt')

		# make a prefit plot
                xaxtitle = datahist.GetXaxis().GetTitle()
                yaxtitle = datahist.GetYaxis().GetTitle()
                lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
                            '2016PreVFP':19520, '2016PostVFP':16810, 
                            '2016Merged':36300 }
                lumi = lumimap[year]
                extracmstext = 'Preliminary'
                extrainfos = []
                extrainfos.append('{} {}'.format(year, flavour))
                extrainfos.append('pT: {}, eta: {}, {}'.format(ptbin, etabin, ftype))
                extrainfos.append('prefit')
                colormap = {}
                colormap['Prompt'] = ROOT.kAzure + 1
                colormap['Nonprompt'] = ROOT.kRed - 7
                figname = os.path.join(thisbin+'_prefit')
                hp.plotdatavsmc( figname, datahist,
                            [prompthist,nonprompthist],
                            datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
                            colormap=colormap,
                            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
                            extracmstext=extracmstext,
                            extrainfos=extrainfos, infosize=15 )
		# write the histograms to a temporary file
		tempfilename = thisbin+'_histograms.root'
		f = ROOT.TFile.Open(tempfilename,'recreate')
		prompthist.Write()
		nonprompthist.Write()
		datahist.Write()
		f.Close()
		prefitnp = nonprompthist.Integral()
		
		### run chi2 fit
		(obs, _) = readroothist( datahist )
		(sig, sig_std) = readroothist( nonprompthist )
		(bkg, bkg_std) = readroothist( prompthist )
		figname = thisbin+'_chi2profile'
		(r,uperror,downerror) = chi2fit(obs, sig, sig_std, bkg=bkg, bkg_std=bkg_std,
					    poirange=(0,5), nsteps=1001, figname=figname)
		print('measured signal strength: {} + {} - {}'.format(r,uperror,downerror))

		### process results
		# make post-fit distributions
		postfitprompthist = prompthist.Clone()
		postfitprompthist.SetDirectory(0)
		postfitnonprompthist = nonprompthist.Clone()
		postfitnonprompthist.SetDirectory(0)
		postfitnonprompthist.Scale(r)
		# make the postfit plot
                xaxtitle = datahist.GetXaxis().GetTitle()
                yaxtitle = datahist.GetYaxis().GetTitle()
                lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
                            '2016PreVFP':19520, '2016PostVFP':16810, 
                            '2016Merged':36300 }
                lumi = lumimap[year]
                extracmstext = 'Preliminary'
                extrainfos = []
                extrainfos.append('{} {}'.format(year, flavour))
                extrainfos.append('pT: {}, eta: {}, {}'.format(ptbin, etabin, ftype))
		extrainfos.append('postfit')
                colormap = {}
                colormap['Prompt'] = ROOT.kAzure + 1
                colormap['Nonprompt'] = ROOT.kRed - 7
                figname = os.path.join(thisbin+'_postfit')
                hp.plotdatavsmc( figname, datahist,
                            [postfitprompthist,postfitnonprompthist],
                            datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
                            colormap=colormap,
                            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
                            extracmstext=extracmstext,
                            extrainfos=extrainfos, infosize=15 )
		
		### fill the yield maps ###
		binindex = numyieldmap.FindBin(ptbin+1e-6,etabin+1e-6)
		# directly take integral of postfit distribution
		# (but still use signal strength measurement for relative error)
		postfitnp = postfitnonprompthist.Integral()
		npyieldmaps[ftype].SetBinContent(binindex,postfitnp)
		npyieldmaps[ftype].SetBinError(binindex,postfitnp*max(uperror,downerror)/r)
		rmaps[ftype].SetBinContent(binindex, r)
		rmaps[ftype].SetBinError(binindex, max(uperror,downerror))
		# print results
                print('initial amount of nonprompt leptons: {}'.format(prefitnp))
                print('measured signal strength: {} + {} - {}'.format(r,uperror,downerror))
                print('post-fit amount of nonprompt leptons: {}'.format(postfitnp))

    # save and plot the signal strength maps
    rmapfile = 'signalstrengths_'+instancename+'.root'
    f = ROOT.TFile.Open(rmapfile,'recreate')
    numrmap.Write("signalstrengths_numerator")
    denomrmap.Write("signalstrengths_denominator")
    f.Close()
    title = 'Signal strengths for {} {}s numerator'.format(year, flavour)
    h2dp.plot2dhistogram( numrmap, rmapfile.replace('.root','_numerator.pdf'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )
    h2dp.plot2dhistogram( numrmap, rmapfile.replace('.root','_numerator.png'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )
    title = 'Signal strengths for {} {}s denominator'.format(year, flavour)
    h2dp.plot2dhistogram( denomrmap, rmapfile.replace('.root','_denominator.pdf'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )
    h2dp.plot2dhistogram( denomrmap, rmapfile.replace('.root','_denominator.png'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )

    # move back to main directory
    os.chdir(cwd)

    ### make fake rate map
    frmap = numyieldmap.Clone()
    frmap.Divide(denomyieldmap)
    # save and plot fake rate map
    frmapfile = os.path.join(frmapdir,'fakeRateMap_data_'+instancename+'.root')
    f = ROOT.TFile.Open(frmapfile,'recreate')
    frmap.Write("fakeRate_" + flavour + "_" + year)
    f.Close()
    title = 'Fake rate map for {} {}s'.format(year, flavour)
    h2dp.plot2dhistogram( frmap, frmapfile.replace('.root','.pdf'), histtitle=title )
    h2dp.plot2dhistogram( frmap, frmapfile.replace('.root','.png'), histtitle=title )
    sys.stderr.write('###done###\n')
