###################################################################
# merge files containing root histograms that are not overlapping #
###################################################################
# e.g. one file containing scale factors for pt < 20 and another for pt > 20

import ROOT

def merge2dhistograms(hist1, hist2):
    # both input arguments are expected to be TH2D
    # the hist at lower values (x or y axis) is expected to be the first argument

    # part 1: get bin edges
    hist1xbins = hist1.GetXaxis()
    hist1ybins = hist1.GetYaxis()
    hist2xbins = hist2.GetXaxis()
    hist2ybins = hist2.GetYaxis()

    # part 2: find how to merge the histograms
    hist1minx = hist1xbins.GetBinLowEdge(1)
    hist1maxx = hist1xbins.GetBinUpEdge(hist1xbins.GetNbins())
    hist1miny = hist1ybins.GetBinLowEdge(1)
    hist1maxy = hist1ybins.GetBinUpEdge(hist1ybins.GetNbins())
    hist2minx = hist2xbins.GetBinLowEdge(1)
    hist2maxx = hist2xbins.GetBinUpEdge(hist2xbins.GetNbins())
    hist2miny = hist2ybins.GetBinLowEdge(1)
    hist2maxy = hist2ybins.GetBinUpEdge(hist2ybins.GetNbins())
    mergemode = ''
    if( hist1maxx==hist2minx and 
	list(hist1ybins.GetXbins())==list(hist2ybins.GetXbins())):
	mergemode = 'xaxis'
    elif( hist1maxy==hist2miny and 
        list(hist1xbins.GetXbins())==list(hist2xbins.GetXbins())):
	mergemode = 'yaxis'
    else:
	print('ERROR: incompatible histograms found.')
	return
    
    # part 3: merge histograms
    if mergemode=='xaxis':
	xax = list(hist1xbins.GetXbins())+list(hist2xbins.GetXbins())[1:]
	yax = list(hist1ybins.GetXbins())
    if mergemode=='xaxis':
        xax = list(hist1xbins.GetXbins())
        yax = list(hist1ybins.GetXbins())+list(hist2ybins.GetYbins())[1:]
    ### not finished since turned out to be useless for the case where I wanted to apply it


if(__name__=='__main__'):

    eleRecoSFFile_pTBelow20 = ROOT.TFile.Open( "../../weights/"
            + "weightFiles/leptonSF/egamma_recoEff_2016_pTBelow20.root" );
    electronRecoSFHist_pTBelow20 = eleRecoSFFile_pTBelow20.Get( "EGamma_SF2D" );
    electronRecoSFHist_pTBelow20.SetDirectory( ROOT.gROOT );
    eleRecoSFFile_pTBelow20.Close();

    eleRecoSFFile_pTAbove20 = ROOT.TFile.Open( "../../weights/"
            + "weightFiles/leptonSF/egamma_recoEff_2016_pTAbove20.root" );
    electronRecoSFHist_pTAbove20 = eleRecoSFFile_pTAbove20.Get( "EGamma_SF2D" );
    electronRecoSFHist_pTAbove20.SetDirectory( ROOT.gROOT );
    eleRecoSFFile_pTAbove20.Close();

    merge2dhistograms(electronRecoSFHist_pTBelow20, electronRecoSFHist_pTAbove20)
