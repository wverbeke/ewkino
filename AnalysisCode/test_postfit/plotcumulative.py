############################################################################
# make plots of the same histograms in many different fitDiagnostics files #
############################################################################

import sys
import os
import ROOT
sys.path.append(os.path.abspath('../plotting'))
from multihistplotter import plothistograms

def gethistlist( files, fittypes, bins, processes ):
    # arguments are equally long lists to specify which histogram from each file to extract
    histlist = []
    for filename,ft,b,p in zip(files,fittypes,bins,processes):
	f = ROOT.TFile.Open(filename,'read')
	histloc = ft+'/'+b+'/'+p
	hist = f.Get(histloc)
	hist.SetDirectory(0)
	histlist.append(hist)
	f.Close()
    return histlist

def makeplots( hists, names ):

    for hist,name in zip(hists,names):
	plothistograms([hist],'','',name,
                    normalize=False,normalizefirst=False,dolegend=True,
                    labellist=None,colorlist=None,
                    logy=False,errorbars=True,ratiorange=None,yrange=None,yminzero=False,
                    lumistr='',
                    doratio=False)

if __name__=='__main__':

    files = sorted([f for f in os.listdir('datacards') if f[:14]=='fitDiagnostics'
		    and f[-8:]=='obs.root'])
    numbers = [int(f.split('_')[2]) for f in files]
    files = [os.path.join('datacards',f) for f in files]
    print(files)
    fittypes = ['shapes_prefit']*len(files)
    bins = ['test_'+str(i) for i in numbers]
    processes = ['WZ']*len(files)

    histlist = gethistlist( files, fittypes, bins, processes )
    print(histlist)

    makeplots( histlist, bins )
