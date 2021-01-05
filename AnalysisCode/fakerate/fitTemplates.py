##############################################################################
# use combine to perform template fits to the data in order to get fake rate #
##############################################################################
import sys
import os
import ROOT
from array import array
from copy import copy
sys.path.append('../../skimmer')
from jobSubmission import initializeJobScript
sys.path.append('../tools')
import histtools as ht 

def readptetabins(inputfile,basename):
    # read pt and eta bins from a root file containing histograms with names
    # basename_pt_<pt>_eta_<eta>
    # factorized binning is assumed, so separate pt and eta bin arrays

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
    
def getseparator():
    # help function for makedatacard
    return '-'*20 + '\n'

def makerow(uncname,unctype,ptoid,pimpacts):
    # help function for makedatacard
    row = [uncname,unctype]
    for i in range(len(ptoid)): row.append('-')
    for p in pimpacts.keys():
	pindex = ptoid[p]+2
	pimpact = pimpacts[p]
	row[pindex] = str(pimpact)
    return row

def makealigned(stringlist):
    # help function for makedatacard
    # append spaces to all strings in stringlist until they have the same length
    
    # get maximum length of strings in list
    maxlen = 0
    for s in stringlist:
        if len(s)>maxlen: maxlen=len(s)
    maxlen += 2
    # replace each string by string of fixed length
    for i,s in enumerate(stringlist):
        stringlist[i] = str('{:<'+str(maxlen)+'}').format(s)
    

