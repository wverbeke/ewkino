##################################################################
# Script to read the output log files from the submission script #
##################################################################
import os
import sys
import ROOT
import numpy as np
import matplotlib.pyplot as plt

### help functions 

def getlogfiles(sdirname):
    # get list of log files
    logfiles = []
    for root,_,flist in os.walk(sdirname):
	for fname in flist:
	    if 'tmvatrain.sh.o' in fname:
		logfiles.append(os.path.join(root,fname))
    # print list of log files
    #print('found following log files:')
    #for fname in logfiles:
    #	print('\t'+fname)
    return logfiles

def sortbyauc(logfiles,printout=True):
    # loop over list of log files, find ROC AUC and sort by it.
    reslist = [] # contains tuples of relevant quantities
    for fname in logfiles:
	f = open(fname,'r')
	contents = f.readlines()[-4:]
	if(contents[0].rstrip('\n')=='---AUC (ROC)---'
	    and contents[2].rstrip('\n')=='---command---'):
	    roc = float(contents[1])
	    cmd = contents[3]
	    reslist.append((fname,cmd,roc))
	else:
	    print('### ERROR ###: something is wrong with the output log file '+fname)
	    print('               cannot read info; check and/or change required format!')
	    sys.exit()
    # sort and print output
    reslist.sort(key=lambda pair: pair[2])
    if printout:
	print('found following AUC results:')
	for el in reslist:
	    print(el)
    return reslist

def plotauc(reslist):
    # read reslist from sortbyauc and plot the AUC scores in a scatter plot
    auclist = [config[2] for config in reslist]
    xax = list(range(len(auclist)))
    plt.figure()
    plt.scatter(xax,auclist,s=10,c=auclist,cmap='jet',edgecolors='none')
    plt.xlabel('configuration number (sorted by resulting AUC)')
    plt.ylabel('AUC (ROC)')
    plt.title('BDT parameters grid search results')
    plt.savefig('figure.png')

def getrocs(sdirname):
    # get list of effB and corresponding effS
    effB = []
    effS = []
    paths = []
    # find all root files in directory
    for root,_,flist in os.walk(sdirname):
	for fname in flist:
	    if not '.root' in fname: continue
	    tfile = ROOT.TFile(os.path.join(root,fname))
	    roc = tfile.Get("outdata/Method_BDT/BDT/MVA_BDT_rejBvsS")
	    # check if roc histogram was actually found
	    try: nbins = roc.GetNbinsX()
	    except:
		print('### WARNING ###: requested histogram does not exist within this root file.')
		print('			skipping it...')
		continue
	    effBj = []
	    effSj = []
	    for i in range(1,roc.GetNbinsX()+1):
		effSj.append(roc.GetBinCenter(i))
		effBj.append(1-roc.GetBinContent(i))
	    effS.append(np.array(effSj))
	    effB.append(np.array(effBj))
	    paths.append(os.path.join(root,fname))
    return (effB,effS,paths)
	    
def plotROCroot(effB,effS,colorlist,labellist,outfigpath,outfigname,title):
        ncurves = len(colorlist)
        if ncurves>14:
                print('### ERROR ###: plot function is not suited for this many curves!')
                sys.exit()
        if(len(effB) != ncurves or len(effS) != ncurves or len(labellist) != ncurves):
                print('### ERROR ###: input to plot function has wrong dimensions!')
                sys.exit()
        cdict = ({0:ROOT.kRed+1,1:ROOT.kPink+7,2:ROOT.kMagenta,3:ROOT.kMagenta-9,
                4:ROOT.kMagenta+2,5:ROOT.kMagenta-1,6:ROOT.kBlue-6,
                7:ROOT.kAzure-2,8:ROOT.kCyan-9,9:ROOT.kBlue+2,10:ROOT.kGreen+4,
                11:ROOT.kGreen+3,12:ROOT.kGreen,13:ROOT.kGreen-3,14:ROOT.kGreen-9})

        # set fonts and text sizes
        titlefont = 6; titlesize = 40
        labelfont = 5; labelsize = 40
        axtitlefont = 5; axtitlesize = 40
        legendfont = 4; legendsize = 30

        # create canvas
        c1 = ROOT.TCanvas("c1","c1")
        c1.SetCanvasSize(800,800)
        #c1.SetLogx()
        c1.SetGrid()
        c1.SetLeftMargin(0.2)
        c1.SetBottomMargin(0.15)
        c1.SetTopMargin(0.12)

        # fill multigraph       
        leg = ROOT.TLegend(0.45, 0.17, 0.88, 0.2+ncurves*0.05)
        graphlist = []
        graphstack = ROOT.TMultiGraph()
        for i in range(ncurves):
                print('processing plot '+str(i+1)+' from '+str(ncurves))
                graphlist.append(ROOT.TGraph(len(effB[i]),effB[i],effS[i]))
                graphlist[i].SetLineColor(cdict[colorlist[i]])
                graphlist[i].SetLineWidth(4)
                graphstack.Add(graphlist[i])
                leg.AddEntry(graphlist[i], labellist[i], "l")
        graphstack.Draw("AL")

	# y-axis layout
        graphstack.SetMinimum(0.)
        graphstack.SetMaximum(1.)
        yax = graphstack.GetYaxis()
        yax.SetLabelFont(10*labelfont+3)
        yax.SetLabelSize(labelsize)
        yax.SetTitle('signal efficiency')
        yax.SetTitleFont(10*axtitlefont+3)
        yax.SetTitleSize(axtitlesize)

        # x-axis layout
        xax = graphstack.GetXaxis()
        xax.SetLimits(0.,1.)
        xax.SetLabelFont(10*labelfont+3)
        xax.SetLabelSize(labelsize)
        xax.SetTitle('background efficiency')
        xax.SetTitleFont(10*axtitlefont+3)
        xax.SetTitleSize(axtitlesize)
	xax.SetNdivisions(505)
        xax.CenterTitle(True)
        xax.SetTitleOffset(1.4)
        graphstack.Draw("AL")

        # legend
        leg.SetTextFont(10*legendfont+3)
        leg.SetTextSize(legendsize)
        leg.Draw()

        # title
        ttitle = ROOT.TLatex()
        ttitle.SetTextFont(10*titlefont+3)
        ttitle.SetTextSize(titlesize)
        ttitle.DrawLatexNDC(0.2,0.92,title)

        savename = outfigpath+outfigname
        c1.SaveAs(savename+'.png')
        print('figures saved in folder '+outfigpath)

### driver code

sdirname = 'configs' # subdirectory to scan for output files
if len(sys.argv)==2:
    sdirname = sys.argv[1]
clist = sortbyauc(getlogfiles(sdirname),printout=True)
plotauc(clist)

'''(effB,effS,paths) = getrocs(sdirname)
labels = []
for path in paths:
    label = ''
    idx = path.find('jobn')
    if idx>=0: label = str(int(path[idx+4])+1)+' variables'
    labels.append(label)
colors = list(range(len(effB)))
plotROCroot(effB,effS,colors,labels,'','test','a plot')'''
