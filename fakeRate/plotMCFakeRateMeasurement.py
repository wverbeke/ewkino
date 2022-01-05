####################################################
# a python looper for plotMCFakeRateMeasurement.cc #
####################################################

import os
import sys
import ROOT
sys.path.append('../plotting/python')
import multihistplotter as mhp

# set global properties
years = ['2016','2017','2018']
flavours = ['muon','electron']
plotflavourpt = True
# (set to True to make extra plots of lepton pt, split for light and heavy fakes)

# check if executable exists
if not os.path.exists('./plotMCFakeRateMeasurement'):
    print('### ERROR ###: executable does not seem to exist...')
    print('               run make -f makeMCPlotFakeRateMeasurement first.')
    sys.exit()

# hadd files if needed
# note that the subfiles are assumed to be in a folder named MCFakeRateMeasurementSubFiles
# you may have to manually create that folder and move the subfiles in there
for year in years:
    for flavour in flavours:
	filename = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
	if os.path.exists(filename): continue
	subfolder = 'MCFakeRateMeasurementSubFiles'
	cmd = 'hadd '+filename+' '+subfolder
	cmd += '/fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms_sample*.root'
	#print(cmd)
	os.system(cmd)

# loop over years and flavours
cwd = os.getcwd()
for year in years:
    for flavour in flavours:
	# check file
	filename  = 'fakeRateMeasurement_MC_'+flavour+'_'+year+'_histograms.root'
	if not os.path.exists(filename):
	    print('### ERROR ###: file '+filename+' not found, skipping it.')
	    continue
	# make fake rate map
        command = './plotMCFakeRateMeasurement {} {}'.format(flavour,year)
        os.system(command)
	if not plotflavourpt: continue
	# read additional 1D histograms
	f = ROOT.TFile.Open(filename)
	instancename = '{}_{}'.format(flavour,year)
	bflavor_denominator = f.Get('fakeRate_denominator_bflavor_{}'.format(instancename))
	bflavor_numerator = f.Get('fakeRate_numerator_bflavor_{}'.format(instancename))
	cflavor_denominator = f.Get('fakeRate_denominator_cflavor_{}'.format(instancename))
        cflavor_numerator = f.Get('fakeRate_numerator_cflavor_{}'.format(instancename))
	light_denominator = f.Get('fakeRate_denominator_lightflavor_{}'.format(instancename))
	light_numerator = f.Get('fakeRate_numerator_lightflavor_{}'.format(instancename))
	for hist in [bflavor_denominator,bflavor_numerator,
		    cflavor_denominator,cflavor_numerator,
		    light_denominator,light_numerator]:
	    hist.SetDirectory(0)
	# common settings for all plots
	outputdir = 'fakeRatePlots'
	if not os.path.exists(outputdir):
	    os.makedirs(outputdir)
	yaxtitle = 'Number of {}s'.format(flavour)
	xaxtitle = 'Lepton p_{T} (GeV)'
	# make a plot of light and heavy fakes passing tight
	histlist = [light_numerator,cflavor_numerator,bflavor_numerator]
	labellist = ['Tight {}s with light origing'.format(flavour),
			'Tight {}s with c-hadron origin'.format(flavour),
			'Tight {}s with b-hadron origin'.format(flavour)]
	figname = 'fakeFlavour_numerator_{}_lin'.format(instancename)
	figname = os.path.join(outputdir,figname)
	mhp.plotmultihistograms(histlist, 
				figname=figname,
				dolegend=True, labellist=labellist,
				xaxtitle=xaxtitle, yaxtitle=yaxtitle,
				logy=False, 
				drawoptions='hist e')
	# same but log scale
	figname = 'fakeFlavour_numerator_{}_log'.format(instancename)
        figname = os.path.join(outputdir,figname)
        mhp.plotmultihistograms(histlist,                         
                                figname=figname,
                                dolegend=True, labellist=labellist,
                                xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                                logy=True, 
                                drawoptions='hist e')
	# make a plot of light and heavy fakes passing FO
        histlist = [light_denominator,cflavor_denominator,bflavor_denominator]
        labellist = ['FO {}s with light origing'.format(flavour),
                        'FO {}s with c-flavor origin'.format(flavour),
			'FO {}s with b-flavor origin'.format(flavour)]
        figname = 'fakeFlavour_denominator_{}_lin'.format(instancename)
        figname = os.path.join(outputdir,figname)
        mhp.plotmultihistograms(histlist,                         
                                figname=figname,
                                dolegend=True, labellist=labellist,
                                xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                                logy=False, 
                                drawoptions='hist e')
	# same but log scale
        figname = 'fakeFlavour_denominator_{}_log'.format(instancename)
        figname = os.path.join(outputdir,figname)
        mhp.plotmultihistograms(histlist,
                                figname=figname,
                                dolegend=True, labellist=labellist,
                                xaxtitle=xaxtitle, yaxtitle=yaxtitle,
                                logy=True, 
                                drawoptions='hist e')
