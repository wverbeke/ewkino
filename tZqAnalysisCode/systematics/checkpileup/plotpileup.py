############################################################
# read the histograms with pileup variations and plot them #
############################################################

import os
import sys
import ROOT
sys.path.append(os.path.join(os.path.dirname(__file__),'../../tools'))
import histtools
sys.path.append(os.path.join(os.path.dirname(__file__),'../../plotting'))
from singlehistplotter import plothistograms

if __name__=='__main__':

    mcfiledir = 'none'
    datafiledir = 'none'
    outfile = 'none'
    variable = 'none'
    histfile = 'none'
    merge = False

    if(len(sys.argv)==1 or 'help' in sys.argv[1:]):
	print('Plot a distribution with nominal and varied pileup weights and compare to data.')
	print('- If a file already exists containing all necessary histograms,')
	print('  use arguments: histfile, outfile, variable, merge=False')
	print('- Else, use arguments: mcfiledir, datafiledir, outfile, variable, merge=True')
	sys.exit()

    for arg in sys.argv[1:]:
	(argkey,argval) = arg.split('=',1)
	if argkey=='mcfiledir': mcfiledir = os.path.abspath(argval)
	elif argkey=='datafiledir': datafiledir = os.path.abspath(argval)
	elif argkey=='outfile': outfile = argval.split('.')[0]
	elif argkey=='variable': variable = argval
	elif argkey=='histfile': histfile = argval
	elif argkey=='merge': merge = (argval=='True')
	else:
	    print('### WARNING ###: argument {} not recognized.'+arg)

    # merge the files
    if merge:
	mcfiles = [os.path.join(mcfiledir,f) for f in os.listdir(mcfiledir) if f[-5:]=='.root']
	datafiles = ([os.path.join(datafiledir,f) for f in os.listdir(datafiledir) 
		    if (f[-5:]=='.root' and 'combined' in f)]) 
		    # (additional selection: only combined data files!)
	histfile = outfile+'.root'
	if os.path.exists(histfile):
	    os.system('rm '+histfile)
	haddcmd = 'hadd '+histfile
	for f in mcfiles + datafiles:
	    haddcmd += ' '+f
	print('will hadd {} files'.format(len(mcfiles+datafiles)))
	os.system(haddcmd)

    # read histograms from merged file
    nomhistlist = histtools.loadallhistograms(histfile,mustcontain=[variable,'nominal'],
						maynotcontain=['data'])
    uphistlist = histtools.loadallhistograms(histfile,mustcontain=[variable,'pileupUp'],
						maynotcontain=['data'])
    downhistlist = histtools.loadallhistograms(histfile,mustcontain=[variable,'pileupDown'],
                                                maynotcontain=['data'])
    datahistlist = histtools.loadallhistograms(histfile,mustcontain=['data',variable,'nominal'])
    
    # do some checks
    if( len(datahistlist)!=1 ):
	print('### ERROR ###: unexpected number of data histograms')
	sys.exit()
    if not ( len(nomhistlist)==len(uphistlist) and len(nomhistlist)==len(downhistlist) ):
	print('### ERROR ###: number of histograms for nominal/up/down is inconsistent')
	sys.exit()

    # sum histograms
    nomhist = nomhistlist[0]
    uphist = uphistlist[0]
    downhist = downhistlist[0]
    datahist = datahistlist[0]
    for nhist,uhist,dhist in zip(nomhistlist[1:],uphistlist[1:],downhistlist[1:]):
	nomhist.Add(nhist)
	uphist.Add(uhist)
	downhist.Add(dhist)

    # plot them
    outpngfile = outfile+'.png'
    clist = [ROOT.kBlack,ROOT.kRed,ROOT.kBlue,ROOT.kGreen]
    labellist = ['data','nominal','up','down']
    plothistograms([datahist,nomhist,uphist,downhist],
		    'number of events',variable,outpngfile,
		    errorbars=True,clist=clist,labels=labellist)
