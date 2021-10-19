#############################################################################
# some tools for dealing with specific uncertainties to go in the datacards #
#############################################################################
# note: most of the functions here are analysis- or naming dependent
#       and are not intended to be generalizable out of the box

import sys
import os
import re
import math

def get_lumi_unc(processes,year,idstr='defaultrun2'):
    ### define conventional sets of luminosity uncertainty, to avoid typing every time
    # input arguments:
    # - processes: list of process names
    # - year: string representing the data-taking year
    # - idstr: string identifying what uncertainty (e.g. correlations etc) to return
    # output:
    # dict mapping lumi uncertainty names to dicts mapping processes to impacts

    # define correlation pattern and magnitude
    if( idstr=='defaultrun2' ):
        if year=='2016': unc = {'lumi':1.009, # correlated
                                'lumi_2016': 1.022, # uncorrelated
                                'lumi_20162017': 1.008} # partially correlated
        elif year=='2017': unc = {'lumi':1.008,
                                  'lumi_2017':1.02,
                                  'lumi_20161017':1.006,
                                  'lumi_20172018':1.004}
        elif year=='2018': unc = {'lumi':1.02,
                                  'lumi_2018':1.015,
                                  'lumi_20172018':1.003}
    elif( idstr=='updatedrun2' ):
	if year=='2016': unc = {'lumi':1.006,
				'lumi_2016':1.01}
	elif year=='2017': unc = {'lumi':1.009,
				  'lumi_2017':1.02,
				  'lumi_20172018':1.006}
	elif year=='2018': unc = {'lumi':1.02,
				  'lumi_2018':1.015,
				  'lumi_20172018':1.002}
    else:
        raise Exception('ERROR in get_lumi_unc: idstr {} not recognized'.format(idstr))
    # make a dict mapping each source to their impacts on processes
    lumi_unc = {}
    for source,mag in unc.items():
        lumi_unc[source] = {}
        for p in processes: lumi_unc[source][p] = mag
    # return result
    return lumi_unc

def subselect_systematics( systematics ):
    ### define which systematics to consider and which to ignore
    # input arguments:
    # - systematics: list of names of systematics
    # output: a similar list with some elements removed
    
    # choose which processes do not need normalization uncertainties
    pnonorm = ['tZq','WZ','ZZH','Xgamma','tbartZ','ttZ','Xgamma_int','Xgamma_ext'] 
    # (tbartZ and ttZ for old and new naming convention)
    # (Xgamma_int and Xgamma_ext for split Xgamma internal/external conversions)

    removesystematics = []

    # choose between separate renormalization and factorization scale variations,
    # correlated variations, or their envelope
    removesystematics.append('rScale.+')
    removesystematics.append('fScale.+')
    removesystematics.append('rfScales.+')
    #removesystematics.append('qcdScalesShapeEnv.+')
    # remove norm uncertainties for all of them
    for p in pnonorm:
	for sys in ['rScale','fScale','rfScales','qcdScales']:
	    removesystematics.append(sys+'Norm_'+p)

    # choose between pdf envelope or rms
    removesystematics.append('pdfShapeEnv')
    #removesystematics.append('_pdfShapeRMS')
    # (warning: do not remove pdfNorm here, as it needs to be taken into account for some processes)
    # (need to disable it for specific processes in calling script)

    # remove the norm uncertainty related to ISR/FSR
    for p in pnonorm:
	for sys in ['isr','fsr']:
	    removesystematics.append(sys+'Norm_'+p)

    # choose between total JEC, grouped sources or individual sources
    #removesystematics.append('JEC')
    # update: signalregion cat2 2017 seems to have postfit plot issues with JECAll variations,
    #         so keep single JEC variation but disable it where needed (see disable_default)
    removesystematics.append('JECGrouped.+')
    removesystematics.append('JECGrouped_Total')
    #removesystematics.append('JECAll.+')
    # note: still to add here which ones should be removed if using JECAll!
    #       with new way of processing, only the correct ones make it up to this stage,
    #       so no need for additional removing.

    # remove individual variations (if not done so before)
    removesystematics.append('qcdScalesShapeVar.+')
    removesystematics.append('pdfShapeVar.+')

    newsystlist = []
    for sys in systematics:
	keep = True
        for rmtag in removesystematics:
	    if '.+' in rmtag:
		if re.match(rmtag,sys): 
		    keep = False
		    break
	    else:
		if rmtag==sys:
		    keep = False
		    break
	#print(sys+' -> '+str(keep))
	if keep: newsystlist.append(sys)
    return newsystlist

