#############################################################
# python script to run the singlehistplotter and make plots #
#############################################################
import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
#sys.path.append(os.path.abspath('../tools'))
#import smalltools as tls
import plottools as tools

dofill = False
doplot = False
doplotloop = False

if len(sys.argv) == 2:
    # apply plotting to all files in given folder
    doplotloop = True
    hist_file_path = os.path.abspath(sys.argv[1])

elif len(sys.argv) == 3:
    doplot = True
    hist_file_path = os.path.abspath(sys.argv[1])
    output_file_path = os.path.abspath(sys.argv[2])

elif len(sys.argv) > 6:
    dofill = True
    input_file_path = os.path.abspath(sys.argv[1])
    hist_file_path = os.path.abspath(sys.argv[2])
    if os.path.exists(hist_file_path):
	print('### WARNING ###: output file already exists. Overwrite it? (y/n)')
	go = raw_input()
	if not go=='y': sys.exit()
	os.system('rm '+hist_file_path)
    xlow = float(sys.argv[3])
    xhigh = float(sys.argv[4])
    nbins = int(sys.argv[5])
    variables = []
    for i in range(6,len(sys.argv)):
	variables.append(sys.argv[i])
    cwd = os.getcwd()

else:
    print('### ERROR ###: singlehistplotter.py requires either 1, 2 or at least 6 command-line arguments.')
    print('Normal usage from the command line:')
    print('  python singlehistplotter.py <hist_folder>')
    print('OR')
    print('  python singlehistplotter.py <hist_file> <output_file>')
    print('OR')
    print('  python singlehistplotter.py <input_file> <hist_file> <xlow> <xhigh> <nbins>')
    print('       at least one <variable>')
    sys.exit()

def submitjob(cwd,input_file_path,output_file_path,
                        xlow, xhigh, nbins, variables):
    script_name = 'singlehistplotter.sh'
    with open(script_name,'w') as script:
        initializeJobScript(script)
        script.write('cd {}\n'.format(cwd))
        command = './singlehistplotter {} {} {} {} {}'.format(
                    input_file_path, output_file_path, xlow, xhigh, nbins)
        for variable in variables:
            command += ' {}'.format(variable)
        script.write(command+'\n')
    submitQsubJob(script_name)
    # alternative: run locally
    #os.system('bash '+script_name)

if dofill:

    # check validity of arguments
    for variable in variables:
	if variable not in ['leadingLeptonPt','subLeadingLeptonPt','trailingLeptonPt',
			    'minTOPMVA','mintZqMVA','minttHMVA']:
	    print('### ERROR ###: variable not in list of recognized variables.')
	    sys.exit()

    # check if executable is present
    if not os.path.exists('./singlehistplotter'):
	print('### ERROR ###: singlehistplotter executable was not found.')
	print('Run make -f makeSingleHistPlotter before running this script.')
	sys.exit()

    # run the command to make the histogram objects
    submitjob(cwd,input_file_path, hist_file_path, xlow, xhigh, nbins, variables)

def loadhistograms(histfile):
    # load histograms from a root file.
    # 'histfile' is a string containing the path to the input root file.
    # the output is a list of histograms
    print('loading histograms...')
    f = ROOT.TFile.Open(histfile)
    histlist = []
    keylist = f.GetListOfKeys()
    for key in keylist:
        hist = f.Get(key.GetName())
        hist.SetDirectory(0)
        try:
            nentries = hist.GetEntries() # maybe replace by more histogram-specific function
        except:
            print('### WARNING ###: key "'+str(key.GetName())+'" does not correspond to valid hist.')
        histlist.append(hist)
    return histlist

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

def plothistograms(mchistlist,yaxtitle,xaxtitle,outfile,errorbars=False):

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
    clist = [ROOT.kAzure,ROOT.kCyan+1,ROOT.kViolet]
    if len(mchistlist)>len(clist):
	print('### ERROR ###: add colors to clist to support plotting this many histograms.')
	sys.exit()
    for i,hist in enumerate(mchistlist):
	hist.SetLineWidth(2)
	hist.SetLineColor(clist[i])
	scale = hist.Integral("width")
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
    for hist in mchistlist:
        legend.AddEntry(hist,hist.GetTitle(),"l")

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

if doplot:

    histlist = loadhistograms(hist_file_path)
    binwidth = histlist[0].GetBinWidth(1)
    if binwidth.is_integer():
        yaxtitle = 'normalized number of events / '+str(int(binwidth))
    else:
        yaxtitle = 'normalized number of events / {0:.2f}'.format(binwidth)
    xaxtitle = histlist[0].GetXaxis().GetTitle()
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
