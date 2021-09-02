############################################
# make plots comparing kinematic variables #
############################################
import os
import sys
sys.path.append('../tools')
import histtools as ht
sys.path.append('../plotting')
import multihistplotter

if __name__=='__main__':

    # parse command line arguments
    inputfile = sys.argv[1]

    # check if input file is .root file or a directory
    # (if a directory, run on all files in it)
    inputfiles = []
    if inputfile[-5:]=='.root':
	inputfiles = [inputfile]
    else:
	inputfiles = [os.path.join(inputfile,f) for f in os.listdir(inputfile) if f[-5:]=='.root']
    
    # other initializations
    categories = ['ZLepGamma','other']
    variables = ([
		    "_abs_eta_recoil",
		    "_Mjj_max",
		    "_lW_asymmetry",
		    "_deepCSV_max",
		    "_deepFlavor_max",
		    "_lT",
		    "_MT",
		    "_pTjj_max",
		    "_dRlb_min",
		    "_dPhill_max",
		    "_HT",
		    "_nJets",
		    "_nBJets",
		    "_dRlWrecoil",
		    "_dRlWbtagged",
		    "_M3l",
		    "_abs_eta_max",
		    "_nMuons",
		    "_nElectrons",
		    "_yield",
		    "_leptonPtLeading",
		    "_leptonPtSubLeading",
		    "_leptonPtTrailing",
		    "_leptonEtaLeading",
		    "_leptonEtaSubLeading",
		    "_leptonEtaTrailing",
		    "_jetPtLeading",
		    "_jetPtSubLeading",
		    "_numberOfVertices",
		    "_bestZMass",
		    "_lW_pt",
		    "_Z_pt"
 
		])

    # loop over input files
    for inputfile in inputfiles:

	# make a directory to put the figures in
	outputdir = inputfile.replace('.root','_figures')
	if os.path.exists(outputdir):
	    os.system('rm -r {}'.format(outputdir))
	os.makedirs(outputdir)

	hists = ht.loadallhistograms(inputfile)
	for variable in variables:
	    print('plotting file {}, variable {}'.format(inputfile,variable))
	    thishists = []
	    for cat in categories:
		thishist = ht.selecthistograms( hists, mustcontainall=[variable,cat] )[1]
		if len(thishist)!=1: 
		    print('found ambiguous histograms')
		    break
		thishists.append(thishist[0])
	
	    outputfile = os.path.join(outputdir,variable.strip('_')+'.png')
	    try:
		multihistplotter.plothistograms( thishists, 
					 outputfile,
					 xaxtitle=variable, 
					 yaxtitle='Normalized number of events',
					 normalize=True,
					 labellist=categories,
					 errorbars=True
					 )
	    except:
		print('WARNING: plotting failed, trying again without normalization')
		multihistplotter.plothistograms( thishists,
                                         outputfile,
                                         xaxtitle=variable,
                                         yaxtitle='Normalized number of events',
                                         normalize=False,
                                         labellist=categories,
                                         errorbars=True
                                         )

