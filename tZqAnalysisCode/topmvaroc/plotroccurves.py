############################################################################
# Read ROOT histograms filled with classifier response and make ROC curves #
############################################################################

import os
import sys
import argparse
import numpy as np
import matplotlib.pyplot as plt
sys.path.append('../tools')
import histtools as ht

def getrocfromhists( signalhist, backgroundhist ):
    
    nbins = signalhist.GetNbinsX()
    xlow = signalhist.GetBinLowEdge(1)
    xhigh = signalhist.GetBinLowEdge(nbins)+signalhist.GetBinWidth(nbins)
    nsig = np.zeros(nbins-1)
    nbck = np.zeros(nbins-1)
    # loop over bins
    for i in range(1,nbins):
        # threshold is lower edge of next bin
        nsig[i-1] = signalhist.Integral(i+1,nbins+1)
        nbck[i-1] = backgroundhist.Integral(i+1,nbins+1)
    # normalize 
    nsig = nsig[::-1]/signalhist.Integral()
    nbck = nbck[::-1]/backgroundhist.Integral()
    return {'signalefficiency':nsig,'backgroundefficiency':nbck}

def plotrocs( inputlist, points=None, title=None ):
    # inputlist contains dicts (one dict per curve)
    # with entries 'sigeff','bckeff','color','label'
    
    fig,ax = plt.subplots()
    for inputdict in inputlist:
        ax.plot(inputdict['bckeff'],inputdict['sigeff'],
                label=inputdict['label'],color=inputdict['color'],
                linewidth=2.)
    if points is not None:
        for i, inputdict in enumerate(points):
            ax.scatter(inputdict['bckeff'],inputdict['sigeff'],
                #label=inputdict['label'],
                label='Cut-based IDs' if i==0 else None,
                s=30, c=inputdict['color'],
                zorder=10)
    ax.legend(loc='lower right', fontsize=10)
    if title is not None: ax.set_title(title, fontsize=15)
    # general axis titles
    ax.set_xlabel('Background effiency', fontsize=15)
    ax.set_ylabel('Signal efficiency', fontsize=15)
    ax.set_xscale('log')
    # set y axis limits: hard-coded
    ax.set_ylim(0.5,1.001)
    # set y axis limits: adaptive
    #ylowlim = np.amin(np.where((sigeff>0.) & (bckeff>0.),sigeff,1.))
    #ylowlim = 2*ylowlim-1.
    #ax.set_ylim((ylowlim,1+(1-ylowlim)/5))
    # set x axis limits: hard-coded
    #ax.set_xlim(5e-3,1)
    # set x axis limits: adaptive
    #ax.set_xlim((np.amin(np.where(inputlist[0]['bckeff']>0.,inputlist[0]['bckeff'],1.))/2.,1.))
    # set x axis limits: adaptive based on y axis
    ax.set_xlim(inputlist[0]['bckeff'][np.nonzero(inputlist[0]['sigeff']>ax.get_ylim()[0])[0][0]], 1)
    ax.grid(which='both', axis='x')
    ax.grid(which='major', axis='y')
    return (fig, ax)


