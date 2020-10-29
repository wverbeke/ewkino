############################################################
# A translation into python of ewkino/plotting/plotCode.cc #
############################################################
# to be used after histfiller.py to create the actual plots of the histograms

import ROOT
import sys
import numpy as np
import json
import os
import plottools as tools

def loadhistograms(histfile,prefix):
    # load histograms from a root file.
    # 'histfile' is a string containing the path to the input root file.
    # 'prefix is the name of histograms to be loaded: 
    # all histograms with name prefix + <int> will be loaded.
    # the output is a python list of histograms, a normalization parameter (see histfiller.py) 
    # and the luminosity that was used for normalization.
    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    try: normalization = int(f.Get("normalization")[0])
    except: normalization = -1
    if(normalization==1):
        try: lumi = f.Get("lumi")[0]
        except: lumi = 0.
    else: lumi = 0
    histlist = []
    i = 0
    while True:
        if(f.Get(prefix+str(i)) == None): break
        histlist.append(f.Get(prefix+str(i)))
        histlist[i].SetDirectory(0)
        i += 1
    return (histlist,normalization,lumi)

def orderhistograms(histlist,ascending=True):
    # order a list of histograms according to sumOfWeights
    weightlist = []
    for hist in histlist:
        sumofweights = hist.GetSumOfWeights()
        weightlist.append(sumofweights)
    weightlist = np.array(weightlist)
    sorted_indices = np.argsort(weightlist)
    if ascending: return [histlist[i] for i in sorted_indices]
    else: return [histlist[i] for i in np.flip(sorted_indices)]

def findbytitle(histlist,title):
    # find a histogram by its title, return the index or -1 if not found
    index = -1
    for i,hist in enumerate(histlist):
        if hist.GetTitle()==title: index = i
    return index

def stackcol(hist,color):
    # set color and line properties of histogram in stack
    hist.SetFillColor(color)
    hist.SetLineWidth(1)
    hist.SetLineColor(ROOT.kBlack)

def setcolorTZQ(tag):
    tag = tag.replace(' ','').replace('{','').replace('}','')
    tag = tag.replace('#','').replace('/','').replace('+','')
    # return a color corresponding to a given tag (so far very analysis-dependent...)
    if(tag=='tZq'): return ROOT.kRed-7
    if(tag=='nonprompt'): return ROOT.kOrange
    if(tag=='WZ'): return ROOT.kCyan-7
    if(tag=='multiboson'): return ROOT.kYellow+1
    if(tag=='tbarttX'): return ROOT.kBlue-10
    if(tag=='tbartZ'): return ROOT.kBlue-6
    if(tag=='ZZH'): return ROOT.kTeal-5
    if(tag=='Xgamma'): return ROOT.kMagenta-7
    print('### WARNING ###: tag not recognized (in setcolorTZQ), returning default color')
    return ROOT.kBlack

def clip(hist):
    # set minimum value for all bins to zero (no net negative weight)
    for i in range(hist.GetNbinsX()+1):
	if hist.GetBinContent(i)<0:
	    hist.SetBinContent(i,0.)
	    hist.SetBinError(i,0)

def getminmax(datahist,mchist,yaxlog):
    # get suitable minimum and maximum values for plotting a given data hist and summed mc hist
    # find maximum:
    histmax = (mchist.GetBinContent(mchist.GetMaximumBin())
                +mchist.GetBinErrorUp(mchist.GetMaximumBin()))
    histmax = max(histmax,datahist.GetBinContent(datahist.GetMaximumBin())
                            +datahist.GetBinErrorUp(datahist.GetMaximumBin()))
    if not yaxlog: 
	return (0,histmax*1.5)
	#return (0.,160.)
    # find minimum (manually to avoid zero)
    histmin = histmax
    for i in range(1,mchist.GetNbinsX()+1):
        if( (not mchist.GetBinContent(i)==0) and mchist.GetBinContent(i)<histmin):
            histmin = mchist.GetBinContent(i)
    rangemin = histmin/5.
    rangemax = histmax*np.power(histmax/rangemin,0.4)
    return (rangemin,rangemax)

