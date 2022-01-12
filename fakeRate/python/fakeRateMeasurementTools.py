###################################
# tools for fake rate measurement #
###################################

import sys
import os
sys.path.append('../../Tools/python')
import histtools as ht

def readptetabins(inputfile, basename):
    ### read pt and eta bins from a root file containing fake rate histograms
    # input arguments:
    # - inputfile: name of root file containing appropriate histograms
    # - basename: part of the histogram name that comes before the pT and eta definition
    # note: the names of the histograms are supposed to be of the form:
    #	    basename_pt_<pt>_eta_<eta>
    # note: factorized binning is assumed, so separate pt and eta bin arrays
    ptbins = []
    etabins = []
    histlist = ht.loadallhistograms(inputfile)
    for hist in histlist:
	if not basename in hist.GetName(): continue
	ptetaname = hist.GetName().split(basename)[-1].strip('_')
	pt = float(ptetaname.split('_')[1].replace('p','.'))
	if pt.is_integer(): pt = int(pt)
	eta = float(ptetaname.split('_')[3].replace('p','.'))
	if eta.is_integer(): eta = int(eta)
	if not pt in ptbins: ptbins.append(pt)
	if not eta in etabins: etabins.append(eta)
    return (sorted(ptbins),sorted(etabins))

def loadselectedhistograms(inputfile, ftype, variable, year, flavour, ptbin, etabin):
    ### load and select histograms
    # input arguments:
    # - inputfile: name of a root file containing appropriate histograms
    # - ftype: either "numerator" or "denominator"
    # - variable: name of the x-axis variable, usually "mT"
    # - year, flavour: strings representing year and lepton flavour
    # - ptbin, etabin: floats representing bin low edges
    # note: histograms are assumed to be named as follows:
    #       process_ptype_ftype_var_year_flavour_pT_ptvalue_eta_etavalue
    #       where:
    #       - process is a process tag (e.g. "DY" or "VV" or "data")
    #       - ptype is either "prompt" or "nonprompt" (absent for data!)
    ptbinstr = str(ptbin).replace('.','p')
    etabinstr = str(etabin).replace('.','p')
    thisbin = '{}_{}_{}_{}_pT_{}_eta_{}'.format(ftype, variable, year, flavour, ptbinstr, etabinstr)
    # load and select histograms
    histlist = ht.loadallhistograms(inputfile)
    newhistlist = []
    for hist in histlist:
        if( hist.GetName().split('_pT_')[-1]==ptbinstr+'_eta_'+etabinstr
            and ftype in hist.GetName() ):
            newhistlist.append(hist)
    histlist = newhistlist
    # get data histogram
    datahists = ht.selecthistograms(histlist, mustcontainall=['data'])[1]
    if len(datahists)!=1:
        raise Exception('ERROR: found {} data histograms'.format(len(datahists))
                        +' while 1 is expected.')
    datahist = datahists[0]
    # get prompt histograms
    prompthists = ht.selecthistograms(histlist, mustcontainall=['_prompt'])[1]
    # get nonprompt histograms
    nonprompthists = ht.selecthistograms(histlist, mustcontainall=['_nonprompt'])[1]
    # set histogram titles
    for hist in prompthists:
        process = hist.GetName().split('_')[0]
        hist.SetTitle(process)
    for hist in nonprompthists:
        process = hist.GetName().split('_')[0]
        hist.SetTitle(process)
    # remove empty QCD histogram from prompt histogram list (see fill method)
    prompthists = ht.selecthistograms(prompthists, maynotcontainone=['QCD'])[1]
    # add all nonprompt contributions except QCD into one
    qcdhist = nonprompthists[0].Clone()
    qcdhist.Reset()
    nonprompthist = nonprompthists[0].Clone()
    nonprompthist.Reset()
    nonprompthist.SetTitle('Other nonprompt')
    for hist in nonprompthists:
	process = hist.GetName().split('_')[0]
	if process=='QCD': qcdhist = hist
	else: nonprompthist.Add( hist )
    nonprompthists = [qcdhist, nonprompthist]
    return {'datahist':datahist, 'prompthists':prompthists, 'nonprompthists':nonprompthists}