def makedatacard(inputfile,datacardname,tag,variable,year,flavour,pt,eta):
    # prepare the datacard for a given root file
    # the root file in this context should contain histograms with the following naming:
    # <total_prompt/total_nonprompt/data>_<numerator/denominator>_<variable>_<year>_<flavour>_pT_<pt>_eta_<eta>
    
    # check if the given arguments correspond to valid histograms in the file
    channel = (str(tag)+'_'+str(variable)+'_'+str(year)+'_'+str(flavour)
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
	rows.append( makerow( 'norm_'+p,'lnN',ptoid,{p:1.1} ) )
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
    threshold = 10
    datacard.write(channel+' autoMCStats '+str(threshold))
    # close datacard
    datacard.close()
    f.Close()
    return initialsignal

def addcombinecommands(script,datacard,impactfig=''):
    # add combine commands for a given datacard to a given script

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
    # read if a 'Fit failed.' statement is present in a combine output txt file
    # (alternatively, use readr and check if it returns 0 + 0 - 0)
    with open(filename,'r') as f:
        for l in f.readlines():
            if 'Fit failed.' in l: return True
    return False


def readr(filename):
    # read signal strength (r) from the output of a combine command directed to a file
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


def subselect_prefitplot(histlist,binname):
    # select suitable histograms from a given list to make prefit plot
    newhistlist = []
    # make separate histogram for the aggregation of all nonprompt contributions
    # from prompt simulation files (total contribution is very small)
    othernphist = histlist[0].Clone()
    othernphist.Reset()
    othernphist.SetName('other_nonprompt_'+binname)
    othernphist.SetTitle('other_nonprompt')
    for hist in histlist:
	if hist.GetName()[-len(binname):]!=binname: continue
	# (make sure this histogram belongs to current bin)
	if 'QCD_prompt' in hist.GetName(): continue
	# (ignore histogram that is empty by construction)
	if( 'nonprompt' in hist.GetName() and 'QCD' not in hist.GetName() ):
	    othernphist.Add(hist)
	    continue
	label = hist.GetName().split('_')[:2]
	if label[0]=='data': label = label[0]
        else: label = label[0]+'_'+label[1]
        hist.SetTitle(label)
	newhistlist.append(hist)
    newhistlist.append(othernphist)
    return newhistlist
 

if __name__=='__main__':
   
    sys.stderr.write('###starting###\n')
 
    if not len(sys.argv)==6:
	print('### ERROR ###: wrong number of command-line arguments')
	print('               use: python fitTemplates.py <var> <flavour> <year>')
	print('			   <fitplotdir> <frmapdir>')
	sys.exit()
    
    # initialize arguments and global parameters
    var = sys.argv[1]
    flavour = sys.argv[2]
    year = sys.argv[3]
    fitplotdir = sys.argv[4]
    frmapdir = sys.argv[5]
    instancename = flavour+'_'+year+'_'+var
    inputfile = 'fakeRateMeasurement_data_'+instancename+'_histograms.root'
    basehistname = 'data_numerator_'+var+'_'+year+'_'+flavour
    (ptbins,etabins) = readptetabins(inputfile,basehistname)
    ptrange = copy(ptbins); ptrange.append(100)
    etarange = copy(etabins); etarange.append(2.5)
    if(flavour=='muon'): etarange[-1]=2.4
    # define temporary file that will be used to store intermediate results in
    tempfilename = 'fitTemplates_temp_'+instancename+'.root'
    # make 2D maps to hold number of nonprompt leptons in num and denom
    nummap = ROOT.TH2D( "fake-rate", "fake-rate; p_{T} (GeV); |#eta|", 
			len(ptrange)-1, array('f',ptrange), len(etarange)-1, array('f',etarange))
    nummap.SetDirectory(0)
    nummap.Sumw2()
    denommap = ROOT.TH2D( "", "", 
			  len(ptrange)-1, array('f',ptrange), len(etarange)-1, array('f',etarange))
    denommap.SetDirectory(0)
    denommap.Sumw2()
    npyieldmaps = {'denominator':denommap,'numerator':nummap}

    print('running on file {}'.format(inputfile))
    print('start looping over pt and eta bins...')
    for ptbin in ptbins:
	for etabin in etabins:
	    #if( ptbin>15. or etabin > 0.5 ): continue # for testing on small number of bins
	    ptbinstr = str(ptbin).replace('.','p')
	    etabinstr = str(etabin).replace('.','p')
	    print('   bin pt {}, eta {}'.format(ptbinstr,etabinstr))
	    for tag in ['denominator','numerator']:
		### make prefit plot ###
		# load and select histograms
		thisbin = tag+'_'+var+'_'+year+'_'+flavour+'_pT_'+ptbinstr+'_eta_'+etabinstr
		histlist = ht.loadallhistograms(inputfile)
		histlist = ht.selecthistograms(histlist,mustcontainall=[thisbin])[1]
		histlist = subselect_prefitplot(histlist,thisbin)
		print('seaching for histograms with tag '+thisbin+'...')
		print('found:')
		for hist in histlist: print('  '+hist.GetName())
		# write histograms to temporary file, then use that to make plots
		f = ROOT.TFile.Open(tempfilename,'recreate')
		for hist in histlist: hist.Write()
		f.Close()
		os.system('./plotHistogramsInFile {} {} {} {}'.format(tempfilename,
			    os.path.join(fitplotdir,flavour+'_pT_'+ptbinstr+'_eta_'+etabinstr
			    +'_'+tag+'_fakeRateMeasurement'+year+'_prefit.png'),
			    'mcvsdata','fakerate'))
		### add all prompt and nonprompt processes and write them to a root file ###
		prompthist = histlist[0].Clone()
		prompthist.Reset()
		prompthist.SetName('total_prompt_'+thisbin)
		prompthist.SetTitle('total_prompt')
		nonprompthist = histlist[0].Clone()
		nonprompthist.Reset()
		nonprompthist.SetName('total_nonprompt_'+thisbin)
		nonprompthist.SetTitle('total_nonprompt')
		datahist = None
		for hist in histlist:
		    if('prompt' in hist.GetName() and not 'nonprompt' in hist.GetName()):
			prompthist.Add(hist)
		    elif 'nonprompt' in hist.GetName(): nonprompthist.Add(hist)
		    elif 'data' in hist.GetName(): datahist = hist
		f = ROOT.TFile.Open(tempfilename,'recreate')
		prompthist.Write()
		nonprompthist.Write()
		datahist.Write()
		f.Close()
		prefitnp = nonprompthist.Integral()
		### make datacard ###
		datacard = 'fitTemplates_datacard_'+instancename+'.txt'
		makedatacard(tempfilename,datacard,tag,var,year,flavour,ptbinstr,etabinstr)
		channelname = (str(tag)+'_'+str(var)+'_'+str(year)+'_'+str(flavour)
				+'_pT_'+ptbinstr+'_eta_'+etabinstr) # name of this 'channel'
		### run combine ###
		script_name = datacard.replace('.txt','.sh')
		impactfig = '' # empty string means no impact plot will be made
		#impactfig = (os.path.join(fitplotdir,flavour+'_pT_'+ptbinstr+'_eta_'+etabinstr
                #            +'_'+tag+'_fakeRateMeasurement'+year+'_impacts'))
		with open( script_name, 'w') as script:
		    initializeJobScript( script )
		    addcombinecommands(script,datacard,impactfig=impactfig)
		os.system('bash '+script_name)
		resfile = datacard.replace('.txt','_out_signalstrength_obs.txt')
		if fitfailed(resfile):
		    print('### WARNING ###: fit for this bin seems to have failed....')
		    print('continuing to next bin without postfit processing.')
		    continue
		(r,uperror,downerror) = readr(resfile)
		print('measured signal strength: {} + {} - {}'.format(r,uperror,downerror))
		### make post-fit distributions ###
		postfitfile = 'fitDiagnostics'+datacard.replace('.txt','.root')
		f = ROOT.TFile.Open(postfitfile,'read')
		# histograms are TH1F so copy to TH1D object for plotting
		temp = f.Get('shapes_fit_s/'+channelname+'/total_prompt')
		postfitprompthist = ROOT.TH1D()
		temp.Copy(postfitprompthist)
		postfitprompthist.SetDirectory(0)
		postfitprompthist.SetTitle('prompt')
		temp = f.Get('shapes_fit_s/'+channelname+'/total_nonprompt')
		postfitnonprompthist = ROOT.TH1D()
		temp.Copy(postfitnonprompthist)
		postfitnonprompthist.SetDirectory(0)
		postfitnonprompthist.SetTitle('nonprompt')
		# also need to re-get the data since x-axis changed in fit
		# but resulting object is a graph, not a histogram
		postfitdatagraph = f.Get('shapes_fit_s/'+channelname+'/data')
		postfitdatahist = ht.tgraphtohist( postfitdatagraph )
		postfitdatahist.SetDirectory(0)
		f.Close()
		# write to temporary file and use that for plotting
		f = ROOT.TFile.Open(tempfilename,'recreate')
                postfitprompthist.Write()
                postfitnonprompthist.Write()
		postfitdatahist.Write()
                f.Close()
		os.system('./plotHistogramsInFile {} {} {} {}'.format(tempfilename,
                            os.path.join(fitplotdir,flavour+'_pT_'+ptbinstr+'_eta_'+etabinstr
                            +'_'+tag+'_fakeRateMeasurement'+year+'_postfit.png'),
                            'mcvsdata','fakerate'))
		### fill the yield maps ###
		binindex = nummap.FindBin(ptbin+1e-6,etabin+1e-6)
		# OPTION 1: multiply initial amount of leptons by signal strength
		#postfitnp = prefitnp*r
                #npyieldmaps[tag].SetBinContent(binindex,postfitnp)
                #npyieldmaps[tag].SetBinError(binindex,prefitnp*max(uperror,downerror))
		# OPTION 2: directly take integral of postfit distribution
		# (but still use signal strength measurement for relative error)
		postfitnp = postfitnonprompthist.Integral()
		npyieldmaps[tag].SetBinContent(binindex,postfitnp)
		npyieldmaps[tag].SetBinError(binindex,postfitnp*max(uperror,downerror)/r)
		# print results
                print('initial amount of nonprompt leptons: {}'.format(prefitnp))
                print('measured signal strength: {} + {} - {}'.format(r,uperror,downerror))
                print('post-fit amount of nonprompt leptons: {}'.format(postfitnp))
    ### make fake rate map
    frmap = nummap.Clone()
    frmap.Divide(denommap)
    # save and plot fake rate map
    frmapfile = os.path.join(frmapdir,'fakeRateMap_data_'+instancename+'.root')
    f = ROOT.TFile.Open(frmapfile,'recreate')
    frmap.Write("fakeRate_" + flavour + "_" + year)
    f.Close()
    os.system('./plotHistogramsInFile {} {} {}'.format(frmapfile, 
		frmapfile.replace('.root','.png'),'2dmap'))
    sys.stderr.write('###done###\n')
