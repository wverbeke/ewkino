##############################################################################
# use combine to perform template fits to the data in order to get fake rate #
##############################################################################

import sys
import os
import ROOT
from array import array
from copy import copy
import numpy as np
# import job submission tools for qsub
sys.path.append('../skimmer')
from jobSubmission import initializeJobScript
# import local tools
sys.path.append('../Tools/python')
import histtools as ht
sys.path.append('../plotting/python')
import histplotter as hp
import hist2dplotter as h2dp
sys.path.append('python')
import fakeRateMeasurementTools as frt


def getseparator():
    ### help function for makedatacard
    return '-'*20 + '\n'

def makerow(uncname, unctype, ptoid, pimpacts):
    ### help function for makedatacard
    # input arguments:
    # - uncname: name for uncertainty
    # - unctype: type of uncertainty
    # - ptoid: dict of process tags to id numbers
    # - pimpacts: dict of process tags to impacts
    # returns:
    # a list with row elements for a datacard
    row = [uncname,unctype]
    for i in range(len(ptoid)): row.append('-')
    for p in pimpacts.keys():
	pindex = ptoid[p]+2
	pimpact = pimpacts[p]
	row[pindex] = str(pimpact)
    return row

def makealigned(stringlist):
    ### help function for makedatacard
    # append spaces to all strings in stringlist until they have the same length
    maxlen = 0
    # get maximum length of strings in list
    for s in stringlist:
        if len(s)>maxlen: maxlen=len(s)
    maxlen += 2
    # replace each string by string of fixed length
    for i,s in enumerate(stringlist):
        stringlist[i] = str('{:<'+str(maxlen)+'}').format(s)
    
def makedatacard(inputfile, datacardname, ftype, variable, year, flavour, pt, eta):
    ### prepare the datacard for a given fit
    # input arguments:
    # - inputfile: root file containing appropriate histograms
    # - datacardname: name of the datacard to create
    # - ftype: either "numerator" or "denominator"
    # - variable: variable name, usually "mT"
    # - year, flavour, pt, eta: used to get the correct histograms from the file
    # note: histograms are assumed to be named as follows:
    #	    process_ptype_ftype_var_year_flavour_pT_ptvalue_eta_etavalue
    #       where:
    #       - process is a process tag, which must be "total_prompt", "total_nonprompt" or "data"
    #       - ptype is either "prompt" or "nonprompt" (absent for data!)
    #       - ftype is either "numerator" or "denominator"
    #       - var is usually "mT"

    # global settings
    prompt_norm_unc = None 
    barlow_beeston_threshold = 1e12 
 
    # check if the given arguments correspond to valid histograms in the file
    channel = (str(ftype)+'_'+str(variable)+'_'+str(year)+'_'+str(flavour)
		+'_pT_'+str(pt)+'_eta_'+str(eta))
    f = ROOT.TFile.Open(inputfile)
    histnames = {}
    processes = ['total_prompt','total_nonprompt','data'] 
    for p in processes:
	histname = p+'_'+channel
	try:
	    hist = f.Get(histname)
	    hist.GetBinContent(1)
	    histnames[p] = histname
	except:
	    print('### WARNING ###: requested histogram {} seems not to be present in {}'
		    .format(histname,inputfile))
	    return -1

    # open (recreate) datacard file
    datacard = open(datacardname,'w')
    # write nchannels, nprocesses and nparameters
    datacard.write('imax\t1'+'\n') # only one channel
    datacard.write('jmax\t1'+'\n') # only one 'background' (prompt)
    datacard.write('kmax\t'+'*\n')
    datacard.write(getseparator())
    # write file info
    simhistname = '$PROCESS_'+channel
    datahistname = simhistname.replace('$PROCESS','data')
    datacard.write('shapes * '+channel+' '+inputfile+' '+simhistname+'\n')
    datacard.write('shapes data_obs '+channel+' '+inputfile+' '+datahistname+'\n')
    datacard.write(getseparator())
    # write bin info
    datacard.write('bin\t\t'+channel+'\n')
    datacard.write('observation\t'+str(f.Get(histnames['data']).Integral())+'\n')
    datacard.write(getseparator())
    # make columns for process definition
    c1 = ['bin','process','process','rate']
    c2 = ['','','','']
    initialsignal = f.Get(histnames['total_nonprompt']).Integral()
    csig = [channel,'total_nonprompt','0',str(initialsignal)]
    columns = [c1,c2,csig]
    pcounter = 1
    ptoid = {'total_nonprompt':0}
    for p in processes:
	if( p=='data' or p=='total_nonprompt' ): continue 
	columns.append([channel,p,str(pcounter),str(f.Get(histnames[p]).Integral())])
	ptoid[p] = pcounter
	pcounter += 1

    # add uncertainties
    rows = []
    # uncertainty on signal normalization (not needed but keep for reference)
    #rows.append( makerow('norm_total_nonprompt','lnN',ptoid,{'total_nonprompt':10}) )
    for p in processes:
	if(p=='data' or p=='total_nonprompt'): continue
	# not fully sure on the best value to put here
	# in principle the uncertainty is large but this spoils the final uncertainty
	# in the fake rate map
	# and a smaller value is already sufficient to get decent fits in every bin
	if prompt_norm_unc is not None: 
	    rows.append( makerow( 'norm_'+p,'lnN',ptoid,{p:prompt_norm_unc} ) )

    # add rows to columns
    for row in rows:
        for rowel,c in zip(row,columns):
            c.append(rowel)
    # align columns
    for c in columns: makealigned(c)
    # write all info row by row
    nrows = len(columns[0])
    for row in range(nrows):
        for col in range(len(columns)):
            datacard.write(columns[col][row]+' ')
        datacard.write('\n')
        if(row==3): datacard.write(getseparator())
    datacard.write(getseparator())
    # manage statistical uncertainties
    if barlow_beeston_threshold is not None:
	datacard.write(channel+' autoMCStats '+str(barlow_beeston_threshold))
    # close datacard
    datacard.close()
    f.Close()
    return initialsignal