if __name__=='__main__':

    # read command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfile', type=os.path.abspath, required=True)
    parser.add_argument('-o', '--outputdir', type=os.path.abspath, required=True)
    parser.add_argument('--extrainfos', default=None)
    parser.add_argument('--lumitext', default=None)
    args = parser.parse_args()

    # fixed arguments
    mvanames = ['TTH', 'TZQ', 'TOP']
    mvawpnames = ['TOPMedium', 'TOPTight']
    cutbasedidnames = ['POGLoose', 'POGMedium', 'POGTight']
    pttags = ['pt10', 'pt10to25', 'pt25'] 
    flavours = ['muon', 'electron']

    # color, style and label definitions
    cdict = {
      'TTH':'b',
      'TZQ': 'blueviolet',
      'TOP': 'deeppink',
    }
    cdict['TOPMedium'] = cdict['TOP']
    cdict['TOPTight'] = cdict['TOP']
    ldict = {
      'TTH': 'ttH lepton MVA',
      'TZQ': 'tZq lepton MVA',
      'TOP': 'TOP lepton MVA (this work)',
      'POGLoose': 'Cut-based ID loose',
      'POGMedium': 'Cut-based ID medium',
      'POGTight': 'Cut-based ID tight'
    }
    flavourlabels = {
        'muon': 'Muons',
        'electron': 'Electrons'
    }
    ptlabels = {
        'pt10': '$p_{T}$ > 10 GeV',
        'pt10to25': '10 < $p_{T}$ < 25 GeV',
        'pt25': '$p_{T}$ > 25 GeV'
    }

    # make output directory if needed
    if not os.path.exists(args.outputdir): os.makedirs(args.outputdir)

    # loop over flavours and pt bins
    for flavour in flavours:
        for pttag in pttags:
            print('making ROC curves for '+flavour+'s with pt in '+pttag)
            # loop over MVAs and get ROC curve
            mvarocs = []
            for i,mvaname in enumerate(mvanames):
                print('mva: {}'.format(mvaname))
                histlist = ht.loadhistograms(args.inputfile,
                  mustcontainall=[mvaname,flavour,pttag])
                  #maynotcontainone=[el for el in pttags if (el!=pttag and pttag in el)])
                histlist = [h for h in histlist if h.GetName().startswith(mvaname+'_')]
                histlist = [h for h in histlist if h.GetName().endswith('_'+pttag)]
                print('Found following histograms:')
                for hist in histlist: print('  '+hist.GetName())
                if len(histlist)!=2:
                    msg = 'ERROR: expecting two histograms (prompt and nonprompt)'
                    raise Exception(msg)
                prompthist = histlist[0]
                nonprompthist = histlist[1]
                if 'nonprompt' in histlist[0].GetName():
                    prompthist = histlist[1]
                    nonprompthist = histlist[0]
                roc = getrocfromhists(prompthist,nonprompthist)
                color = cdict[mvaname]
                label = ldict[mvaname]
                mvarocs.append({'sigeff':roc['signalefficiency'],
                                'bckeff':roc['backgroundefficiency'],
                                'label':label,
                                'color':color})
            # loop over cut-based IDs and get efficiencies
            cutbasedids = []
            for i, cutbasedidname in enumerate(cutbasedidnames + mvawpnames):
                print('cut-based id: {}'.format(cutbasedidname))
                histlist = ht.loadhistograms(args.inputfile,
                  mustcontainall=[cutbasedidname,flavour,pttag])
                histlist = [h for h in histlist if h.GetName().startswith(cutbasedidname+'_')]
                histlist = [h for h in histlist if h.GetName().endswith('_'+pttag)]
                print('Found following histograms:')
                for hist in histlist: print('  '+hist.GetName())
                if len(histlist)!=2:
                    msg = 'ERROR: expecting two histograms (prompt and nonprompt)'
                    raise Exception(msg)
                prompthist = histlist[0]
                nonprompthist = histlist[1]
                if 'nonprompt' in histlist[0].GetName():
                    prompthist = histlist[1]
                    nonprompthist = histlist[0]
                sigeff = float(prompthist.GetBinContent(2))/prompthist.Integral()
                bckeff = float(nonprompthist.GetBinContent(2))/nonprompthist.Integral()
                print('Result: signal efficiency: {:.3f}, background efficiency: {:.3f}'.format(
                      sigeff, bckeff))
                cutbasedids.append({'sigeff': sigeff, 'bckeff': bckeff,
                                    'label': ldict.get(cutbasedidname, cutbasedidname),
                                    'color': cdict.get(cutbasedidname, 'dimgrey')})
            # make plot
            (fig, ax) = plotrocs(mvarocs, points=cutbasedids)
            # extra text and aesthetics
            extralabel = flavourlabels.get(flavour,flavour)+', '+ptlabels.get(pttag,pttag)
            txt = ax.text(0.97, 0.32, extralabel,
                    ha='right', transform=ax.transAxes, fontsize=10)
            txt.set_bbox(dict(facecolor='white', alpha=0.7, edgecolor='gray'))
            # lumi text
            if args.lumitext is not None:
                txt = ax.text(0.99, 1.02, args.lumitext,
                        ha='right', transform=ax.transAxes, fontsize=10)
            # cms text
            txt = ax.text(0.03, 0.97, r'$\bf{CMS}$ Work in Progress',
                    va='top', transform=ax.transAxes, fontsize=11)
            # save figure
            outputfile = '_'.join(['roc', flavour, pttag])
            outputfile = os.path.join(args.outputdir, outputfile)
            fig.savefig(os.path.splitext(outputfile)[0]+'.png')
