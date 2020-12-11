#############################################################
# python script to run the singlehistplotter and make plots #
#############################################################
import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.join(os.path.dirname(__file__),'../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.join(os.path.dirname(__file__),'../tools'))
import histtools
import plottools as tools

def submitjob(cwd, command):
    script_name = 'singlehistplotter.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        script.write(command+'\n')
    submitQsubJob(script_name)

def getminmax(histlist):
    # get suitable minimum and maximum values for plotting a hist collection (not stacked)
    totmax = 0.
    totmin = 99.
    for hist in histlist:
        for i in range(1,hist.GetNbinsX()+1):
            val = hist.GetBinContent(i)
            if val > totmax: totmax = val
            if val < totmin: totmin = val
    topmargin = (totmax-totmin)/2.
    bottommargin = (totmax-totmin)/5.
    #return (totmin-bottommargin,totmax+topmargin)
    return (0,totmax+topmargin)
    #return(totmin/10.,totmax*10.)

def plothistograms(mchistlist,yaxtitle,xaxtitle,outfile,
		    errorbars=False,normalize=False,
		    clist=None,labels=None):

    tools.setTDRstyle()
    ROOT.gROOT.SetBatch(ROOT.kTRUE)

    ### define global parameters for size and positioning
    ROOT.gROOT.SetBatch(ROOT.kTRUE)
    cheight = 600 # height of canvas
    cwidth = 450 # width of canvas
    # fonts and sizes:
    #titlefont = 6; titlesize = 60
    labelfont = 5; labelsize = 22
    axtitlefont = 5; axtitlesize = 22
    #infofont = 6; infosize = 40
    #legendfont = 4; legendsize = 30
    # title offset
    ytitleoffset = 2.5
    xtitleoffset = 1.
    # margins:
    ptopmargin = 0.07
    pbottommargin = 0.12
    leftmargin = 0.2
    rightmargin = 0.05
    # legend box
    plegendbox = [leftmargin+0.03,1-ptopmargin-0.15,1-rightmargin-0.03,1-ptopmargin-0.03]

    ### operations on mc histograms
    if clist is None:
	clist = [ROOT.kAzure,ROOT.kCyan+1,ROOT.kViolet]
    if len(mchistlist)>len(clist):
	print('### ERROR ###: add colors to clist to support plotting this many histograms.')
	sys.exit()
    for i,hist in enumerate(mchistlist):
	hist.SetLineWidth(2)
	hist.SetLineColor(clist[i])
	scale = 1
	if normalize: scale = hist.Integral("width")
        for j in range(0,hist.GetNbinsX()+2):
            if hist.GetBinContent(j)<=0:
                hist.SetBinContent(j,0.)
                hist.SetBinError(j,0.)
            else:
                hist.SetBinContent(j,hist.GetBinContent(j)/scale)
                hist.SetBinError(j,hist.GetBinError(j)/scale)


    ### make legend for upper plot and add all histograms
    legend = ROOT.TLegend(plegendbox[0],plegendbox[1],plegendbox[2],plegendbox[3])
    legend.SetNColumns(1)
    legend.SetFillStyle(0)
    for i,hist in enumerate(mchistlist):
	label = hist.GetTitle()
	if labels is not None: label = labels[i]
        legend.AddEntry(hist,label,"l")

    ### make canvas and pads
    c1 = ROOT.TCanvas("c1","c1")
    c1.SetCanvasSize(cwidth,cheight)
    pad1 = ROOT.TPad("pad1","",0.,0.,1.,1.)
    pad1.SetTopMargin(ptopmargin)
    pad1.SetBottomMargin(pbottommargin)
    pad1.SetLeftMargin(leftmargin)
    pad1.SetRightMargin(rightmargin)
    pad1.Draw()

    ### make upper part of the plot
    pad1.cd()
    #pad1.SetLogy()
    (rangemin,rangemax) = getminmax(mchistlist)
    mchistlist[0].SetMinimum(rangemin)
    mchistlist[0].SetMaximum(rangemax)

    # X-axis layout
    xax = mchistlist[0].GetXaxis()
    xax.SetNdivisions(5,4,0,ROOT.kTRUE)
    xax.SetLabelSize(labelsize)
    xax.SetLabelFont(10*labelfont+3)
    xax.SetTitle(xaxtitle)
    xax.SetTitleFont(10*axtitlefont+3)
    xax.SetTitleSize(axtitlesize)
    xax.SetTitleOffset(xtitleoffset)
    # Y-axis layout
    yax = mchistlist[0].GetYaxis()
    yax.SetMaxDigits(3)
    yax.SetNdivisions(8,4,0,ROOT.kTRUE)
    yax.SetLabelFont(10*labelfont+3)
    yax.SetLabelSize(labelsize)
    yax.SetTitle(yaxtitle)
    yax.SetTitleFont(10*axtitlefont+3)
    yax.SetTitleSize(axtitlesize)
    yax.SetTitleOffset(ytitleoffset)

    # histograms
    extraoptions = ""
    if errorbars: extraoptions += " e"
    mchistlist[0].Draw("hist"+extraoptions)
    for hist in mchistlist[1:]:
        hist.Draw("hist same"+extraoptions)
    legend.SetFillColor(ROOT.kWhite)
    legend.Draw("same")
    ROOT.gPad.RedrawAxis()

    # draw header
    tools.drawLumi(pad1,lumitext="simulation")

    ### save the plot
    c1.SaveAs(outfile.rstrip('.png')+'.png')

#--------------------------------------------------------#

if __name__=='__main__':
    
    # intitializations
    dofill = False
    doplot = False
    doplotloop = False

    # parse command line arguments
    if len(sys.argv) == 2:
	# apply plotting to all files in given folder
	dofill = False
	doplot = False
	doplotloop = True
	hist_file_path = os.path.abspath(sys.argv[1])

    elif len(sys.argv) == 3:
	# apply plotting to a single file
	dofill = False
	doplot = True
	doplotloop = False
	hist_file_path = os.path.abspath(sys.argv[1])
	output_file_path = os.path.abspath(sys.argv[2])

    elif len(sys.argv) > 7:
	dofill = True
	doplot = False
	doplotloop = False
	input_file_path = os.path.abspath(sys.argv[1])
	hist_file_path = os.path.abspath(sys.argv[2])
	event_selection = sys.argv[3]
	if os.path.exists(hist_file_path):
	    print('### WARNING ###: output file already exists. Overwrite it? (y/n)')
	    go = raw_input()
	    if not go=='y': sys.exit()
	    os.system('rm '+hist_file_path)
	xlow = float(sys.argv[4])
	xhigh = float(sys.argv[5])
	nbins = int(sys.argv[6])
	variables = []
	for i in range(7,len(sys.argv)):
	    variables.append(sys.argv[i])
	cwd = os.getcwd()

    else:
	print('### ERROR ###: singlehistplotter.py requires either 1, 2 or at least 7 command-line arguments.')
	print('Normal usage from the command line:')
	print('  python singlehistplotter.py <hist_folder>')
	print('OR')
	print('  python singlehistplotter.py <hist_file> <output_file>')
	print('OR')
	print('  python singlehistplotter.py <input_file> <hist_file> <event_selection>')
	print('				     <xlow> <xhigh> <nbins>')
	print('				     at least one <variable>')
	sys.exit()

    # fill histograms if requested
    if dofill:
	# check validity of arguments
	for variable in variables:
	    if variable not in ['leadingLeptonPt','subLeadingLeptonPt','trailingLeptonPt',
				'leadingPtCorrection','subLeadingPtCorrection','trailingPtCorrection',
				'leadingPtCorrectionRel','subLeadingPtCorrectionRel',
				'trailingPtCorrectionRel',
		                'minTOPMVA','mintZqMVA','minttHMVA']:
		print('### ERROR ###: variable not in list of recognized variables.')
		sys.exit()
	if event_selection not in ['atLeastThreeLooseLightLeptons',
				    'threeFOLightLeptons','threeTightLightLeptons',
				    'threeFOLightLeptonsThreeTightVeto',
				    'fullEventSelection']:
	    print('### ERROR ###: event selection not in list of recognized selections.')
            sys.exit()
	# check if executable is present
	if not os.path.exists('./singlehistplotter'):
	    print('### ERROR ###: singlehistplotter executable was not found.')
	    print('Run make -f makeSingleHistPlotter before running this script.')
	    sys.exit()

	# run the command to make the histogram objects
	command = './singlehistplotter {} {} {} {} {} {}'.format(
                    input_file_path, hist_file_path, event_selection, xlow, xhigh, nbins)
        for variable in variables: command += ' {}'.format(variable)
	# job submission
	submitjob(cwd, command)
	# alternative: run locally
        #os.system(command)

    if doplot:

	histlist = loadhistograms(hist_file_path)
	binwidth = histlist[0].GetBinWidth(1)
	if binwidth.is_integer():
	    yaxtitle = 'normalized number of events / '+str(int(binwidth))
	else:
	    yaxtitle = 'normalized number of events / {0:.2f}'.format(binwidth)
	#xaxtitle = histlist[0].GetXaxis().GetTitle()
	xaxtitle = r'p_{T} correction relative to reco-p_{T}'
	plothistograms(histlist,yaxtitle,xaxtitle,output_file_path,errorbars=True)

    if doplotloop:
    
	filelist = [os.path.join(hist_file_path,f) for f in os.listdir(hist_file_path) if f[-5:]=='.root']
	for f in filelist:
	    histlist = loadhistograms(f)
	    binwidth = histlist[0].GetBinWidth(1)
	    if binwidth.is_integer():
		yaxtitle = 'normalized number of events / '+str(int(binwidth))
	    else:
		yaxtitle = 'normalized number of events / {0:.2f}'.format(binwidth)
	    #xaxtitle = histlist[0].GetXaxis().GetTitle()
	    xaxtitle = 'lepton pT (GeV)'
	    plothistograms(histlist,yaxtitle,xaxtitle,f[:-5],errorbars=True)