def addcombinecommands(script, datacard, impactfig=''):
    ### add combine commands for a given datacard to a given script
    # input arguments:
    # - script: name of executable script
    # - datacard: name of datacard
    # - impactfig: name of impact plot file (default: do not make impact plot)

    # make workspace
    script.write('text2workspace.py '+datacard+'\n')
    workspace = datacard.replace('.txt','.root')
    name = datacard.replace('.txt','')
    # make outputfiles
    ss_obs_outfile = datacard.replace('.txt','_out_signalstrength_obs.txt')
    impactjson = name+'_impacts.json'
    doimpacts = True
    if len(impactfig)==0:
	doimpacts = False

    # run FitDiagnostics to compute signal strength
    ss_command = 'combine -M FitDiagnostics '+workspace+' -n '+name
    ss_command += ' --saveShapes --saveWithUncertainties'
    ss_command += ' --cminDefaultMinimizerStrategy 0'
    ss_command += ' --robustFit=1'
    ss_command += ' --rMin 0 --rMax 3'
    script.write(ss_command+' > '+ss_obs_outfile+' 2> '+ss_obs_outfile+'\n')

    if not doimpacts: return 

    # run Impacts to compute pull on prompt normalization
    command = 'combineTool.py -M Impacts -d '+workspace+' -n '+name
    command += ' -m 100' # seems to be required argument, put any value?
    initfit = command + ' --doInitialFit\n'
    impacts = command + ' --doFits\n'
    output = command + ' --output {}\n'.format(impactjson)
    plot = 'plotImpacts.py -i {} -o {}\n'.format(impactjson,impactfig)
    script.write(initfit)
    script.write(impacts)
    script.write(output)
    script.write(plot)

def fitfailed(filename):
    ### read if a 'Fit failed.' statement is present in a combine output txt file
    # (alternatively, use readr and check if it returns 0 + 0 - 0)
    with open(filename,'r') as f:
        for l in f.readlines():
            if 'Fit failed.' in l: return True
    return False

def readr(filename):
    ### read signal strength (r) from the output of a combine command directed to a file
    r = 0.
    uperror = 0.
    downerror = 0.
    with open(filename,'r') as f:
        for l in f.readlines():
            if l[:11]=='Best fit r:':
                l = l.replace('Best fit r: ','').replace(' (68% CL)','')
                l = l.split(' ')
                r = l[0]
                uperror = l[2].split('/')[1].strip('+')
                downerror = l[2].split('/')[0].strip('-')
    return (float(r),float(uperror),float(downerror))


