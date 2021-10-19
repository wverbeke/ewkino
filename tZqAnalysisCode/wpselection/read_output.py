########################################
# read and plot output of wP selection #
########################################
import os
import sys
import copy
import numpy as np
import matplotlib.pyplot as plt
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist
from extendsamplelist import findsample

### default arguments
inputdir = 'test'
#samplelistdir = '/pnfs/iihe/cms/store/user/llambrec/trileptonskim_oldtuples'
samplelistdir = '/user/llambrec/ewkino/AnalysisCode/samplelists'
years = ['2016','2017','2018']

### overwrite using command line
if(len(sys.argv)>1):
    inputdir = sys.argv[1]

def read_wpselectionfile(wpsfile):
    # read an output file of wpselection.py
    # depends on the chosen formatting, modify here when needed!
    # output structure: dict of wp -> (dict of category -> number of events)
    res = {}
    with open(wpsfile,'r') as wpsf:
	for line in wpsf.readlines():
	    wpslist = [float(el) for el in line.split(':')]
	    wp = wpslist[0]
	    res[wp] = {'mmm':max(0,wpslist[1]),'mme':max(0,wpslist[2]),
			'mee':max(0,wpslist[3]),'eee':max(0,wpslist[4]),
			'all':max(0,wpslist[1]+wpslist[2]+wpslist[3]+wpslist[4])}
    return res

def formatoutput(wpsdict):
    # print the output of read_wpselectionfile in a readable format
    print('---------------------------------')
    header = str('{:<5}').format('WP')
    for cat in sorted(wpsdict[wpsdict.keys()[0]].keys()):
	header += str('{:<13}').format(cat)
    print(header)
    for wp in sorted(wpsdict.keys()):
	info = str('{:<5}').format(wp)
	for cat in sorted(wpsdict[wp].keys()):
	    info += str('{:<13}').format('{0:.3E}'.format(wpsdict[wp][cat]))
	print(info)
    print('---------------------------------')

def add_wpselectiondicts(d1,d2):
    # add two dicts that result from read_wpselectionfile
    # (for backgrounds)
    if len(d1.keys())==0:
	res = copy.deepcopy(d2)
	return res
    if(d1.keys().sort() != d2.keys().sort()
	    or d1[d1.keys()[0]].keys().sort() != d2[d2.keys()[0]].keys().sort()): 
	print('### ERROR ###: keys of dicts to be added are not equal')
	return None
    res = copy.deepcopy(d1)
    for key in d1.keys():
	for key2 in d1[key]:
	    res[key][key2] += d2[key][key2]
    return res

def plot_S_and_B(Sdict,Bdictlist,labels,outfilepath,cat='all',extrainfo=''):
    # plot absolute numbers of signal and background
    # 'Bdictlist' is a list of background dicts (see read_wpselectionfile)
    # the maximum number of samples supported currently is 4, but easily extendable
    # 'labels' is a list of labels (for background only)
    # for the allowed values of cat, see function read_wpselectionfile

    print('starting function plot_S_and_B')

    # get values
    xax = np.zeros(len(Sdict.keys()))
    yaxS = np.zeros(len(Sdict.keys()))
    yaxB = np.zeros((len(Bdictlist),len(Sdict.keys())))
    for i,x in enumerate(sorted(Sdict.keys())):
	xax[i] = x
	yaxS[i] = Sdict[x][cat]
	for j in range(len(Bdictlist)): yaxB[j,i] = Bdictlist[j][x][cat]
    
    # do basic plotting
    fig,ax = plt.subplots()
    ax.plot(xax,yaxS,color='g',marker='o',markersize=3,linestyle='-',label='signal')
    clist = ['red','deeppink','darkviolet','blue']
    if len(clist)<len(Bdictlist): 
	print('### ERROR ###: maximum number of background samples is '+len(clist))
	return None
    for j in range(len(Bdictlist)):
	ax.plot(xax,yaxB[j,:],color=clist[j],marker='o',markersize=3,linestyle='-',label=labels[j])
    ax.legend()

    # set plot style properties
    plt.xlim(( xax[0]-(xax[-1]-xax[0])/10. , xax[-1]+(xax[-1]-xax[0])/10. ))
    plt.ylim(( 0., max(np.amax(yaxB),np.amax(yaxS))*1.2 ))
    ax.grid()
    plt.xlabel('lepton MVA working point')
    plt.ylabel('number of events')
    plt.title('Number of signal and background events')
    if cat=='all': extrainfo += '\n'+r'($e/\mu , e/\mu , e/\mu$) - signatures'
    else: extrainfo += '\n($'+cat.replace('m','\mu ')+'$) - signatures'
    ax.text(0.03,0.88,extrainfo,transform=ax.transAxes,fontsize=14,
		bbox=dict(boxstyle='square',facecolor='white'))
    fig.savefig(outfilepath)
    plt.close()