def disable_default( processinfo, year, region, npfromdata ):
    ### disable some systematics for some processes in a ProcessInfoCollection object

    # disable some systematics for nonprompt
    if npfromdata:
	for sys in processinfo.slist:
	    if 'JECGrouped' in sys: processinfo.disablesys( sys, ['nonprompt'] )
	    if 'JECAll' in sys: processinfo.disablesys( sys, ['nonprompt'] )
	    if sys=='CR_QCD': processinfo.disablesys( sys, ['nonprompt'] )
	    if sys=='CR_GluonMove': processinfo.disablesys( sys, ['nonprompt'] )
	    if sys=='UE': processinfo.disablesys( sys, ['nonprompt'] )
	    # (maybe still to be extended? check!)

    # disable some systematics for normed processes
    # (note: also check subselect_systematics!)
    processinfo.disablesys( 'pdfNorm', (['tZq','WZ','ZZH',
					'ttZ','nonprompt',
					#'Xgamma',
					'Xgamma_int','Xgamma_ext'
					] ) )

    # disable pileup for 2017 (problem in histograms for tZq sample)
    # now dealt with in runsystematics by ignoring events with nominal weight 0.
    # but similar issue it seems in tX process, maybe later run over all samples with fix
    if year=='2017': processinfo.disablesys( 'pileup', ['tX'] )
    if year=='2017' and region=='wzcontrolregion': processinfo.disablesys( 'pileup', ['nonprompt'])
    
    # disable split JEC uncertainties in 2017 signalregion 2
    # only need to disable for postfit plotting, fit itself does not seem to have problems
    # issue not yet fully understood
    # for now, replace by single JEC variation to see if that helps -> yes
    dosinglejec = True # set to false for fit, true for postfit plots
    if 'JEC' in processinfo.slist: processinfo.disablesys( 'JEC', processinfo.plist )
    if dosinglejec:
	if( 
	    (year=='2017' and region=='signalregion_cat2')  
	    ): 
	    changelist = ['WZ','ttZ','ZZH','tX','multiboson','Xgamma']
	    for sys in processinfo.slist:
		if 'JECAll' in sys: processinfo.disablesys( sys, changelist )
	    processinfo.enablesys( 'JEC', changelist, 1 )

    # disable all systematics for WZ contribution to 2016 ZZ region
    # only need to disable for postfit plotting, fit itself does not seem to have problems (?)
    # issue not yet fully understood
    # contribution is anyway quasi 0 (see e.g. prefit plots)
    dodisablewzsys = True # set to false for the fit, true for postfit plots
    if dodisablewzsys:
	if(
	    (year=='2016' and region=='zzcontrolregion')
	    ):
	    for sys in processinfo.slist:
		processinfo.disablesys( sys, ['WZ'] ) 

def addnormsys_default( processinfo, year, region, npfromdata ):
    ### add default normalization uncertainties to a ProcessInfoCollection object
        
    # add normalization uncertainties
    normsyslist = []
    # - add luminosity uncertainty
    uncs_lumi = get_lumi_unc( processinfo.plist, year, idstr='updatedrun2' )
    for unc,impacts in uncs_lumi.items():
        processinfo.addnormsys( unc, impacts )
        if npfromdata: processinfo.disablesys( unc, ['nonprompt'] )
        normsyslist.append(unc)
    # - add trigger uncertainty
    trname = 'trigger_'+year
    trimpacts = {}
    for p in processinfo.plist: trimpacts[p] = 1.02
    processinfo.addnormsys( trname, trimpacts )
    if npfromdata: processinfo.disablesys( trname, ['nonprompt'] )
    normsyslist.append(trname)
    # - add individual normalization uncertainties
    uncs_norm = ({'WZ':1.1,'ZZH':1.1,'ttZ':1.15,'nonprompt':1.3,
		    #'Xgamma':1.1,
		    'Xgamma_int':1.1,'Xgamma_ext':1.1
		})
    # (full norm unc for all controlled processes)
    #uncs_norm = {'tbartZ':1.15,'Xgamma':1.1,'nonprompt':1.3}
    # (alternative smaller set for when using rateparams for WZ and ZZ)
    for normp,mag in uncs_norm.items():
        nname = 'norm_'+normp
        nimpacts = {}
        for p in processinfo.plist: nimpacts[p]='-'
        nimpacts[normp] = mag # will crash if normp is not in recognized processes
        processinfo.addnormsys( nname, nimpacts )
        normsyslist.append(nname)
    # - add individual extrapolation uncertainties
    #   note: contrary to general normalization uncertainties, 
    #   these are only applied in specific regions!
    uncs_extrapol = {}
    if 'signalregion_cat1' in region:
        uncs_extrapol = {'WZ':1.04}
    elif 'signalregion_cat2' in region:
        uncs_extrapol = {'WZ':1.05}
    elif 'signalregion_cat3' in region:
        uncs_extrapol = {'WZ':1.13}
    for normp,mag in uncs_extrapol.items():
        nname = 'extrapol_'+normp
        nimpacts = {}
        for p in processinfo.plist: nimpacts[p]='-'
        nimpacts[normp] = mag # will crash if normp is not in recognized processes
        processinfo.addnormsys( nname, nimpacts )
        normsyslist.append(nname)

    return normsyslist