if __name__=='__main__':
   
    sys.stderr.write('###starting###\n')
 
    if not len(sys.argv)==5:
	print('### ERROR ###: wrong number of command-line arguments')
	print('               use: python fitTemplates.py <var> <flavour> <year> <frmapdir>')
	sys.exit()
    
    # initialize arguments and global parameters
    var = sys.argv[1]
    flavour = sys.argv[2]
    year = sys.argv[3]
    frmapdir = sys.argv[4]
    instancename = '{}_{}_{}'.format(flavour, year, var)
    inputfile = os.path.abspath('fakeRateMeasurement_data_'+instancename+'_histograms.root')

    # check if input file exists
    if not os.path.exists(inputfile):
	raise Exception('ERROR: required input file {} does not exist.'.format(inputfile))
    # make a working directory to store intermediate results
    workingdir = 'fakeRateTemplateFits_{}'.format(instancename)
    if not os.path.exists(workingdir):
	os.makedirs(workingdir)

    # read pt and eta bins and ranges
    basehistname = 'data_numerator_'+var+'_'+year+'_'+flavour
    (ptbins,etabins) = frt.readptetabins(inputfile,basehistname)
    ptrange = copy(ptbins); ptrange.append(100)
    etarange = copy(etabins); etarange.append(2.5)
    if(flavour=='muon'): etarange[-1]=2.4

    # make 2D maps to hold number of nonprompt leptons in num and denom
    numyieldmap = ROOT.TH2D( "fake-rate", "fake-rate; p_{T} (GeV); |#eta|", 
		len(ptrange)-1, array('f',ptrange), 
		len(etarange)-1, array('f',etarange))
    numyieldmap.SetDirectory(0)
    numyieldmap.Sumw2()
    denomyieldmap = ROOT.TH2D( "", "", 
		len(ptrange)-1, array('f',ptrange), 
		len(etarange)-1, array('f',etarange))
    denomyieldmap.SetDirectory(0)
    denomyieldmap.Sumw2()
    # group the nummap and denommap in a dict for easier access
    npyieldmaps = {'denominator':denomyieldmap,'numerator':numyieldmap}

    # make 2D maps to hold the measured signal strengths in num and denom
    numrmap = ROOT.TH2D( "signal strength", "signal strength; p_{T} (GeV); |#eta|",
                len(ptrange)-1, array('f',ptrange),
                len(etarange)-1, array('f',etarange))
    numrmap.SetDirectory(0)
    numrmap.Sumw2()
    denomrmap = ROOT.TH2D( "signal strength", "signal strength; p_{T} (GeV); |#eta|",
                len(ptrange)-1, array('f',ptrange),
                len(etarange)-1, array('f',etarange))
    denomrmap.SetDirectory(0)
    denomrmap.Sumw2()
    # group the nummap and denommap in a dict for easier access
    rmaps = {'denominator':denomrmap,'numerator':numrmap}

    # move to workingdir
    cwd = os.getcwd()
    os.chdir(workingdir)

    # loop over pt and eta bins
    print('running on file {}'.format(inputfile))
    print('start looping over pt and eta bins...')
    for ptbinnb,ptbin in enumerate(ptbins):
	for etabinnb,etabin in enumerate(etabins):
	    #if( ptbinnb!=2 or etabinnb!=2 ): continue # for testing on small number of bins
	    ptbinstr = str(ptbin).replace('.','p')
	    etabinstr = str(etabin).replace('.','p')
	    print('   bin pt {}, eta {}'.format(ptbinstr,etabinstr))
	    for ftype in ['denominator','numerator']:
	
		### get the correct histograms and make a prefit plot
		thisbin = ftype+'_'+var+'_'+year+'_'+flavour+'_pT_'+ptbinstr+'_eta_'+etabinstr
		# get histograms
		histograms = frt.loadselectedhistograms(inputfile,
                                ftype, var, year, flavour, ptbin, etabin)
                datahist = histograms['datahist']
		prompthists = histograms['prompthists']
		nonprompthists = histograms['nonprompthists']	
		# add all prompt processes into one histogram
		prompthist = prompthists[0].Clone()
		prompthist.Reset()
		for hist in prompthists: prompthist.Add( hist )
		prompthist.SetName('total_prompt_'+thisbin)
		prompthist.SetTitle('Prompt')
		# do the same for nonprompt processes
		nonprompthist = nonprompthists[0].Clone()
		nonprompthist.Reset()
		for hist in nonprompthists: nonprompthist.Add( hist )
		nonprompthist.SetName('total_nonprompt_'+thisbin)
		nonprompthist.SetTitle('Nonprompt')

		# temp for testing: set the errors to zero
		#for i in range(0,prompthist.GetNbinsX()+2):
		#    if prompthist.GetBinContent(i)>0:
		#	error = prompthist.GetBinError(i) 
		#	prompthist.SetBinError(i, error/100.)
		#    if nonprompthist.GetBinContent(i)>0: 
		#    	error = nonprompthist.GetBinError(i)
		#    	nonprompthist.SetBinError(i, error/100.)

		# make a prefit plot
                xaxtitle = datahist.GetXaxis().GetTitle()
                yaxtitle = datahist.GetYaxis().GetTitle()
                lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
                            '2016PreVFP':19520, '2016PostVFP':16810, 
                            '2016Merged':36300 }
                lumi = lumimap[year]
                extracmstext = 'Preliminary'
                extrainfos = []
                extrainfos.append('{} {}'.format(year, flavour))
                extrainfos.append('pT: {}, eta: {}, {}'.format(ptbin, etabin, ftype))
                extrainfos.append('prefit')
                colormap = {}
                colormap['Prompt'] = ROOT.kAzure + 1
                colormap['Nonprompt'] = ROOT.kRed - 7
                figname = os.path.join(thisbin+'_prefit')
                hp.plotdatavsmc( figname, datahist,
                            [prompthist,nonprompthist],
                            datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
                            colormap=colormap,
                            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
                            extracmstext=extracmstext,
                            extrainfos=extrainfos, infosize=15 )
		# write the histograms to a temporary file
		tempfilename = thisbin+'_histograms.root'
		f = ROOT.TFile.Open(tempfilename,'recreate')
		prompthist.Write()
		nonprompthist.Write()
		datahist.Write()
		f.Close()
		prefitnp = nonprompthist.Integral()
		
		### run combine fit
		# make the datacard
		datacard = thisbin+'_datacard.txt'
		makedatacard(tempfilename, datacard, ftype, var, year, flavour, ptbinstr, etabinstr)
		# run combine
		script_name = datacard.replace('.txt','.sh')
		impactfig = '' # empty string means no impact plot will be made
		#impactfig = datacard.replace('.txt','_impacts')
		with open( script_name, 'w') as script:
		    initializeJobScript( script, cmssw_version = 'CMSSW_10_2_16_patch1' )
		    addcombinecommands(script, datacard, impactfig=impactfig)
		os.system('bash '+script_name)
		resfile = datacard.replace('.txt','_out_signalstrength_obs.txt')
		if fitfailed(resfile):
		    print('### WARNING ###: fit for this bin seems to have failed....')
		    print('continuing to next bin without postfit processing.')
		    continue

		### process results
		(r,uperror,downerror) = readr(resfile)
		print('measured signal strength: {} + {} - {}'.format(r,uperror,downerror))
		# make post-fit distributions
		postfitfile = 'fitDiagnostics'+datacard.replace('.txt','.root')
		f = ROOT.TFile.Open(postfitfile,'read')
		# get post-fit histograms and set the correct x-axis bin values
		temp = f.Get('shapes_fit_s/'+thisbin+'/total_prompt')
		postfitprompthist = prompthist.Clone()
		postfitprompthist.Reset()
		for i in range(0, postfitprompthist.GetNbinsX()+2):
		    postfitprompthist.SetBinContent( i, temp.GetBinContent(i) )
		    postfitprompthist.SetBinError( i, temp.GetBinError(i) )
		postfitprompthist.SetTitle('Prompt')
		postfitprompthist.SetDirectory(0)
		temp = f.Get('shapes_fit_s/'+thisbin+'/total_nonprompt')
		postfitnonprompthist = nonprompthist.Clone()
                postfitnonprompthist.Reset()
                for i in range(0, postfitnonprompthist.GetNbinsX()+2):
                    postfitnonprompthist.SetBinContent( i, temp.GetBinContent(i) )
                    postfitnonprompthist.SetBinError( i, temp.GetBinError(i) )
		postfitnonprompthist.SetTitle('Nonprompt')
		postfitnonprompthist.SetDirectory(0)
		f.Close()
		# make the postfit plot
                xaxtitle = datahist.GetXaxis().GetTitle()
                yaxtitle = datahist.GetYaxis().GetTitle()
                lumimap = {'all':137600, '2016':36300, '2017':41500, '2018':59700,
                            '2016PreVFP':19520, '2016PostVFP':16810, 
                            '2016Merged':36300 }
                lumi = lumimap[year]
                extracmstext = 'Preliminary'
                extrainfos = []
                extrainfos.append('{} {}'.format(year, flavour))
                extrainfos.append('pT: {}, eta: {}, {}'.format(ptbin, etabin, ftype))
		extrainfos.append('postfit')
                colormap = {}
                colormap['Prompt'] = ROOT.kAzure + 1
                colormap['Nonprompt'] = ROOT.kRed - 7
                figname = os.path.join(thisbin+'_postfit')
                hp.plotdatavsmc( figname, datahist,
                            [postfitprompthist,postfitnonprompthist],
                            datalabel='Data', p2yaxtitle='#frac{Data}{Pred.}',
                            colormap=colormap,
                            xaxtitle=xaxtitle,yaxtitle=yaxtitle,lumi=lumi,
                            extracmstext=extracmstext,
                            extrainfos=extrainfos, infosize=15 )
		
		### fill the yield maps ###
		binindex = numyieldmap.FindBin(ptbin+1e-6,etabin+1e-6)
		# directly take integral of postfit distribution
		# (but still use signal strength measurement for relative error)
		postfitnp = postfitnonprompthist.Integral()
		npyieldmaps[ftype].SetBinContent(binindex,postfitnp)
		npyieldmaps[ftype].SetBinError(binindex,postfitnp*max(uperror,downerror)/r)
		rmaps[ftype].SetBinContent(binindex, r)
		rmaps[ftype].SetBinError(binindex, max(uperror,downerror))
		# print results
                print('initial amount of nonprompt leptons: {}'.format(prefitnp))
                print('measured signal strength: {} + {} - {}'.format(r,uperror,downerror))
                print('post-fit amount of nonprompt leptons: {}'.format(postfitnp))

    # save and plot the signal strength maps
    rmapfile = 'signalstrengths_'+instancename+'.root'
    f = ROOT.TFile.Open(rmapfile,'recreate')
    numrmap.Write("signalstrengths_numerator")
    denomrmap.Write("signalstrengths_denominator")
    f.Close()
    title = 'Signal strengths for {} {}s numerator'.format(year, flavour)
    h2dp.plot2dhistogram( numrmap, rmapfile.replace('.root','_numerator.pdf'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )
    h2dp.plot2dhistogram( numrmap, rmapfile.replace('.root','_numerator.png'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )
    title = 'Signal strengths for {} {}s denominator'.format(year, flavour)
    h2dp.plot2dhistogram( denomrmap, rmapfile.replace('.root','_denominator.pdf'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )
    h2dp.plot2dhistogram( denomrmap, rmapfile.replace('.root','_denominator.png'), histtitle=title,
			    drawoptions='col0ztexte', cmin=0.5, cmax=1.5 )

    # move back to main directory
    os.chdir(cwd)

    ### make fake rate map
    frmap = numyieldmap.Clone()
    frmap.Divide(denomyieldmap)
    # save and plot fake rate map
    frmapfile = os.path.join(frmapdir,'fakeRateMap_data_'+instancename+'.root')
    f = ROOT.TFile.Open(frmapfile,'recreate')
    frmap.Write("fakeRate_" + flavour + "_" + year)
    f.Close()
    title = 'Fake rate map for {} {}s'.format(year, flavour)
    h2dp.plot2dhistogram( frmap, frmapfile.replace('.root','.pdf'), histtitle=title )
    h2dp.plot2dhistogram( frmap, frmapfile.replace('.root','.png'), histtitle=title )
    sys.stderr.write('###done###\n')