def plot_S_vs_B(Sdict,Bdict,outfilepath,ptype='S/sqrt(S+B)',extrainfo=''):
    # plot a function of S and B
    # for the allowed values of ptype, see below.

    # check options
    if(ptype not in ['S/sqrt(S+B)','S/sqrt(B)','S/B']): 
	print('### ERROR ###: plot type "'+ptype+'" not recognized')
	return None

    print('starting function plot_S_vs_B')

    # get values
    categories = sorted(Sdict[Sdict.keys()[0]].keys())
    wps = sorted(Sdict.keys())
    vals = np.zeros((len(categories),len(wps)))
    xax = np.zeros(len(wps))
    for j,wp in enumerate(wps):
	xax[j] = wp
	for i,cat in enumerate(categories):
	    sval = Sdict[wp][cat]
	    bval = Bdict[wp][cat]
	    if(sval < 1e-12 or bval < 1e-12):
		val = 0
	    elif ptype=='S/sqrt(S+B)': val = sval/np.sqrt(sval+bval)
	    elif ptype=='S/sqrt(B)': val = sval/np.sqrt(bval)
	    elif ptype=='S/B': val = sval/bval
	    vals[i,j] = val

    # do basic plotting
    fig,ax = plt.subplots()
    clist = ['black','red','deeppink','darkviolet','blue']
    for i,cat in enumerate(categories):
	ax.plot(xax,vals[i,:],color=clist[i],marker='o',markersize=3,
		linestyle='-',label='$'+cat.replace('m','\mu ')+'$')
    ax.legend(ncol=len(categories),loc='lower center')

    # set plot style properties
    plt.xlim(( xax[0]-(xax[-1]-xax[0])/10. , xax[-1]+(xax[-1]-xax[0])/10. ))
    ymax = np.amax(vals)
    ymin = np.amin(vals)
    plt.ylim(( ymin-(ymax-ymin)/3. , ymax+(ymax-ymin)/3. ))
    ax.grid()
    plt.xlabel('lepton MVA working point')
    plt.ylabel(ptype)
    plt.title('Signal significance')
    ax.text(0.03,0.97,extrainfo,transform=ax.transAxes,fontsize=14,
		verticalalignment='top',horizontalalignment='left',
                bbox=dict(boxstyle='square',facecolor='white'))
    fig.savefig(outfilepath)
    plt.close()

#################################################
# main funcion					#
#################################################
allsignals = {}
allpbackgrounds = {}
allnpbackgrounds = {}
for year in years:

    # check if input directory exists
    thisinputdir = os.path.join(inputdir,year)
    if not os.path.exists(thisinputdir):
	print('### ERROR ###: input directory '+thisinputdir+' not found, skipping it.')
	continue

    # check if sample list can be found (needed to read process tags)
    thissamplelist = os.path.join(samplelistdir,'samplelist_tzq_'+year+'_MC.txt')
    if(not os.path.exists(thissamplelist)):
        print('### ERROR ###: sample list for year '+year+' not found, skipping it.')
	continue

    # read the sample list
    thissamplelist = readsamplelist(thissamplelist,unique=True)

    # loop over all files and fill dictionaries with event numbers
    signal = {}
    pbackground = {}
    npbackground = {}
    for wpsfile in [f for f in os.listdir(thisinputdir) if '.txt' in f]:
	wpsfilepath = os.path.join(thisinputdir,wpsfile)
	sample_name = wpsfile[:wpsfile.rfind('_')]
	#print(sample_name)
	process_name = findsample(sample_name,thissamplelist)['process_name']
	#print(process_name)
	if process_name=='tZq':
	    signal = add_wpselectiondicts(signal,read_wpselectionfile(wpsfilepath))
	elif process_name=='nonprompt':
	    npbackground = add_wpselectiondicts(npbackground,read_wpselectionfile(wpsfilepath))
	else:
	    pbackground = add_wpselectiondicts(pbackground,read_wpselectionfile(wpsfilepath))
    totbackground = add_wpselectiondicts(pbackground,npbackground)
    
    allsignals = add_wpselectiondicts(allsignals,signal)
    allpbackgrounds = add_wpselectiondicts(allpbackgrounds,pbackground)
    allnpbackgrounds = add_wpselectiondicts(allnpbackgrounds,npbackground)

    # make plots of absolute numbers:
    for cat in ['all','mmm','mme','mee','eee']:
	plot_S_and_B(signal,[totbackground,pbackground,npbackground],
			['total B','prompt B','nonprompt B'],
			os.path.join(thisinputdir,'SandB_'+cat+'.png'),
			extrainfo=year+' simulation',cat=cat)
    # make plots of figures of merit:
    for i,metric in enumerate(['S/sqrt(S+B)','S/sqrt(B)','S/B']):
	plot_S_vs_B(signal,totbackground,os.path.join(thisinputdir,'SvsB_'+str(i)+'.png'),
		    ptype=metric,extrainfo=year+' simulation')
	plot_S_vs_B(signal,npbackground,os.path.join(thisinputdir,'SvsnpB_'+str(i)+'.png'),
                    ptype=metric,extrainfo=year+' simulation\nnon-prompt background only')

# make all plots for all years combined
alltotbackgrounds = add_wpselectiondicts(allpbackgrounds,allnpbackgrounds)
for cat in ['all','mmm','mme','mee','eee']:
    plot_S_and_B(allsignals,[alltotbackgrounds,allpbackgrounds,allnpbackgrounds],
                    ['total B','prompt B','nonprompt B'],
                    os.path.join(inputdir,'SandB_'+cat+'.png'),
                    extrainfo='all years',cat=cat)
for i,metric in enumerate(['S/sqrt(S+B)','S/sqrt(B)','S/B']):
    plot_S_vs_B(allsignals,alltotbackgrounds,os.path.join(inputdir,'SvsB_'+str(i)+'.png'),
                ptype=metric,extrainfo='all years')
    plot_S_vs_B(allsignals,allnpbackgrounds,os.path.join(inputdir,'SvsnpB_'+str(i)+'.png'),
                ptype=metric,extrainfo='all years\nnon-prompt background only')