def drawbincontent(mchistlist,mchisterror,tag):
    text = ROOT.TLatex()
    text.SetTextAlign(21)
    text.SetTextFont(5);
    text.SetTextSize(22);
    taghist = mchistlist[findbytitle(mchistlist,tag)]
    for i in range(1, taghist.GetNbinsX()+1):
	xcoord  = taghist.GetXaxis().GetBinCenter(i)
	ycoord  = mchisterror.GetBinContent(i)+mchisterror.GetBinErrorUp(i)
	printvalue = taghist.GetBinContent(i)
	text.DrawLatex(xcoord, ycoord+0.3, '{:.2f}'.format(printvalue))
    return None

def plotdatavsmc(datahist,mchistlist,mcsysthist,yaxtitle,yaxlog,xaxtitle,lumi,outfile):
    
    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    rfrac = 0.25 # fraction of ratio plot in canvas
    # fonts and sizes:
    #titlefont = 6; titlesize = 60
    labelfont = 5; labelsize = 15
    axtitlefont = 5; axtitlesize = 22
    #infofont = 6; infosize = 40
    #legendfont = 4; legendsize = 40
    # title offset
    ytitleoffset = 2.1
    xtitleoffset = 4.5
    # margins:
    p1topmargin = 0.07
    p2bottommargin = 0.4
    leftmargin = 0.15
    rightmargin = 0.05
    # legend box
    p1legendbox = [leftmargin+0.03,1-p1topmargin-0.25,1-rightmargin-0.03,1-p1topmargin-0.03]
    p2legendbox = [leftmargin+0.03,0.84,1-rightmargin-0.03,0.97]
    # marker properties for data
    markerstyle = 20
    markercolor = 1
    markersize = 0.75  

    ### order mc histograms
    # order by sumofweights
    mchistlist = orderhistograms(mchistlist)
    # get tZq to the back (i.e. on top of the plot)
    tzqindex = findbytitle(mchistlist,"tZq")
    if(tzqindex>-1):
        indices = list(range(len(mchistlist)))
        indices.remove(tzqindex)
        indices = indices+[tzqindex]
        mchistlist = [mchistlist[i] for i in indices]

    ### operations on mc histograms
    mchistsum = mchistlist[0].Clone()
    mchistsum.Reset()
    mchiststack = ROOT.THStack("mchiststack","")
    for hist in mchistlist:
        stackcol(hist,setcolorTZQ(hist.GetTitle()))
	clip(hist) # set negative bins to zero!
        mchistsum.Add(hist)
        mchiststack.Add(hist)

    print('checkpoint 1')
    
    ### calculate total mc error and set its histogram properties
    mcerror = mchistsum.Clone()
    if mcsysthist is not None:
        for i in range(1,mchistsum.GetNbinsX()+1):
            staterror = mchistsum.GetBinError(i)
            systerror = mcsysthist.GetBinContent(i)
            mcerror.SetBinError(i,np.sqrt(np.power(staterror,2)+np.power(systerror,2)))
    mcerror.SetFillStyle(3244)
    mcerror.SetLineWidth(0)
    mcerror.SetFillColor(ROOT.kGray+2)
    mcerror.SetMarkerStyle(0)

    ### calculate total and statistical mc error (scaled)
    scstaterror = mcerror.Clone()
    scerror = mcerror.Clone()
    for i in range(1,mcerror.GetNbinsX()+1):
        scstaterror.SetBinContent(i,1.)
        scerror.SetBinContent(i,1.)
        if not mcerror.GetBinContent(i)==0:
            scstaterror.SetBinError(i,mchistsum.GetBinError(i)/mchistsum.GetBinContent(i))
            scerror.SetBinError(i,mcerror.GetBinError(i)/mcerror.GetBinContent(i))
        else:
            scstaterror.SetBinError(i,0.)
            scerror.SetBinError(i,0.)
    scstaterror.SetFillStyle(1001)
    scerror.SetFillStyle(1001)
    scstaterror.SetFillColor(ROOT.kCyan-4)
    scerror.SetFillColor(ROOT.kOrange-4)
    scstaterror.SetMarkerStyle(1)
    scerror.SetMarkerStyle(1)

    print('checkpoint 2')

    ### operations on data histogram
    datahist.SetMarkerStyle(markerstyle)
    datahist.SetMarkerColor(markercolor)
    datahist.SetMarkerSize(markersize)

    ### calculate data to mc ratio
    ratiograph = ROOT.TGraphAsymmErrors(datahist)
    for i in range(1,datahist.GetNbinsX()+1):
        if not mchistsum.GetBinContent(i)==0:
            ratiograph.GetY()[i-1] *= 1./mchistsum.GetBinContent(i)
            ratiograph.SetPointError(i-1,0,0,datahist.GetBinErrorLow(i)/mchistsum.GetBinContent(i),
                                        datahist.GetBinErrorUp(i)/mchistsum.GetBinContent(i))
        # avoid drawing empty mc or data bins
        else: ratiograph.GetY()[i-1] = 1e6
        if(datahist.GetBinContent(i)<=0): ratiograph.GetY()[i-1] += 1e6

    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(p1legendbox[0],p1legendbox[1],p1legendbox[2],p1legendbox[3])
    legend.SetNColumns(2)
    legend.SetFillStyle(0)
    legend.AddEntry(datahist,datahist.GetTitle(),"pe1")
    for hist in mchistlist:
        legend.AddEntry(hist,hist.GetTitle(),"f")
    legend.AddEntry(mcerror,"total sim. unc.","f")

    print('checkpoint 3')

    ### make legend for lower plot and add all histograms
    legend2 = ROOT.TLegend(p2legendbox[0],p2legendbox[1],p2legendbox[2],p2legendbox[3])
    legend2.SetNColumns(3); 
    legend2.SetFillStyle(0);
    legend2.AddEntry(scstaterror, "stat. pred. unc.", "f");
    legend2.AddEntry(scerror, "total pred. unc.", "f");
    legend2.AddEntry(ratiograph, "obs./pred.", "pe12");

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,rfrac,1.,1.)
    pad1.SetTopMargin(p1topmargin)
    pad1.SetBottomMargin(0.03)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.Draw()
    pad2 = ROOT.TPad("pad2","",0.,0.,1.,rfrac)
    pad2.SetTopMargin(0.01)
    pad2.SetBottomMargin(p2bottommargin)
    pad2.SetLeftMargin(leftmargin)
    pad2.SetRightMargin(rightmargin)
    pad2.Draw()

    print('checkpoint 4')
    
    ### make upper part of the plot
    pad1.cd()
    # determine range of pad
    if(yaxlog): pad1.SetLogy()
    (rangemin,rangemax) = getminmax(datahist,mcerror,yaxlog)
    #(rangemin,rangemax) = (0,120) # temp override
    mcerror.SetMinimum(rangemin)
    mcerror.SetMaximum(rangemax)

    # X-axis layout
    xax = mcerror.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(0)
    # Y-axis layout
    yax = mcerror.GetYaxis()
    yax.SetMaxDigits(4)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw mcerror first to get range correct
    mcerror.Draw("e2")
    # now draw in correct order
    mchiststack.Draw("hist same")
    mcerror.Draw("e2 same")
    datahist.Draw("pe1 same")
    legend.Draw("same")
    # draw some extra info if needed
    #drawbincontent(mchistlist,mcerror,'tZq')
    ROOT.gPad.RedrawAxis()

    # draw header
    lumistr = '{0:.1f}'.format(lumi/1000.)
    tools.drawLumi(pad1,lumitext=lumistr+" fb^{-1} (13 TeV)")

    ### make the lower part of the plot
    pad2.cd()
    # X-axis layout
    xax = scerror.GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = scerror.GetYaxis()
    yax.SetRangeUser(0.,1.999);
    yax.SetTitle("obs./pred.");
    yax.SetMaxDigits(3)
    yax.SetNdivisions(4,5,0)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # draw objects
    scerror.Draw("e2")
    scstaterror.Draw("e2 same")
    ratiograph.Draw("pe01 same")
    legend2.Draw("same")
    ROOT.gPad.RedrawAxis()

    # make and draw unit ratio line
    xmax = datahist.GetXaxis().GetBinUpEdge(datahist.GetNbinsX())
    xmin = datahist.GetXaxis().GetBinLowEdge(1)
    line = ROOT.TLine(xmin,1,xmax,1)
    line.SetLineStyle(2)
    line.Draw("same")
    
    ### save the plot
    c1.SaveAs(outfile+'.png')

