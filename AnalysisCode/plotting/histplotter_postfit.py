##################################################################
# script to read combine output file and plot the postfit shapes #
##################################################################
import sys 
import os
import ROOT
import histplotter as hp

def loadhistograms(histfile,channel,mustnotcontain=[]):
    ### read fitDiagnostics output file and get histograms
    print('loading histograms...')
    mustnotcontain.append('total')
    f = ROOT.TFile.Open(histfile)
    dirname = 'shapes_fit_s/'+channel
    f.cd(dirname)
    gdir = ROOT.gDirectory
    histdict = {}
    keylist = gdir.GetListOfKeys()
    for key in keylist:
        hist = f.Get(dirname+'/'+key.GetName())
        try:
	    hist.SetDirectory(0)
	except:
	    print('')
        # check if histogram needs to be included
	keep = True
	for tag in mustnotcontain:
	    if tag in hist.GetName(): keep = False; break
	if not keep: continue
        # add hist to dict
        histdict[hist.GetName()] = hist
    return histdict

def tgraphtohist(tgraph,emptyhist):
    ### convert TGraphAsymmetricErrors to TH1F (only for data!)
    for i in range(tgraph.GetN()):
	for c in range(int(tgraph.GetY()[i])):
	    emptyhist.Fill(tgraph.GetX()[i])
    emptyhist.SetName(tgraph.GetName())
    emptyhist.SetTitle(tgraph.GetTitle())
    return emptyhist
    

if __name__=='__main__':

    datacarddir = os.path.abspath('../combine/datacards')
    datacard = 'datacard_signalregion_1_2016.txt'
    histfile = os.path.join(datacarddir,'fitDiagnostics'+datacard.replace('.txt','.root'))
    channel = datacard.replace('datacard_','').replace('dc_combined_','').replace('.txt','')

    # load histograms
    histdict = loadhistograms(histfile,channel)
    mchistlist = [histdict[h] for h in histdict.keys() if not 'data' in h]
    emptyhist = mchistlist[0].Clone()
    emptyhist.Reset()
    datahistlist = [histdict[h] for h in histdict.keys() if 'data' in h]
    datahist = tgraphtohist(datahistlist[0],emptyhist)

    # set histogram titles (for plot legend)
    for hist in mchistlist:
	hist.SetTitle(hist.GetName())
    datahist.SetTitle('data')

    figdir = os.path.join(datacarddir,datacard.replace('.txt',''))
    lumi = 0.
    if '2016' in histfile: lumi = 35900
    elif '2017' in histfile: lumi = 41500
    elif '2018' in histfile: lumi = 59700

    # get total systematic variation
    #slist = getallsystematics(allhists,varname)
    #syshist = getsystematichistogram(allhists,mcprocesses,[],varname,slist)
    #print('--------')
    #for i in range(1,syshist.GetNbinsX()+1):
    #    print(syshist.GetBinContent(i))
    syshist = None

    # set plot properties
    binwidth = mchistlist[0].GetBinWidth(1)
    if binwidth.is_integer():
        yaxtitle = 'events / '+str(int(binwidth)) #+' '+vardict['unit']
    else:
        yaxtitle = 'events / {0:.2f}'.format(binwidth) #+' '+vardict['unit']
    xaxtitle = 'bin number'
    #if not vardict['unit']=='':
    #    xaxtitle += '('+vardict['unit']+')'
    #figname = os.path.join(figdir,'fit_variable_postfit')
    figname = 'test'

    hp.plotdatavsmc(datahist,mchistlist,syshist,yaxtitle,False,xaxtitle,lumi,figname+'_lin')
    hp.plotdatavsmc(datahist,mchistlist,syshist,yaxtitle,True,xaxtitle,lumi,figname+'_log')

