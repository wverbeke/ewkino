#######################################################################
# python script to scan a range of working points of a given variable #
#######################################################################
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
import ROOT

def scansingletree(infile,treename,variable,wps):
    f = ROOT.TFile.Open(infile)
    tree = f.Get(treename)
    try:
	tree.GetEntry(0)
    except:
	print('### WARNING ###: something went wrong for tree '+treename+' in file '+infile)
    res = np.zeros(len(wps))
    nentries = tree.GetEntries()
    vallist = np.zeros(nentries)
    weightlist = np.zeros(nentries)
    for j in range(0,nentries):
	tree.GetEntry(j)
	varvalue = getattr(tree,variable)
	weight = getattr(tree,'_normweight')
	vallist[j] = varvalue
	weightlist[j] = weight
	res += np.where(wps<varvalue,weight,0)
    hist = np.histogram(vallist,bins=wps,weights=weightlist) # not yet used at this point...
    return res

def loadfiles(indirs,mustcontain=[],mustnotcontain=[]):
    filelist = []
    # loop over all directories and files in these directories
    for indir in indirs:
	for filename in os.listdir(indir):
	    # check if file is a root file
	    if not filename[-5:]=='.root': continue
	    # check if file contains correct tags (logical OR between mustcontain!)
	    isgood = False
	    if len(mustcontain)==0: isgood = True
	    for tag in mustcontain:
		if tag in filename:
		    isgood = True
		    break
	    if not isgood: continue
	    for tag in mustnotcontain:
		if tag in filename:
		    isgood = False
		    break
	    if not isgood: continue
	    # add file
	    filelist.append(os.path.join(indir,filename))
    return filelist

def plot_S_and_B(wps,signalscan,backgroundscan,xaxtitle,yaxtitle,outfilepath,extrainfo=''):

    # do basic plotting
    fig,ax = plt.subplots()
    ax.plot(wps,signalscan,color='green',marker='o',markersize=3,linestyle='-',label='signal')
    ax.plot(wps,backgroundscan,color='red',marker='o',markersize=3,linestyle='-',label='background')
    ax.legend()

    # set plot style properties
    plt.xlim(( wps[0]-(wps[-1]-wps[0])/10. , wps[-1]+(wps[-1]-wps[0])/10. ))
    plt.ylim(( 0., max(np.amax(signalscan),np.amax(backgroundscan))*1.2 ))
    ax.grid()
    plt.xlabel(xaxtitle)
    plt.ylabel(yaxtitle)
    plt.title('Number of signal and background events')
    if extrainfo != '':
	ax.text(0.03,0.88,extrainfo,transform=ax.transAxes,fontsize=14,
                bbox=dict(boxstyle='square',facecolor='white'))
    fig.savefig(outfilepath)
    plt.close()


### Set list of input files
indirs = [os.path.abspath('/user/llambrec/Files/tthid_topmva_reduced/signalregion/2016MC_flat')]
indirs.append(os.path.abspath('/user/llambrec/Files/tthid_topmva_reduced/signalregion/2017MC_flat'))
indirs.append(os.path.abspath('/user/llambrec/Files/tthid_topmva_reduced/signalregion/2018MC_flat'))
treenames = ["blackJackAndHookers/treeCat1"]
treenames.append("blackJackAndHookers/treeCat2")
treenames.append("blackJackAndHookers/treeCat3")

### Set properties of variable to scan
variable = '_eventBDT'
wps = np.linspace(-1.,1.,num=50,endpoint=True)

### Get signal and background files
signalfiles = loadfiles(indirs,mustcontain=['tZq'])
backgroundfiles = loadfiles(indirs,mustnotcontain=['tZq'],
				    mustcontain=['DYJetsToLL','TTJets','TTTo'])
print('--- found following signal files: ---')
for sf in signalfiles: print(sf)
print('--- found following background files: ---')
for bf in backgroundfiles: print(bf)
#sys.exit()

### Loop over trees and scan variable
#signalhist = np.zeros(len(wps))
signalscan = np.zeros(len(wps))
for sfile in signalfiles:
    for treename in treenames:
	signalscan += scansingletree(sfile,treename,variable,wps)
#backgroundhist = np.zeros(len(wps))
backgroundscan = np.zeros(len(wps))
for bfile in backgroundfiles:
    for treename in treenames:
	backgroundscan += scansingletree(bfile,treename,variable,wps) 

### Make a plot
plot_S_and_B(wps,signalscan,backgroundscan,variable,'number of events','test.png',extrainfo='')
signalscan = signalscan/signalscan[0]
backgroundscan = backgroundscan/backgroundscan[0]
plot_S_and_B(wps,signalscan,backgroundscan,variable,'efficiency','test_relative.png',extrainfo='')