if __name__=="__main__":
    # read a root file containing histograms and make the plots
    # for all variables in an input dict (can be the same dict used for histfiller.py)
    
    ### Configure input parameters (hard-coded)
    # file to read the histograms from
    histfile = os.path.abspath('histograms/histograms.root')
    # variables with axis titles, units, etc.
    variables = [
        {'name':'_abs_eta_recoil','title':r'#||{#eta}_{recoil}','unit':''},
        {'name':'_Mjj_max','title':r'M_{jet+jet}^{max}','unit':'GeV'},
        {'name':'_lW_asymmetry','title':r'asymmetry (lepton from W)','unit':''},
        {'name':'_deepCSV_max','title':r'highest deepCSV','unit':''},
        {'name':'_lT','title':'L_{T}','unit':'GeV'},
        {'name':'_MT','title':'M_{T}','unit':'GeV'},
        {'name':'_pTjj_max','title':r'p_{T}^{max}(jet+jet)','unit':'GeV'},
        {'name':'_dRlb_min','title':r'#Delta R(lep,bjet)_{min}','unit':''},
        {'name':'_dPhill_max','title':r'#Delta #Phi (lep,lep)_{max}','unit':''},
        {'name':'_HT','title':r'H_{T}','unit':'GeV'},
        {'name':'_nJets','title':r'number of jets','unit':''},
        {'name':'_dRlWrecoil','title':r'#Delta R(lep_{W},jet_{recoil})','unit':''},
        {'name':'_dRlWbtagged','title':r'#Delta R(lep_{W},jet_{b-tagged})','unit':''},
        {'name':'_M3l','title':r'M_{3l}','unit':'GeV'},
        {'name':'_abs_eta_max','title':r'#||{#eta}_{max}','unit':''},
	#{'name':'_eventBDT','title':r'event BDT output score','unit':''},
	{'name':'_nMuons','title':r'number of muons in event','unit':''},
	{'name':'_nElectrons','title':r'number of electrons in event','unit':''},
	{'name':'_yield','title':r'total yield','unit':''},
	#{'name':'_leptonMVATOP_min','title':r'minimum TOP MVA value in event','unit':''},
	#{'name':'_leptonMVAttH_min','title':r'minimum ttH MVA value in event','unit':''},
	{'name':'_leptonPtLeading','title':r'p_T^{leading}','unit':'GeV'},
	{'name':'_leptonPtSubLeading','title':r'p_T^{subleading}','unit':'GeV'},
	{'name':'_leptonPtTrailing','title':r'p_T^{trailing}','unit':'GeV'}	
    ]

    ### Overwrite using cmd args
    if(len(sys.argv)==3):
	histfile = sys.argv[1]
	variables = json.loads(sys.argv[2])
    elif(not len(sys.argv)==1):
	print('### ERROR ###: wrong number of command line args')
	sys.exit()

    histdir = histfile[:histfile.rfind('/')]
    ### Loop over input variables
    for k,vardict in enumerate(variables):
        varname = str(vardict['name'])
        # (explicit conversion from unicode to str seems necessary...)

        ### Load histograms
        mchistlist,normalization,lumi = loadhistograms(histfile,'mc_'+varname+'_')
	datahistlist,_,_ = loadhistograms(histfile,'data_'+varname+'_')
        if not len(datahistlist)==1:
            print('### ERROR ###: list of data histograms has unexpected length: '+str(len(datahistlist)))
            sys.exit()
        datahist = datahistlist[0]
	npdatahistlist,_,_ = loadhistograms(histfile,'npdata_'+varname+'_')
	if not (len(npdatahistlist)==0 or len(npdatahistlist)==1):
	    print('### ERROR ###: list of nonprompt data histograms has unexpected length.')
	    sys.exit()
	npdatahist = None
	if len(npdatahistlist)>0: 
	    npdatahist = npdatahistlist[0]
	    mchistlist.append(npdatahist)
        
        ### Set plot properties
        binwidth = datahist.GetBinWidth(1)
        if binwidth.is_integer():
            yaxtitle = 'events / '+str(int(binwidth))+' '+vardict['unit']
        else:
            yaxtitle = 'events / {0:.2f}'.format(binwidth)+' '+vardict['unit']
        xaxtitle = vardict['title']
        if not vardict['unit']=='':
            xaxtitle += '('+vardict['unit']+')'
	figname = os.path.join(histdir,varname)
	plotdatavsmc(datahist,mchistlist,None,yaxtitle,False,xaxtitle,lumi,figname+'_lin')
	plotdatavsmc(datahist,mchistlist,None,yaxtitle,True,xaxtitle,lumi,figname+'_log')
